//
// Created by spaceeye on 25.10.22.

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string> &str_data, bool debug) {
    auto res = Transpiler::transpile_data(str_data);
    if (debug) {Transpiler::display_tokens(res);}

    auto tree = Parser::create_tree(res, debug);

    auto parts = compile_(tree);
    if (debug) {
        int num = 0;

        for (int i = 0; i < parts.size(); i++) {
            display_code(parts[i].fn_code, num);
            std::cout << "\n";
        }
    }

    return construct_program(parts);
}

int type_size(VariableType type) {
    switch (type) {
        case VariableType::VOID: return 0;
        case VariableType::B_ANY: return 8;

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

std::vector<FunctionPart> Compiler::compile_(TreeResult &tree_res) {
    std::vector<FunctionPart> code_parts{};
    //TODO only main for now.

    for (int i = 0; i < tree_res.object_roots.size(); i++) {
        auto scope = StackScope{};
        bool has_args = false;

        if (i != tree_res.main_idx) {
            scope.push_scope();
            //positional argument + size of all arguments
            scope.push(4, 0, 0, VariableType::UINT);

            auto & fn_call = *static_cast<FunctionDeclaration*>(tree_res.object_roots[i].get());
            for (auto & arg: fn_call.arguments) {
                int _type_size;
                //if reference
                if (std::get<1>(arg)) {
                    _type_size = 4;
                } else {
                    _type_size = type_size(std::get<0>(arg));
                }

                scope.push(_type_size, scope.get_current_total(), std::get<2>(arg), std::get<0>(arg));
                has_args = true;
            }
            scope.push_scope_level();
        }

        int do_not_push = (has_args ? 1: 0);

        auto node = tree_res.object_roots[i];
        code_parts.emplace_back();
        code_parts.back().id = static_cast<FunctionDeclaration*>(node.get())->fn_id;
        recursive_compile(code_parts.back(), scope, node, i == tree_res.main_idx, do_not_push);
    }

    //so that main function would be the first.
    std::swap(code_parts[0], code_parts[tree_res.main_idx]);

    return code_parts;
}

void
Compiler::recursive_compile(FunctionPart &part, StackScope &scope, std::shared_ptr<BaseAction> &node, bool is_main,
                            int &do_not_push_scope) {
    auto & main_part = part.fn_code;
    while (node != nullptr) {
        switch (node->act_type) {
            case ActionType::VariableDeclaration: {
                auto & var_d = *static_cast<VariableDeclaration*>(node.get());
                int num_b = 0;
                switch (var_d.var_type) {
                    case VariableType::INT:
                    case VariableType::UINT:
                    case VariableType::FLOAT:
                        main_part.emplace_back(ByteCode::PUSH);
                        put_4_num(main_part, 4);
                        put_4_num(main_part, 0);
                        num_b = 4;
                        break;
                }
                scope.push(num_b, scope.get_current_total(), var_d.var_id, var_d.var_type);
            }
                break;
            case ActionType::VariableCall:
                break;
            case ActionType::FunctionCall: {
                scope.push_scope();
                auto & fn_call = *static_cast<FunctionCallAction*>(node.get());

                main_part.emplace_back(ByteCode::START_ARGUMENTS);
                if (fn_call.fn_type == FunctionType::UserFunction) {
                    main_part.emplace_back(ByteCode::PUSH_STACK_SCOPE);

                    scope.push_scope_level();

                    main_part.emplace_back(ByteCode::PUSH);
                    put_4_num(main_part, 4);
                    part.calls_from_custom.emplace_back(main_part.size());
                    put_4_num(main_part, 0);
                    // TODO idfk why there should be a scope for user functions to work properly.
                    //  I probably fucked up scoping but it works so idfc right now.
                    scope.push_scope();
                    scope.push(4, scope.get_current_total(), 0, VariableType::UINT);
                }

                for (int i = 0; i < fn_call.arguments.size(); i++) {
                    auto & arg = fn_call.arguments[i];
                    switch (arg->act_type) {
                        case ActionType::VariableCall: {
                            auto & var_call = *static_cast<VariableCall*>(arg.get());
                            auto [data, scope_level] = scope.get_var(var_call.var_id);

                            uint32_t var_size = std::get<0>(data);

                            int var_pos = -scope.get_total_between(scope_level, scope.scope.size()-
                                                                                (scope.scope.size()-2 != scope_level ? 1: 2))
                                          + std::get<1>(data)
                            ;

                            //if var is reference, then function will modify stack pos directly.
                            //if ref of b_any then must also place type information

                            if (arg_is_ref(fn_call, i)) {
                                if (fn_call.required_arguments[i] == VariableType::B_ANY) {
                                    main_part.emplace_back(ByteCode::PUSH);
                                    put_4_num(main_part, 4);
                                    put_4_num(main_part, (uint32_t)var_call.type);
                                    scope.push(4, scope.get_current_total(), 0, VariableType::UINT);
                                }

                                //Pushing relative position of variable
                                main_part.emplace_back(ByteCode::PUSH);
                                put_4_num(main_part, 4);
                                put_4_num(main_part, var_pos);

                                //pushing current stack level
                                main_part.emplace_back(ByteCode::PUSH_CURRENT_STACK_LEVEL);

                                //Will push absolute pos while popping prev 2 numbers.
                                main_part.emplace_back(ByteCode::GET_ABSOLUTE_POS);
                                scope.push(4, scope.get_current_total(), 0, VariableType::UINT);
                            //if not ref, then make copy of stack variable
                            } else {
                                if (fn_call.required_arguments[i] == VariableType::B_ANY) {
                                    main_part.emplace_back(ByteCode::PUSH);
                                    put_4_num(main_part, 4);
                                    put_4_num(main_part, (uint32_t)var_call.type);
                                    scope.push(4, scope.get_current_total(), 0, VariableType::UINT);
                                }

                                main_part.emplace_back(ByteCode::COPY_PUSH);
//                                if (var_size == 4) {
                                    put_4_num(main_part, var_pos);
                                    put_4_num(main_part, var_size);
                                    scope.push(4, scope.get_current_total(), 0, VariableType::UINT);
//                                } else if (var_size == 8) {
//                                    put_4_num(fn_code, var_pos);
//                                    put_4_num(fn_code, var_size);
//                                    scope.push(8, scope.get_current_total(), 0, VariableType::UINT);
//                                }


                            }
                        }
                            break;
                        case ActionType::FunctionCall: {
                            if (fn_call.required_arguments[i] == VariableType::B_ANY) {
                                auto &arg_fn_call = *static_cast<FunctionCallAction*>(arg.get());
                                main_part.emplace_back(ByteCode::PUSH);
                                put_4_num(main_part, 4);
                                put_4_num(main_part, (uint32_t)arg_fn_call.return_type);
                                scope.push(4, scope.get_current_total(), 0, VariableType::UINT);
                            }

                            //If nested function call, then just recursively process it, as returned value will be on
                            //top of the stack.
                            auto rec_arg = arg;
                            recursive_compile(part, scope, rec_arg, false, do_not_push_scope);
                        }
                            break;
                        //If number const, then just push value saved in code to stack.
                        //TODO numeric const can't be referenced. Needs to create a temp var.
                        // TODO make check for numeric consts so that they can't be put as ref arg
                        case ActionType::NumericConst: {
                            auto & num_call = *static_cast<NumericConst*>(arg.get());
                            if (fn_call.required_arguments[i] == VariableType::B_ANY) {
                                main_part.emplace_back(ByteCode::PUSH);
                                put_4_num(main_part, 4);
                                put_4_num(main_part, (uint32_t)num_call.type);
                                scope.push(4, scope.get_current_total(), 0, VariableType::UINT);
                            }

                            main_part.push_back(ByteCode::PUSH);
                            put_4_num(main_part, 4);
                            put_4_num(main_part, num_call.value);
                            scope.push(4, scope.get_current_total(), 0, VariableType::INT);
                        }
                            break;
                        case ActionType::StringConst:
                            //TODO
                            break;
                    }
                }
                main_part.emplace_back(ByteCode::END_ARGUMENTS);
                //if builtin function, then popping of pushed stack values must be processed manually.
                //if the function is user defined, popping of stack values will be handled by return call.
                if (fn_call.fn_type == FunctionType::BuiltinFunction) {
                    main_part.emplace_back(ByteCode::BUILTIN_CALL);
                    put_4_num(main_part, fn_call.fn_id);

                    //how many bytes it needs to return
                    auto needed_byte_len = type_size(fn_call.return_type);
                    bool popped = false;

                    needed_byte_len = (needed_byte_len == 8 ? 4: needed_byte_len);
                    //if not 0, then it must swap result with the top's scope position
                    //then it must calculate how much values it must pop.
                    if (needed_byte_len != 0) {
                        //if builtin return type is not void, then it will push 4 bytes as an answer.
                        scope.push(needed_byte_len, scope.get_current_total(), 0, VariableType::UINT);

                        generate_code_to_return_var_from_scope(scope, needed_byte_len, main_part, popped);
                    }
                    main_part.emplace_back(ByteCode::POP_STACK_SCOPE);
                    if (!popped) { free_scope(scope, main_part);}
                } else {
                    scope.push_one_scope_above(type_size(fn_call.return_type), 0, 0, VariableType::UINT);
                    scope.pop_scope();
                    scope.pop_scope();

                    main_part.emplace_back(ByteCode::GOTO);
                    part.calls_to_custom.emplace_back(main_part.size(), fn_call.fn_id);
                    put_4_num(main_part, 0);

                    part.parent_end_of_fn_call.emplace_back(main_part.size());
                    main_part.emplace_back(ByteCode::POP_STACK_SCOPE);
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
                //TODO think about it
                if (!do_not_push_scope) {
                    scope.push_scope();
                    main_part.emplace_back(ByteCode::PUSH_STACK_SCOPE);
                } else {do_not_push_scope--;}
                break;
            case ActionType::EndLogicBlock: {
                free_scope(scope, main_part);
                main_part.emplace_back(ByteCode::POP_STACK_SCOPE);
            }
                break;
            //if there is a return call then we assume that there is a return positional element at the top of the stack.
            case ActionType::ReturnCall: {
                if (is_main) {
                    main_part.emplace_back(ByteCode::GOTO);
                    put_4_num(main_part, UINT32_MAX);
                    break;
                }

                auto times = scope.collapse_to_scope_level();

                for (int i = 0; i < times; i++) {
                    main_part.emplace_back(ByteCode::POP_STACK_SCOPE);
                }

                auto & ret_call = *static_cast<ReturnCall*>(node.get());
                bool popped = false;

                main_part.emplace_back(ByteCode::POP_STACK_SCOPE);
                if (ret_call.argument.get() != nullptr) {
                    switch (ret_call.argument->act_type) {
                        case ActionType::VariableCall: {
                            auto &var_call = *static_cast<VariableCall *>(ret_call.argument.get());
                            auto return_data = scope.get_var(var_call.var_id);
                            auto first_scope_var = scope.get_min_pos_var_of_scope();

                            int needed_byte_len = std::get<0>(return_data.first);

                            if (needed_byte_len != 0) {
                                //pointer to return position can be easily aligned with return type
                                if (needed_byte_len == 4) {
                                    //TODO 1. swap ret_pos (first) with return var (whatever)
                                    //     2. swap ret_pos (whatever) to the place directly below ret_pos
                                    //     3. pop scope without popping return var and ret_pos
                                    //     4. execute call rel_goto
                                    //     5. after returning, pop ret_pos
                                    main_part.emplace_back(ByteCode::SWAP);
                                    put_4_num(main_part, 0);
                                    put_4_num(main_part, std::get<1>(return_data.first));
                                    put_4_num(main_part, 4);

                                    //if return variable and positional argument are not already aligned
                                    if (std::get<1>(return_data) != 4) {
                                        main_part.emplace_back(ByteCode::SWAP);
                                        put_4_num(main_part, 4);
                                        put_4_num(main_part, std::get<1>(return_data.first));
                                        put_4_num(main_part, 4);
                                    }

                                    scope.push_one_scope_above(return_data.first);
                                    scope.scope.back().erase(scope.scope.back().begin());
                                } else {
                                    //TODO
                                }
                            }
                        }
                            break;
                        case ActionType::FunctionCall: {
                            auto &fn_call = *static_cast<FunctionCallAction *>(ret_call.argument.get());
                        }
                            break;
                        case ActionType::StringConst:
                            break;
                        case ActionType::NumericConst:
                            break;
                    }
                }

                auto num = scope.get_current_total() - 4;
                if (num > 0) {
                    main_part.emplace_back(ByteCode::POP);
                    put_4_num(main_part, num);
                }

                main_part.emplace_back(ByteCode::REL_GOTO);
                scope.pop_scope();
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
                                                 bool &popped) {
    bcode.emplace_back(ByteCode::SWAP);
    put_4_num(bcode, std::get<1>(scope.get_min_pos_var_of_scope()));
    put_4_num(bcode, scope.get_current_total()-needed_byte_len);
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
    popped = true;
}

void Compiler::free_scope(StackScope &scope, std::vector<ByteCode> &bcode) {
    if (scope.scope.size() > 0) {
        uint32_t scope_total = scope.get_current_total();
        scope.pop_scope();
        bcode.emplace_back(ByteCode::POP);
        put_4_num(bcode, scope_total);
    }
}


void Compiler::display_code(std::vector<ByteCode> &code, int &num) {
    for (int i = 0; i < code.size(); i++) {
        switch (code[i]) {
            case ByteCode::PUSH: {
                std::cout << num << ". PUSH " << *((int32_t*)&code[++i]);
                i+=4;
                std::cout << " " << *((int32_t*)&code[i]) << "\n";
                i+=3; num+=9;
            }
                break;
            case ByteCode::POP: {
                std::cout << num << ". POP  " << *((int32_t*)&code[++i]) << "\n";
                i+=3; num+=5;
            }
                break;
            case ByteCode::SWAP: {
                std::cout << num << ". SWAP " << *((int32_t*)&code[++i]) << " ";
                i+=4;
                std::cout << " " << *((int32_t*)&code[i]) << " ";
                i+=4;
                std::cout << " " << *((int32_t*)&code[i]) << "\n";
                i+=3; num+=13;
            }
                break;
            case ByteCode::COPY_PUSH:
                std::cout << num << ". COPY_PUSH " << *((int32_t*)&code[++i]) << " ";
                i+=4;
                std::cout << *((uint32_t*)&code[i]) << "\n";
                i+=3; num+=9;
                break;
            case ByteCode::BUILTIN_CALL: {
                std::cout << num << ". BUILTIN_CALL " << std::get<0>(builtin_functions_id_names[*((int32_t*)&code[++i])]) << "\n";
                i+=3; num+=5;
            }
                break;
            case ByteCode::GOTO:
                std::cout << num << ". GOTO " << *((int32_t*)&code[++i]) << "\n";
                i+=3; num+=5;
                break;
            case ByteCode::REL_GOTO:
                std::cout << num << ". REL_GOTO\n"; num++;
                break;
            case ByteCode::PUSH_STACK_SCOPE:
                std::cout << num << ". PUSH_STACK_SCOPE\n"; num++;
                break;
            case ByteCode::POP_STACK_SCOPE:
                std::cout << num << ". POP_STACK_SCOPE\n"; num++;
                break;
            case ByteCode::PUSH_CURRENT_STACK_LEVEL:
                std::cout << num << ". PUSH_CURRENT_STACK_LEVEL\n"; num++;
                break;
            case ByteCode::GET_ABSOLUTE_POS:
                std::cout << num << ". GET_ABSOLUTE_POS\n"; num++;
            case ByteCode::COND_GOTO:
                break;
            case ByteCode::START_ARGUMENTS:
                std::cout << num << ". START_ARGUMENTS\n"; num++;
                break;
            case ByteCode::END_ARGUMENTS:
                std::cout << num << ". END_ARGUMENTS\n"; num++;
                break;
        }
    }
}

std::vector<ByteCode> Compiler::construct_program(std::vector<FunctionPart> &parts) {
    for (auto & part: parts) {optimize(part);}

    link_code_parts(parts);

    return concat_code(parts);
}

void Compiler::optimize(FunctionPart &part) {
    //TODO
}

void Compiler::link_code_parts(std::vector<FunctionPart> &parts) {
//    std::vector<FunctionPart> finished_parts;

    uint32_t total_size = 0;
    for (auto & part: parts) {
        part.len_before += total_size;
        total_size += part.fn_code.size();
    }

    //TODO map this
    for (auto & part: parts) {
        for (int i = 0; i < part.calls_to_custom.size(); i++) {
            uint32_t fn_call_pos;
            for (auto & _part: parts) {
                if (_part.id == part.calls_to_custom[i].second) {
                    fn_call_pos = _part.len_before; break;
                }
            }

            *(uint32_t*)&part.fn_code[part.calls_to_custom[i].first] = fn_call_pos;
            *(uint32_t*)&part.fn_code[part.calls_from_custom[i]] = part.len_before + part.parent_end_of_fn_call[i];
        }
    }
}

std::vector<ByteCode> Compiler::concat_code(std::vector<FunctionPart> & parts) {
    std::vector<ByteCode> to_return{};
    to_return.reserve(parts.back().len_before + parts.back().fn_code.size());

    for (auto & part: parts) {
        to_return.insert(to_return.end(), part.fn_code.begin(), part.fn_code.end());
    }
    return to_return;
}