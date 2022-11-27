//
// Created by spaceeye on 25.10.22.

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string> &str_data, bool debug) {
    auto res = Transpiler::transpile_data(str_data);
    if (debug) {Transpiler::display_tokens(res);}
    auto ast = Parser::create_ast(res, debug);
    Parser::validate_ast(ast, debug);

    auto code = compile_(ast);
    if (debug) {display_code(code);}

    return code;
}

int type_size(VariableType type) {
    switch (type) {
        case VariableType::VOID: return 0;
        case VariableType::B_ANY: return 4;

        case VariableType::INT:
        case VariableType::UINT:
        case VariableType::FLOAT:
            return 4;
        //8 bytes for 64 bit pointer to object. type enum is not needed as everything should be resolved at validate stage of compilation.
        case VariableType::ARRAY:
        case VariableType::STRING:
            return 8;
    }
}

bool arg_is_ref(FunctionCallAction & node, int pos) {
    if (node.fn_type == FunctionType::BuiltinFunction) {
        return std::get<2>(builtin_functions_id_names[node.fn_id])[pos].second;
    } else {
        //TODO
        return false;
    }
}

void put_4_num(std::vector<ByteCode> & bcode, uint32_t num) {
//    for (int i = 0; i < 4; i++) {
//        uint32_t temp_num = num;
//        bcode.emplace_back((ByteCode)(temp_num>>((i)*4)));
//    }
    int pos = bcode.size();
    for (int i = 0; i < 4; i++) {bcode.emplace_back();}
    *((uint32_t*)&bcode[pos]) = num;
}

void put_8_num(std::vector<ByteCode> & bcode, uint64_t num) {
    for (int i = 0; i < 8; i++) {
        uint32_t temp_num = num;
        bcode.emplace_back((ByteCode)(temp_num>>((i)*8)));
    }
}

std::vector<ByteCode> Compiler::compile_(ASTCreationResult &ast) {
    std::vector<ByteCode> bcode;
    //TODO only main for now.

    StackScope scope;
    int stack_size = 0;

    auto node = ast.function_roots[0];
    recursive_compile(bcode, scope, stack_size, node);

    return bcode;
}

