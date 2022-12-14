//
// Created by spaceeye on 10.12.22.
//

#include "../TreeValidator.h"

void TreeValidator::validate_ast(ASTCreationResult &ast_result, const Options &options) {
    if (!check_for_main(ast_result)) {throw std::logic_error("No void main() function");}

    ValidateScope scope;
    scope.populate_builtins(ast_result.reg);
    std::vector<int> ids{};
    int last_id = -1;

    for (auto root: ast_result.object_roots) {
        auto _root = root;
        recursive_validate(scope, root, ids, last_id, _root, options);
    }
}

void TreeValidator::recursive_validate(ValidateScope &scope, std::shared_ptr<BaseAction> &root, std::vector<int> &ids,
                                       int &last_id, std::shared_ptr<BaseAction> &prev_root, const Options &options) {
    int do_not_push_scope = 0;
    while (root != nullptr) {
        switch (root->act_type) {
            case ActionType::VariableDeclaration:
                validate_variable_declaration(scope, root, options); break;
            case ActionType::VariableCall:
                validate_variable_call(scope, root, options); break;
            case ActionType::FunctionCall:
                validate_fn_call(root, scope, ids, last_id, prev_root, options); break;
            case ActionType::FunctionDeclaration:
                validate_fn_decl(scope, root, last_id, do_not_push_scope, options); break;
            case ActionType::ForLoop:
                break;
            case ActionType::WhileLoop:
                validate_while_loop(root, scope, ids, last_id, prev_root, options); break;
            case ActionType::IfStatement:
                validate_if_statement(root, scope, ids, last_id, prev_root, options); break;
            case ActionType::ReturnCall:
                validate_return_call(root, scope, ids, last_id, prev_root, options); break;
            case ActionType::StartLogicBlock:
                start_logic_block(scope, root, ids, last_id, prev_root, do_not_push_scope,
                                  options); break;
            case ActionType::EndLogicBlock: {
                scope.pop_scope();
                ids.pop_back();
                return;
            }
                break;
        }
        if (root == nullptr) {
            throw std::logic_error("Something gone wrong. Parser validate stage. Root has become nullptr. If you see this, open an issue on the github or message me, SpaceEye.");
        }

        prev_root = root;
        root = root->next_action;
    }
}

void TreeValidator::start_logic_block(ValidateScope &scope, std::shared_ptr<BaseAction> &root, std::vector<int> &ids,
                                      int &last_id, std::shared_ptr<BaseAction> &prev_root, int &do_not_push_scope,
                                      const Options &options) {
    if (!do_not_push_scope) {
        scope.push_scope();
    } else { do_not_push_scope--; }
    root = root->next_action;
    ids.emplace_back(last_id);
    recursive_validate(scope, root, ids, last_id, prev_root, options);
}

void TreeValidator::validate_return_call(const std::shared_ptr<BaseAction> &root, ValidateScope &scope,
                                         std::vector<int> &ids, int &last_id,
                                         std::shared_ptr<BaseAction> &prev_root, const Options &options) {
    auto & ret_call = *static_cast<ReturnCall*>(root.get());
    auto fn = scope.get_fn(ids.back());
    auto return_type = std::get<2>(fn);

    auto & arg = ret_call.argument;
    if (return_type == VariableType::VOID) {
        if (arg != nullptr) {throw std::logic_error("Non-empty return from void function. " + rcs(ret_call.pos));}
    } else {
        if (arg == nullptr) {throw std::logic_error("Empty return from non-void function. " + rcs(ret_call.pos));}
        VariableType got_type;
        switch (arg->act_type) {
            case ActionType::VariableCall: {
                auto & var = *static_cast<VariableCall*>(arg.get());
                got_type = scope.get_var_type(var.var_id);
            }
                break;
            case ActionType::FunctionCall: {
                auto & fn_call = *static_cast<FunctionCallAction*>(arg.get());

                auto in_arg = arg;
                recursive_validate(scope, in_arg, ids, last_id, prev_root, options);
                got_type = fn_call.return_type;
            }
                break;
            case ActionType::NumericConst: {
                auto & num_const = *static_cast<NumericConst*>(arg.get());
                got_type = num_const.type;
            }
                break;
            case ActionType::StringConst: {
                auto & str_const = *static_cast<StringConst*>(arg.get());
                got_type = VariableType::STRING;
            }
                break;
            default:
                throw std::logic_error("Invalid return argument. "+rcs(arg->pos));
        }

        if (!((return_type == VariableType::NUMERIC_ANY && var_type_is_numeric(got_type)) || got_type == return_type)) {
            throw std::logic_error(
                    "Invalid return type. Expected "
                    + type_to_str(return_type) + " got " + type_to_str(got_type) +". "+rcs(arg->pos));}

    }
}

