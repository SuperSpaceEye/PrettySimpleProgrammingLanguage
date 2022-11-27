//
// Created by spaceeye on 27.11.22.
//

#include "../Parser.h"

bool check_for_main(ASTCreationResult & ast_result);

void Parser::validate_ast(ASTCreationResult &ast_result, bool debug) {
    if (!check_for_main(ast_result)) {throw std::logic_error("No void main() function");}

    ValidateScope scope;
    scope.populate_builtins(ast_result.reg);

    for (auto root: ast_result.function_roots) {
        recursive_validate(scope, root, -1);
    }
}

void Parser::recursive_validate(ValidateScope &scope, std::shared_ptr<BaseAction> &root, int parent_obj_id) {
    int do_not_push_scope = 0;
    while (root != nullptr) {
        switch (root->act_type) {
            case ActionType::VariableDeclaration: {
                auto & var_decl = *static_cast<VariableDeclaration*>(root.get());
                if (scope.check_id_in_lscope(var_decl.var_id)) {throw std::logic_error("Cannot redeclare a variable in same scope");}
                scope.push_id(var_decl.var_id, var_decl.var_type);
            }
                break;
            case ActionType::VariableCall: {
                auto & var_call = *static_cast<VariableCall*>(root.get());
                if (!scope.check_id(var_call.var_id)) {throw std::logic_error("Variable was not declared in this scope.");}
            }
                break;
            case ActionType::FunctionCall: {
                auto & fn_call = *static_cast<FunctionCallAction*>(root.get());
                for (int i = 0; i < fn_call.arguments.size(); i++) {
                    auto arg = fn_call.arguments[i];
                    auto req_type = std::get<1>(scope.get_fn(fn_call.name_id))[i];
                    switch (arg->act_type) {
                        case ActionType::VariableCall: {
                            auto & var_call = *static_cast<VariableCall*>(arg.get());
                            if (!(req_type == VariableType::B_ANY || scope.get_var_type(var_call.var_id) == req_type)) {
                                throw std::logic_error("Type of function argument doesn't match required type");
                            }
                        }
                            break;
                        case ActionType::FunctionCall: {
                            auto & afn_call = *static_cast<FunctionCallAction*>(arg.get());
                            if (!(req_type == VariableType::B_ANY || req_type == afn_call.return_type)) {
                                throw std::logic_error("Type of function argument doesn't match required type");
                            }
                            recursive_validate(scope, arg, 0);
                        }
                            break;
                        case ActionType::NumericConst: {
                            auto & num_arg = *static_cast<NumericConst*>(arg.get());
                            if (!(req_type == VariableType::B_ANY || req_type == VariableType::STRING)) {throw std::logic_error("Type of numeric const does not match type of required argument.");}
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
                parent_obj_id = fn_decl.fn_id;

                auto args = std::vector<VariableType>{};
                for (auto & arg: fn_decl.arguments) {args.emplace_back(std::get<0>(arg));}

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
            case ActionType::WhileLoop:
                break;
            case ActionType::IfStatement:
                break;
            case ActionType::StartLogicBlock:
                if (!do_not_push_scope) {
                    scope.push_scope();
                } else {do_not_push_scope--;}
                break;
            case ActionType::EndLogicBlock: {
                scope.pop_scope();
            }
                break;
            case ActionType::NumericConst:
                break;
            case ActionType::ReturnCall: {
                auto & ret_call = *static_cast<ReturnCall*>(root.get());
                auto fn = scope.get_fn(parent_obj_id);
                auto return_type = std::get<2>(fn);

                auto & arg = ret_call.argument;
                if (return_type == VariableType::VOID) {
                    if (arg != nullptr) {throw std::logic_error("Return from void function.");}
                } else {
                    switch (arg->act_type) {
                        case ActionType::VariableCall: {
                            auto & var = *static_cast<VariableCall*>(arg.get());
                            if (!(return_type == VariableType::B_ANY || scope.get_var_type(var.var_id) != return_type)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        case ActionType::FunctionCall: {
                            auto & fn_call = *static_cast<FunctionCallAction*>(arg.get());
                            if (!(return_type == VariableType::B_ANY || fn_call.return_type != return_type)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        case ActionType::NumericConst: {
                            auto & num_const = *static_cast<NumericConst*>(arg.get());
                            if (!(return_type == VariableType::B_ANY || num_const.type != return_type)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        case ActionType::StringConst: {
                            if (!(return_type == VariableType::B_ANY || return_type != VariableType::STRING)) {throw std::logic_error("Invalid return type.");}
                        }
                            break;
                        default:
                            throw std::logic_error("Invalid return argument");
                    }
                }
            }
                break;
        }
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