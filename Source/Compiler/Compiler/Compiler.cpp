//
// Created by spaceeye on 25.10.22.

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string> &str_data, bool debug) {
    auto res = Transpiler::transpile_data(str_data);
    if (debug) {Transpiler::display_tokens(res);}

    auto tree = Parser::create_tree(res, debug);

    auto code = compile_(tree);
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
    int pos = bcode.size();
    for (int i = 0; i < 4; i++) {bcode.emplace_back();}
    *((uint32_t*)&bcode[pos]) = num;
}

void put_8_num(std::vector<ByteCode> & bcode, uint64_t num) {
    int pos = bcode.size();
    for (int i = 0; i < 8; i++) {bcode.emplace_back();}
    *((uint64_t *)&bcode[pos]) = num;
}

std::vector<ByteCode> Compiler::compile_(TreeResult &tree_res) {
    std::vector<ByteCode> bcode;
    //TODO only main for now.

    StackScope scope;
    int stack_size = 0;

    auto node = tree_res.object_roots[tree_res.main_idx];
    recursive_compile(bcode, scope, stack_size, node);

    return bcode;
}

void Compiler::recursive_compile(std::vector<ByteCode> &bcode, StackScope &scope, int &stack_size,
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
                            //if var is reference, then function will modify stack pos directly.
                            if (arg_is_ref(fn_call, i)) {
                                bcode.emplace_back(ByteCode::PUSH);
                                put_4_num(bcode, 4);
                                put_4_num(bcode, var_pos);
                                stack_size += 4;
                                scope.push(4, stack_size-4, 0, VariableType::INT);
                            //if not ref, then make copy of stack variable
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
                            //If nested function call, then just recursively process it, as returned value will be on
                            //top of the stack.
                            recursive_compile(bcode, scope, stack_size, arg);
                        }
                            break;
                        //If number const, then just push value saved in code to stack.
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
                //if builtin function, then popping of pushed stack values must be processed manually.
                //if the function is user defined, popping of stack values will be handled by return call.
                if (fn_call.fn_type == FunctionType::BuiltinFunction) {
                    bcode.emplace_back(ByteCode::BUILTIN_CALL);
                    put_4_num(bcode, fn_call.fn_id);

                    //how many bytes it needs to return
                    auto needed_byte_len = type_size(fn_call.return_type);
                    bool popped = false;

                    //if not 0, then it must swap result with the top's scope position
                    //then it must calculate how much values it must pop.
                    if (needed_byte_len != 0) {
                        generate_code_to_return_var_from_scope(scope, needed_byte_len, bcode, stack_size, popped);
                    }
                    if (!popped) {free_scope(scope, bcode, stack_size);}
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
                free_scope(scope, bcode, stack_size);
            }
                break;
            //swap values
            case ActionType::ReturnCall: {
                auto & ret_call = *static_cast<ReturnCall*>(node.get());
                bool popped = false;
                switch (ret_call.argument->act_type) {
                    case ActionType::VariableCall: {
                        auto & var_call = *static_cast<VariableCall*>(ret_call.argument.get());
                        auto return_data = scope.get_var(var_call.var_id);
                        auto first_scope_var = scope.get_min_pos_var_of_scope();

//                        auto need_byte_len = type_size(std::get<3>(return_data));

                        //if variable to return is first declared, then it just needs to not pop data when returning
                        if (std::get<2>(return_data) == std::get<2>(first_scope_var)) {
                            scope.push_one_scope_above(
                                    std::get<0>(return_data),
                                    std::get<1>(return_data),
                                            0,
                                            VariableType::INT);
                            scope.delete_from_scope(std::get<2>(return_data));
                        //if variable is not first declared, then it needs to swap data with first position, without
                        //popping returning data.
                        } else {
                            int needed_byte_len = std::get<0>(return_data);
                            generate_code_to_return_var_from_scope(scope, needed_byte_len, bcode, stack_size, popped);
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

                if (!popped) {free_scope(scope, bcode, stack_size);}
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

void
Compiler::generate_code_to_return_var_from_scope(StackScope &scope, int needed_byte_len, std::vector<ByteCode> &bcode,
                                                 int &stack_size, bool &popped) {
    bcode.emplace_back(ByteCode::SWAP);
    put_4_num(bcode, std::get<1>(scope.get_min_pos_var_of_scope()));
    put_4_num(bcode, stack_size);
    put_4_num(bcode, needed_byte_len);

    int scope_total = scope.get_current_total();
    if (needed_byte_len > scope_total) {
        bcode.emplace_back(ByteCode::PUSH);
        put_4_num(bcode, 4);
        put_4_num(bcode, needed_byte_len - scope_total);
        scope.push(needed_byte_len - scope_total, 0, 0, VariableType::INT);
    }

    scope_total = scope.get_current_total();

    if (scope_total != needed_byte_len) {
        bcode.emplace_back(ByteCode::POP);
        put_4_num(bcode, scope_total - needed_byte_len);
        scope.push_one_scope_above(needed_byte_len, std::get<1>(scope.get_min_pos_var_of_scope()), 0, VariableType::INT);
    }
    scope.pop_scope();
    stack_size -= (scope_total - needed_byte_len);
    popped = true;
}

void Compiler::free_scope(StackScope &scope, std::vector<ByteCode> &bcode, int &stack_size) {
    auto scope_total = scope.get_current_total();
    scope.pop_scope();
    stack_size -= scope_total;
    bcode.emplace_back(ByteCode::POP);
    put_4_num(bcode, scope_total);
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