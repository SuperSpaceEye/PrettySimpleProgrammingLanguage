//
// Created by spaceeye on 27.11.22.
//

#include "../Parser.h"

bool check_for_main(ASTCreationResult & ast_result);

bool Parser::validate_ast(ASTCreationResult &ast_result, bool debug) {
    if (!check_for_main(ast_result)) {throw std::logic_error("No main() function");}

    Scope scope;
    scope.populate_builtins(ast_result.reg);

    for (auto & root: ast_result.function_roots) {
        recursive_validate(scope, root);
    }

    return true;
}

void Parser::recursive_validate(Scope &scope, std::shared_ptr<BaseAction> &root) {
    int do_not_push_scope = 0;
    while (root != nullptr) {
        switch (root->act_type) {
            case ActionType::VariableDeclaration: {
                auto & var_decl = *static_cast<VariableDeclaration*>(root.get());
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
                    auto & arg = fn_call.arguments[i];
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
                            recursive_validate(scope, arg);
                        }
                            break;
                        case ActionType::NumericConst:
                            break;
                        default:
                            throw std::logic_error("Invalid function argument");
                    }
                }
            }
                break;
            case ActionType::FunctionDeclaration: {
                auto & fn_decl = *static_cast<FunctionDeclaration*>(root.get());

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
            case ActionType::ReturnCall:
                break;
        }
        root = root->next_action;
    }
}

bool check_for_main(ASTCreationResult & ast_result) {
    bool has_main = false;
    for (auto & item: ast_result.reg.function_ids) {
        if (std::get<1>(item) == "main") {
            has_main = true;
            break;
        }
    }
    if (!has_main) {return false;}
    return true;
}