void TreeValidator::validate_if_statement(std::shared_ptr<BaseAction> &root, ValidateScope &scope,
                                          std::vector<int> &ids,
                                          int &last_id, std::shared_ptr<BaseAction> &prev_root,
                                          const Options &options) {
    auto & if_call = *static_cast<IfStatement*>(root.get());

    switch (if_call.expression->act_type) {
        case ActionType::VariableCall: {
            auto to_val = if_call.expression;
            recursive_validate(scope, to_val, ids, last_id, prev_root, options);

            auto & var_call = *static_cast<VariableCall*>(if_call.expression.get());

            if (var_call.type != VariableType::UINT) {
                if (options.warning.alert_on_implicit_cast_in_structure_arguments) {std::cout << "Warning: implicit cast to "+type_to_str(VariableType::UINT)+" from "+type_to_str(var_call.type)+" in an if call argument. "+rcs(var_call.pos);}
                make_implicit_cast(if_call.expression,
                                   var_call.type,
                                   VariableType::UINT);}
        }
            break;
        case ActionType::FunctionCall: {
            auto & fn_call = *static_cast<FunctionCallAction*>(if_call.expression.get());

            auto expr = if_call.expression;
            recursive_validate(scope, expr, ids, last_id, prev_root, options);
            if (fn_call.return_type != VariableType::UINT) {
                if (options.warning.alert_on_implicit_cast_in_structure_arguments) {std::cout << "Warning: implicit cast to "+type_to_str(VariableType::UINT)+" from "+type_to_str(fn_call.return_type)+" in an if call argument. "+rcs(fn_call.pos);}
                make_implicit_cast(if_call.expression,
                                   fn_call.return_type,
                                   VariableType::UINT);}
        }
            break;
        //TODO
//        case ActionType::NumericConst: {
//
//            auto &num_const = *static_cast<NumericConst*>(if_call.expression.get());
//
//            std::shared_ptr<BaseAction> temp_root;
//
//            if (num_const.value) {
//                temp_root = if_call.true_branch;
//            } else {
//                temp_root = if_call.false_branch;
//            }
//
//            auto in = temp_root;
//            auto temp_prev_root = prev_root;
//            recursive_validate(scope, in, ids, last_id, temp_prev_root, options);
//
//            auto temp = std::make_shared<BaseAction>(BaseAction{ActionType::StartLogicBlock});
//
//            prev_root->next_action = temp;
//            prev_root = prev_root->next_action;
//
//            prev_root->next_action = temp_root;
//            temp_prev_root->next_action = if_call.next_action;
//            root = temp_prev_root;
//        }
//            return;
        default: {throw std::logic_error("Incorrect type of action in an if statement expression. "+rcs(if_call.pos));}
    }
    auto & if_true  = if_call.true_branch;
    auto & if_false = if_call.false_branch;

    auto in = if_true;
    recursive_validate(scope, in, ids, last_id, prev_root, options);
    in = if_false;
    recursive_validate(scope, in, ids, last_id, prev_root, options);
}

void
TreeValidator::validate_while_loop(const std::shared_ptr<BaseAction> &root, ValidateScope &scope, std::vector<int> &ids,
                                   int &last_id,
                                   std::shared_ptr<BaseAction> &prev_root, const Options &options) {
    auto & while_act = *static_cast<WhileLoop*>(root.get());

    switch (while_act.expression->act_type) {
        case ActionType::VariableCall: {
            auto & var_call = *static_cast<VariableCall*>(while_act.expression.get());

            recursive_validate(scope, while_act.expression, ids, last_id, prev_root, options);

            if (var_call.type != VariableType::UINT) {
                if (options.warning.alert_on_implicit_cast_in_structure_arguments) {std::cout << "Warning: implicit cast to "+type_to_str(VariableType::UINT)+" from "+type_to_str(var_call.type)+" in a while call argument. "+rcs(var_call.pos);}
                make_implicit_cast(while_act.expression, var_call.type, VariableType::UINT);}
        }
            break;
        case ActionType::FunctionCall: {
            auto & fn_call = *static_cast<FunctionCallAction*>(while_act.expression.get());

            auto in = while_act.expression;
            recursive_validate(scope, in, ids, last_id, prev_root, options);
            if (fn_call.return_type != VariableType::UINT) {
                if (options.warning.alert_on_implicit_cast_in_structure_arguments) {std::cout << "Warning: implicit cast to "+type_to_str(VariableType::UINT)+" from "+type_to_str(fn_call.return_type)+" in a while call argument. "+rcs(fn_call.pos);}
                make_implicit_cast(while_act.expression, fn_call.return_type, VariableType::UINT);}
        }
            break;
        case ActionType::NumericConst: {
            auto & num_const = *static_cast<NumericConst*>(while_act.expression.get());

            // works for all types
            if (num_const.value == 0) {
                // just generate code for the while loop if it will never be called
                prev_root->next_action = root->next_action;
                return;
            }
        }
            break;
        default:
            throw std::logic_error("Wrong type of action in a while loop expression. "+rcs(while_act.pos));
    }

    auto body = while_act.body;
    recursive_validate(scope, body, ids, last_id, prev_root, options);
}

