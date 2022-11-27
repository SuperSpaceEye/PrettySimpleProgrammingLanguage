//
// Created by spaceeye on 25.10.22.
//

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string> &str_data, bool debug) {
    auto res = Transpiler::transpile_data(str_data);
    if (debug) {Transpiler::display_tokens(res);}
    auto ast = Parser::create_ast(res, debug);
    Parser::validate_ast(ast, debug);

    return compile_(ast);
}

std::vector<ByteCode> Compiler::compile_(ASTCreationResult &ast) {
    std::vector<ByteCode> bcode;
    //TODO only main for now.

    StackScope scope;
    int stack_size = 0;

    auto & node = ast.function_roots[0];
    while (node != nullptr) {
        switch (node->act_type) {
            case ActionType::VariableDeclaration: {
                auto & var_d = *static_cast<VariableDeclaration*>(node.get());
                int num_b = 0;
                switch (var_d.var_type) {
                    case VariableType::INT:
                    case VariableType::UINT:
                    case VariableType::FLOAT:
                        bcode.emplace_back(ByteCode::PUSH);
                        bcode.emplace_back((ByteCode)0);bcode.emplace_back((ByteCode)0);bcode.emplace_back((ByteCode)0);bcode.emplace_back((ByteCode)0);
                        num_b = 0;
                        break;
                }
                scope.push(num_b, stack_size, var_d.var_id, var_d.var_type);
                stack_size += num_b;
            }
                break;
            case ActionType::VariableCall:
                break;
            case ActionType::FunctionCall:
                break;
            case ActionType::FunctionDeclaration:
                break;
            case ActionType::ForLoop:
                break;
            case ActionType::WhileLoop:
                break;
            case ActionType::IfStatement:
                break;
            case ActionType::StartLogicBlock:
                break;
            case ActionType::EndLogicBlock: {
                auto cleanup = scope.pop_scope();
                for (auto & item: cleanup) {
                    bcode.emplace_back(ByteCode::POP);
                    bcode.emplace_back((ByteCode)0);bcode.emplace_back((ByteCode)0);bcode.emplace_back((ByteCode)0);bcode.emplace_back((ByteCode)0);
                    // will overwrite 4 null bytes of an array with item uint num bytes
                    *((uint32_t*)bcode[bcode.size()-4]) = (uint32_t)(std::get<0>(item));
                }
            }
                break;
            case ActionType::ReturnCall:
                break;
        }

        node = node->next_action;
    }

    return bcode;
}