void Compiler::recursive_compile(std::vector<ByteCode> &bcode, StackScope &scope, int stack_size,
                                 std::shared_ptr<BaseAction> &node) {
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
                        put_4_num(bcode, 4);
                        put_4_num(bcode, 0);
                        num_b = 4;
                        break;
                }
                scope.push(num_b, stack_size, var_d.var_id, var_d.var_type);
                stack_size += num_b;
            }
                break;
            case ActionType::VariableCall:
                break;
            case ActionType::FunctionCall: {
                scope.push_scope();
                auto & fn_call = *static_cast<FunctionCallAction*>(node.get());
                for (int i = 0; i < fn_call.arguments.size(); i++) {
                    auto & arg = fn_call.arguments[i];
                    switch (arg->act_type) {
                        case ActionType::VariableCall: {
                            auto & var_call = *static_cast<VariableCall*>(arg.get());
                            auto data = scope.get_var(var_call.var_id);
                            uint32_t var_size = std::get<0>(data);
                            uint32_t var_pos = std::get<1>(data);
                            if (arg_is_ref(fn_call, i)) {
                                bcode.emplace_back(ByteCode::PUSH);
                                put_4_num(bcode, 4);
                                put_4_num(bcode, var_pos);
                                stack_size += 4;
                                scope.push(4, stack_size-4, 0, VariableType::INT);
                            } else {
                                bcode.emplace_back(ByteCode::COPY_PUSH);
                                if (var_size == 4) {
                                    put_4_num(bcode, var_pos);
                                    put_4_num(bcode, var_size);
                                    stack_size += 4;
                                    scope.push(4, stack_size-4, 0, VariableType::INT);
                                } else if (var_size == 8) {
                                    put_8_num(bcode, var_pos);
                                    put_8_num(bcode, var_size);
                                    stack_size += 8;
                                    scope.push(8, stack_size-8, 0, VariableType::INT);
                                }
                            }
                        }
                            break;
                        case ActionType::FunctionCall: {
                            //return value will be on top of the stack.
                            recursive_compile(bcode, scope, stack_size, arg);
                        }
                            break;
                        case ActionType::NumericConst: {
                            auto & num_call = *static_cast<NumericConst*>(arg.get());
                            bcode.push_back(ByteCode::PUSH);
                            put_4_num(bcode, 4);
                            put_4_num(bcode, num_call.value);
                            stack_size += 4;
                            scope.push(4, stack_size-4, 0, VariableType::INT);
                        }
                            break;
                        case ActionType::StringConst:
                            //TODO
                            break;
                    }
                }
                if (fn_call.fn_type == FunctionType::BuiltinFunction) {
                    bcode.emplace_back(ByteCode::BUILTIN_CALL);
                    put_4_num(bcode, fn_call.fn_id);

                    auto needed_byte_len = type_size(fn_call.return_type);

                    //TODO put into function
                    if (needed_byte_len != 0) {
                        bcode.emplace_back(ByteCode::SWAP);
                        put_4_num(bcode, std::get<1>(scope.get_min_pos_var_of_scope()));
                        put_4_num(bcode, stack_size);
                        put_4_num(bcode, needed_byte_len);

                        while (needed_byte_len > 0) {
                            //if scope doesn't have enough variables to free from pop.
                            if (scope.get_current_total() == 0) {
                                bcode.emplace_back(ByteCode::PUSH);
                                put_4_num(bcode, 4);
                                put_4_num(bcode, needed_byte_len);
                                scope.push(needed_byte_len, 0, 0, VariableType::INT);
                            }
                            auto begin_var_data = scope.get_min_pos_var_of_scope();
                            needed_byte_len -= std::get<0>(begin_var_data);
                            scope.free_var_from_popping(std::get<2>(begin_var_data));
                        }

                        //if there is too many not freed bytes
                        if (needed_byte_len < 0) {
                            //only num bytes matters
                            scope.push(std::abs(needed_byte_len), 0, 0, VariableType::INT);
                        }
                    }

                    auto cleanup = scope.pop_scope();
                    for (auto & item: cleanup) {
                        bcode.emplace_back(ByteCode::POP);
                        put_4_num(bcode, std::get<0>(item));
                        stack_size -= std::get<0>(item);
                    }
                } else {

                }
            }
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
                scope.push_scope();
                break;
            case ActionType::EndLogicBlock: {
                auto cleanup = scope.pop_scope();
                for (auto & item: cleanup) {
                    bcode.emplace_back(ByteCode::POP);
                    put_4_num(bcode, std::get<0>(item));
                    stack_size -= std::get<0>(item);
                }
            }
                break;
            //swap values
            case ActionType::ReturnCall: {
                auto & ret_call = *static_cast<ReturnCall*>(node.get());
                switch (ret_call.argument->act_type) {
                    case ActionType::VariableCall: {
                        auto & var_call = *static_cast<VariableCall*>(ret_call.argument.get());
                        auto return_data = scope.get_var(var_call.var_id);
                        auto first_scope_var = scope.get_min_pos_var_of_scope();
                        //if variable to return is first declared, then it just needs to not pop data when returning
                        if (std::get<2>(return_data) != std::get<2>(first_scope_var)) {
                            scope.free_var_from_popping(std::get<2>(return_data));
                        //if variable is not first declared, then it needs to swap data with first position, without
                        //popping returning data.
                        } else {
                            int needed_byte_len = std::get<0>(return_data);

                            bcode.emplace_back(ByteCode::SWAP);
                            put_4_num(bcode, std::get<1>(return_data));
                            put_4_num(bcode, std::get<1>(first_scope_var));
                            put_4_num(bcode, needed_byte_len);

                            while (needed_byte_len > 0) {
                                //if scope doesn't have enough variables to free from pop.
                                if (scope.get_current_total() == 0) {
                                    bcode.emplace_back(ByteCode::PUSH);
                                    put_4_num(bcode, 4);
                                    put_4_num(bcode, needed_byte_len);
                                    scope.push(needed_byte_len, 0, 0, VariableType::INT);
                                }
                                auto begin_var_data = scope.get_min_pos_var_of_scope();
                                needed_byte_len -= std::get<0>(begin_var_data);
                                scope.free_var_from_popping(std::get<2>(begin_var_data));
                            }

                            //if there is too many not freed bytes
                            if (needed_byte_len < 0) {
                                //only num bytes matters
                                scope.push(std::abs(needed_byte_len), 0, 0, VariableType::INT);
                            }
                        }
                    }
                        break;
                    case ActionType::FunctionCall:
                        break;
                    case ActionType::StringConst:
                        break;
                    case ActionType::ReturnCall:
                        break;
                }
            }
                break;
//            case ActionType::NumericConst:
//                break;
//            case ActionType::StringConst:
//                break;
        }

        node = node->next_action;
    }
}


void Compiler::display_code(std::vector<ByteCode> & code) {
    for (int i = 0; i < code.size(); i++) {
        switch (code[i]) {
            case ByteCode::PUSH: {
                std::cout << "PUSH " << *((uint32_t*)&code[++i]);
                i+=4;
                std::cout << " " << *((uint32_t*)&code[i]) << "\n";
                i+=3;
            }
                break;
            case ByteCode::POP: {
                std::cout << "POP  " << *((uint32_t*)&code[++i]) << "\n";
                i+=3;
            }
                break;
            case ByteCode::SWAP: {
                std::cout << "SWAP " << *((uint32_t*)&code[++i]) << " ";
                i+=4;
                std::cout << " " << *((uint32_t*)&code[i]) << " ";
                i+=4;
                std::cout << " " << *((uint32_t*)&code[i]) << "\n";
                i+=3;
            }
                break;
            case ByteCode::COPY_PUSH:
                break;
            case ByteCode::BUILTIN_CALL: {
                std::cout << "BUILTIN_CALL " << std::get<0>(builtin_functions_id_names[*((uint32_t*)&code[++i])]) << "\n";
                i+=3;
            }
                break;
            case ByteCode::GOTO:
                break;
            case ByteCode::EXIT:
                break;
            case ByteCode::COND_GOTO:
                break;
        }
    }
}