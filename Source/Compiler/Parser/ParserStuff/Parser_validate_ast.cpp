//
// Created by spaceeye on 27.11.22.
//

#include "../Parser.h"

bool check_for_main(ASTCreationResult & ast_result);

void Parser::validate_ast(ASTCreationResult &ast_result, bool debug) {
    if (!check_for_main(ast_result)) {throw std::logic_error("No void main() function");}

    ValidateScope scope;
    scope.populate_builtins(ast_result.reg);
    std::vector<int> ids{};
    int last_id = -1;

    for (auto root: ast_result.object_roots) {
        auto _root = root;
        recursive_validate(scope, root, ids, last_id, _root);
    }
}

void
Parser::recursive_validate(ValidateScope &scope, std::shared_ptr<BaseAction> &root, std::vector<int> &ids, int &last_id,
                           std::shared_ptr<BaseAction> &prev_root) {
    int do_not_push_scope = 0;
    while (root != nullptr) {
        switch (root->act_type) {
            case ActionType::VariableDeclaration: {
                auto & var_decl = *static_cast<VariableDeclaration*>(root.get());
                if (scope.check_id_in_lscope(var_decl.var_id)) {throw std::logic_error("Cannot redeclare a variable in same scope");}
                switch (var_decl.var_type) {
                    case VariableType::VOID:
                    case VariableType::B_ANY:
                        throw std::logic_error("Can't declare variable with type void");
                        break;
                    case VariableType::INT:
                    case VariableType::UINT:
                    case VariableType::FLOAT:
                    case VariableType::ARRAY:
                    case VariableType::STRING:
                        break;
                }
                scope.push_id(var_decl.var_id, var_decl.var_type);
            }
                break;
            case ActionType::VariableCall: {
                auto & var_call = *static_cast<VariableCall*>(root.get());
                if (!scope.check_id(var_call.var_id)) {throw std::logic_error("Variable was not declared in this scope.");}
                var_call.type = scope.get_var_type(var_call.var_id);
            }
                break;
            case ActionType::FunctionCall: {
                auto & fn_call = *static_cast<FunctionCallAction*>(root.get());

                int num_needed_args = std::get<1>(scope.get_fn(fn_call.name_id)).size();
                int num_actual_args = fn_call.arguments.size();

                if (num_needed_args != num_actual_args) {
                    throw std::logic_error("Num required arguments does not equal num given arguments");
                }

                auto set_type = VariableType::VOID;
                for (int i = 0; i < fn_call.arguments.size(); i++) {
                    auto arg = fn_call.arguments[i];
                    auto val = std::get<1>(scope.get_fn(fn_call.name_id))[i];
                    auto req_type = val.first;
                    auto is_ref = val.second;
                    fn_call.required_arguments.emplace_back(req_type);

                    switch (arg->act_type) {
                        case ActionType::VariableCall: {
                            auto _arg = arg;
                            recursive_validate(scope, _arg, ids, last_id, prev_root);

                            auto & var_call = *static_cast<VariableCall*>(arg.get());

                            if (req_type == VariableType::B_ANY) {
                                if (set_type == VariableType::VOID) {
                                    set_type = var_call.type;
                                    if (fn_call.return_type == VariableType::B_ANY) { fn_call.return_type = set_type; }
                                }
                                if (var_call.type != set_type) {
                                    throw std::logic_error("Types in arguments in B_ANY do not match");
                                }
                            }

                            if (!(req_type == VariableType::B_ANY || scope.get_var_type(var_call.var_id) == req_type)) {
                                throw std::logic_error("Type of function argument doesn't match required type");
                            }
                        }
                            break;
                        case ActionType::FunctionCall: {
                            auto & afn_call = *static_cast<FunctionCallAction*>(arg.get());

                            if (is_ref) {throw std::logic_error("Function argument requires reference to variable.");}

                            recursive_validate(scope, arg, ids, last_id, prev_root);

                            if (req_type == VariableType::B_ANY) {
                                if (set_type == VariableType::VOID) {
                                    set_type = afn_call.return_type;
                                    if (fn_call.return_type == VariableType::B_ANY) { fn_call.return_type = set_type; }
                                }
                                if (afn_call.return_type != set_type) {
                                    throw std::logic_error("Types in arguments in B_ANY do not match");
                                }
                            }

                            if (!(req_type == VariableType::B_ANY || req_type == afn_call.return_type)) {
                                throw std::logic_error("Type of function argument doesn't match required type");
                            }
                        }
                            break;
                        case ActionType::NumericConst: {
                            auto & num_arg = *static_cast<NumericConst*>(arg.get());

                            if (is_ref) {throw std::logic_error("Function argument requires reference to variable.");}

                            if (req_type == VariableType::B_ANY) {
                                if (set_type == VariableType::VOID) {
                                    set_type = num_arg.type;
                                    if (fn_call.return_type == VariableType::B_ANY) { fn_call.return_type = set_type; }
                                }
                                if (num_arg.type != set_type) {
                                    throw std::logic_error("Types in arguments in B_ANY do not match");
                                }
                            }

                            if (!(req_type == VariableType::B_ANY || req_type == num_arg.type)) {throw std::logic_error("Type of numeric const does not match type of required argument.");}
                        }
                            break;
                        case ActionType::StringConst: {
                            if (!(req_type == VariableType::B_ANY || req_type == VariableType::STRING)) {throw std::logic_error("Type of a function argument is not string");}
                        }
                            break;
                        default:
                            throw std::logic_error("Invalid function argument");
                    }
                }
            }
                break;
            case ActionType::FunctionDeclaration: {
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
                break;
            case ActionType::ForLoop:
                break;
            case ActionType::WhileLoop: {
                auto & while_act = *static_cast<WhileLoop*>(root.get());

                switch (while_act.expression->act_type) {
                    case ActionType::VariableCall: {
                        auto & var_call = *static_cast<VariableCall*>(while_act.expression.get());

                        recursive_validate(scope, while_act.expression, ids, last_id, prev_root);

                        //TODO add checks for numeric types
                        if (var_call.type != VariableType::UINT) {
                            auto to_uint = std::make_shared<FunctionCallAction>(FunctionCallAction{
                                BaseAction{ActionType::FunctionCall},
                                FunctionType::BuiltinFunction,
                                (int)BuiltinIDS::TO_UINT,
                                0,
                                VariableType::UINT,
                                {while_act.expression},
                                {VariableType::B_ANY}
                            });

                            while_act.expression = to_uint;
                        }
                    }
                        break;
                    case ActionType::FunctionCall: {
                        auto & fn_call = *static_cast<FunctionCallAction*>(while_act.expression.get());

                        auto in = while_act.expression;
                        recursive_validate(scope, in, ids, last_id, prev_root);

                        if (fn_call.return_type != VariableType::UINT) {
                            auto to_uint = std::make_shared<FunctionCallAction>(FunctionCallAction{
                                    BaseAction{ActionType::FunctionCall},
                                    FunctionType::BuiltinFunction,
                                    (int)BuiltinIDS::TO_UINT,
                                    0,
                                    VariableType::UINT,
                                    {while_act.expression},
                                    {VariableType::B_ANY}
                            });

                            while_act.expression = to_uint;
                        }
                    }
                        break;
                    case ActionType::NumericConst: {
                        auto & num_const = *static_cast<NumericConst*>(while_act.expression.get());

                        // works for all types
                        if (num_const.value == 0) {
                            // just generate code for the while loop if it will never be called
                            prev_root->next_action = root->next_action;
                            goto while_loop_validation_end;
                        }
                    }
                        break;
                    default:
                        throw std::logic_error("Wrong statement in while loop expression.");
                }

                auto body = while_act.body;
                recursive_validate(scope, body, ids, last_id, prev_root);
            }
                while_loop_validation_end:;
                break;
            case ActionType::IfStatement: {
                auto & if_call = *static_cast<IfStatement*>(root.get());

                switch (if_call.expression->act_type) {
                    //TODO make implicit type casting for arguments.
                    case ActionType::VariableCall: {

                        auto to_val = if_call.expression;
                        recursive_validate(scope, to_val, ids, last_id, prev_root);

                        auto & var_call = *static_cast<VariableCall*>(if_call.expression.get());

                        if (var_call.type != VariableType::UINT) {throw std::logic_error("Type of expression in if statement must be uint.");}
                    }
                        break;
                    case ActionType::FunctionCall: {
                        auto & fn_call = *static_cast<FunctionCallAction*>(if_call.expression.get());

                        auto expr = if_call.expression;
                        recursive_validate(scope, expr, ids, last_id, prev_root);
                        if (fn_call.return_type != VariableType::UINT) { throw std::logic_error("Return type of a function in an if statement expression must be uint.");}
                    }
                        break;
                    default: {throw std::logic_error("Incorrect if statement expression");}
                }
                auto & if_true = if_call.true_branch;
                auto & if_false = if_call.false_branch;

                auto in = if_true;
                recursive_validate(scope, in, ids, last_id, prev_root);
                in = if_false;
                recursive_validate(scope, in, ids, last_id, prev_root);
            }
                break;
            case ActionType::StartLogicBlock:
                if (!do_not_push_scope) {
                    scope.push_scope();
                } else {do_not_push_scope--;}
                root = root->next_action;
                ids.emplace_back(last_id);
                recursive_validate(scope, root, ids, last_id, prev_root);
                break;
            case ActionType::EndLogicBlock: {
                scope.pop_scope();
                ids.pop_back();
                return;
            }
                break;
            case ActionType::NumericConst:
                break;
            case ActionType::ReturnCall: {
                auto & ret_call = *static_cast<ReturnCall*>(root.get());
                auto fn = scope.get_fn(ids.back());
                auto return_type = std::get<2>(fn);

                auto & arg = ret_call.argument;
                if (return_type == VariableType::VOID) {
                    if (arg != nullptr) {throw std::logic_error("Return from void function.");}
                } else {
                    switch (arg->act_type) {
                        case ActionType::VariableCall: {
                            auto & var = *static_cast<VariableCall*>(arg.get());

                            if (!(return_type == VariableType::B_ANY || scope.get_var_type(var.var_id) == return_type)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        case ActionType::FunctionCall: {
                            auto & fn_call = *static_cast<FunctionCallAction*>(arg.get());

                            auto in_arg = arg;
                            recursive_validate(scope, in_arg, ids, last_id, prev_root);

                            if (!(return_type == VariableType::B_ANY || fn_call.return_type == return_type)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        case ActionType::NumericConst: {
                            auto & num_const = *static_cast<NumericConst*>(arg.get());
                            if (!(return_type == VariableType::B_ANY || num_const.type == return_type)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        case ActionType::StringConst: {
                            if (!(return_type == VariableType::B_ANY || return_type == VariableType::STRING)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        default:
                            throw std::logic_error("Invalid return argument");
                    }
                }
            }
                break;
        }
        prev_root = root;
        root = root->next_action;
    }
}

bool check_for_main(ASTCreationResult & ast_result) {
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