void TreeValidator::validate_fn_decl(ValidateScope &scope, const std::shared_ptr<BaseAction> &root, int &last_id,
                                     int &do_not_push_scope,
                                     const Options &options) {
    auto & fn_decl = *static_cast<FunctionDeclaration*>(root.get());
    last_id = fn_decl.fn_id;

    auto args = std::vector<std::pair<VariableType, bool>>{};
    for (auto & arg: fn_decl.arguments) {args.emplace_back(std::get<0>(arg), std::get<1>(arg));}

    scope.push_fn(fn_decl.fn_id, args, fn_decl.return_type);

    do_not_push_scope++;
    scope.push_scope();
    for (auto & argument: fn_decl.arguments) {
        scope.push_id(std::get<2>(argument), std::get<0>(argument));
    }
}

void TreeValidator::validate_fn_call(const std::shared_ptr<BaseAction> &root, ValidateScope &scope, std::vector<int> &ids,
                                     int &last_id, std::shared_ptr<BaseAction> &prev_root, const Options &options) {
    auto & fn_call = *static_cast<FunctionCallAction*>(root.get());

    int num_needed_args = std::get<1>(scope.get_fn(fn_call.name_id)).size();
    int num_actual_args = fn_call.arguments.size();
    bool is_builtin = fn_call.fn_type == FunctionType::BuiltinFunction;

    if (num_needed_args != num_actual_args) {
        throw std::logic_error("Number of required arguments {"+std::to_string(num_needed_args)+"} does not "
                                "equal the number of given arguments {"+std::to_string(num_actual_args)+"} to a function call. "+rcs(fn_call.pos));
    }

    auto set_type = VariableType::VOID;
    for (int i = 0; i < fn_call.arguments.size(); i++) {
        auto arg = fn_call.arguments[i];
        auto val = std::get<1>(scope.get_fn(fn_call.name_id))[i];
        auto req_type = val.first;
        auto is_ref = val.second;
        fn_call.required_arguments.emplace_back(req_type);

        VariableType got_type;
        switch (arg->act_type) {
            case ActionType::VariableCall: {
                auto _arg = arg;
                recursive_validate(scope, _arg, ids, last_id, prev_root, options);
                auto & var_call = *static_cast<VariableCall*>(arg.get());
                got_type = var_call.type;
            }
                break;
            case ActionType::FunctionCall: {
                auto & afn_call = *static_cast<FunctionCallAction*>(arg.get());
                if (is_ref) {throw std::logic_error("Function argument requires reference to a variable. "+rcs(afn_call.pos));}
                recursive_validate(scope, arg, ids, last_id, prev_root, options);
                got_type = afn_call.return_type;
            }
                break;
            case ActionType::NumericConst: {
                auto & num_arg = *static_cast<NumericConst*>(arg.get());
                if (is_ref) {throw std::logic_error("Function argument requires reference to variable. "+rcs(num_arg.pos));}
                got_type = num_arg.type;
            }
                break;
            case ActionType::StringConst: {
                got_type = VariableType::STRING;
            }
                break;
            default:
                throw std::logic_error("Invalid function argument. "+rcs(arg->pos));
        }

        if (req_type == VariableType::NUMERIC_ANY) {
            if (!var_type_is_numeric(got_type)) {throw std::logic_error("Non numeric value was given to NUMERIC_ANY type."+rcs(arg->pos));}
            if (set_type == VariableType::VOID) {
                set_type = got_type;
                if (fn_call.return_type == VariableType::NUMERIC_ANY) { fn_call.return_type = set_type; }
            }
            if (got_type != set_type) {
                try_implicit_cast_fn_arg(options, is_builtin, arg, set_type, got_type);
            }
        }

        if (!((req_type == VariableType::NUMERIC_ANY && var_type_is_numeric(got_type)) || got_type == req_type)) {
            try_implicit_cast_fn_arg(options, is_builtin, arg, req_type, got_type);
        }
    }
}

void TreeValidator::try_implicit_cast_fn_arg(const Options &options, bool is_builtin, std::shared_ptr<BaseAction> &arg,
                                        VariableType &req_type, VariableType &got_type) {
    if (( is_builtin && options.other.allow_implicit_cast_for_builtin_fns) ||
        (!is_builtin && options.other.allow_implicit_cast_for_user_defined_fns)) {

        if ((options.warning.alert_on_builtin_fns_implicit_cast && is_builtin) ||
            (options.warning.alert_on_user_defined_fns_implicit_cast && !is_builtin)) {
            std::cout << "Warning! Implicit cast to "+type_to_str(req_type)+" from "+type_to_str(got_type)+" at "+rcs(arg->pos);
        }

        make_implicit_cast(arg, got_type, req_type);
    } else {
        throw std::logic_error("Expected type {"+type_to_str(req_type)+"} of function argument doesn't match type of given value {"+type_to_str(got_type)+"}."+rcs(arg->pos));
    }
}

void TreeValidator::validate_variable_call(ValidateScope &scope, const std::shared_ptr<BaseAction> &root,
                                           const Options &options) {
    auto & var_call = *static_cast<VariableCall*>(root.get());
    if (!scope.check_id(var_call.var_id)) {throw std::logic_error("Variable was not declared in this scope. "+rcs(var_call.pos));}
    var_call.type = scope.get_var_type(var_call.var_id);
}

void TreeValidator::validate_variable_declaration(ValidateScope &scope, const std::shared_ptr<BaseAction> &root,
                                                  const Options &options) {
    auto & var_decl = *static_cast<VariableDeclaration*>(root.get());
    if (scope.check_id_in_lscope(var_decl.var_id)) {throw std::logic_error("Can't redeclare a variable in the same scope. "+rcs(var_decl.pos));}
    switch (var_decl.var_type) {
        case VariableType::VOID:
        case VariableType::NUMERIC_ANY:
            throw std::logic_error("Can't declare variable with type void. "+rcs(var_decl.pos));
        case VariableType::INT:
        case VariableType::UINT:
        case VariableType::FLOAT:
        case VariableType::ARRAY:
        case VariableType::STRING:
            break;
    }
    scope.push_id(var_decl.var_id, var_decl.var_type);
}

void TreeValidator::validate_function_paths(TreeResult &tres) {
    for (const auto & root: tres.object_roots) {
        auto node = root;
        auto res = recursive_validate_function_path(node);
        if (!res) {
            throw std::logic_error("Function has a path without a return. "+ rcs(root->pos));
        }
    }
}

bool TreeValidator::recursive_validate_function_path(std::shared_ptr<BaseAction> node) {
    while (node != nullptr) {
        switch (node->act_type) {
            case ActionType::IfStatement: {
                auto & if_act = *static_cast<IfStatement*>(node.get());

                auto true_res = recursive_validate_function_path(if_act.true_branch);
                bool false_res = false;
                if (if_act.false_branch != nullptr) {
                    false_res = recursive_validate_function_path(if_act.false_branch);
                }

                if (true_res && false_res) {
                    return true;
                }
            }
                break;
            case ActionType::ReturnCall:
                return true;
        }
        node = node->next_action;
    }
    return false;
}

void TreeValidator::make_implicit_cast(std::shared_ptr<BaseAction> &target, VariableType original_type,
                                       VariableType target_type) {
    switch (original_type) {
        case VariableType::INT:
        case VariableType::UINT:
        case VariableType::FLOAT:
            break;
        default:
            throw std::logic_error("Can't implicitly convert type {"+type_to_str(original_type)+"}."+rcs(target->pos));
    }
    BuiltinIDS to_func;
    switch (target_type) {
        case VariableType::INT:    to_func = BuiltinIDS::TO_INT;   break;
        case VariableType::UINT:   to_func = BuiltinIDS::TO_FLOAT; break;
        case VariableType::FLOAT:  to_func = BuiltinIDS::TO_FLOAT; break;
        case VariableType::STRING: to_func = BuiltinIDS::TO_STR;   break;
        default:
            throw std::logic_error("Can't conversion to type {"+type_to_str(target_type)+"}."+rcs(target->pos));
    }

    target = std::make_shared<FunctionCallAction>(
            FunctionCallAction{
                    BaseAction{ActionType::FunctionCall},
                    FunctionType::BuiltinFunction,
                    (int)to_func,
                    0,
                    target_type,
                    {target},
                    {VariableType::NUMERIC_ANY}
            });
}

bool TreeValidator::check_for_main(ASTCreationResult & ast_result) {
    bool has_main = false;
    for (auto & item: ast_result.reg.function_ids) {
        if (std::get<1>(item) == "main") {
            if (std::get<2>(item)->return_type == VariableType::VOID) {
                has_main = true;
            }
            break;
        }
    }
    if (!has_main) {return false;}
    return true;
}