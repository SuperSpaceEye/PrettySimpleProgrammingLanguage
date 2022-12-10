//
// Created by spaceeye on 25.10.22.

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string> &str_data, const Options &options) {
    auto res = Transpiler::transpile_data(str_data);
    if (options.debug.show_transpiler_output) {Transpiler::display_tokens(res);}

    auto tree = Parser::create_tree(res, options);
    auto parts = compile_(tree);
    auto code = compile_to_bytecode(parts, options);

    if (options.debug.show_compiler_output) {
        int num = 0;

        std::vector<int64_t> delimiters;
        delimiters.reserve(parts.size());
        for (auto & part: parts) {delimiters.emplace_back(part.len_before);}
        delimiters.emplace_back(INT64_MAX);
        display_code(code, num, delimiters);
        std::cout << "\n";
    }

    return code;
}

int type_size(VariableType type) {
    switch (type) {
        case VariableType::VOID: return 0;
        case VariableType::NUMERIC_ANY: return 8;

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

    for (int i = 0; i < tree_res.object_roots.size(); i++) {
        auto scope = StackScope{};
        bool has_args = false;

        if (i != tree_res.main_idx) {
            scope.push_scope();
            //positional argument
            scope.push(4, -1, VariableType::UINT);

            auto &fn_call = *static_cast<FunctionDeclaration *>(tree_res.object_roots[i].get());
            for (auto &arg: fn_call.arguments) {
                int _type_size;
                //if reference
                if (std::get<1>(arg)) {
                    _type_size = 4;
                } else {
                    _type_size = type_size(std::get<0>(arg));
                }

                scope.push(_type_size, std::get<2>(arg), std::get<0>(arg));
                has_args = true;
            }
        }

        int do_not_push = (has_args ? 1: 0);

        auto node = tree_res.object_roots[i];
        code_parts.emplace_back();
        if (i == tree_res.main_idx) { code_parts.back().fn_code.emplace_back(ByteCode::PUSH_STACK_FRAME);}
        code_parts.back().id = static_cast<FunctionDeclaration*>(node.get())->fn_id;
        recursive_compile(code_parts.back(), scope, node, i == tree_res.main_idx, do_not_push, 0, 0, 0);
    }

    //so that main function would be the first.
    std::swap(code_parts[0], code_parts[tree_res.main_idx]);

    return code_parts;
}

//if function_call_nesting == 0 and return type of called function is not 0, then it will pop that unused value.
//if user_nested_fn_call is == 0 and the called function is user defined, then it will emplace_back FunctionPart's parent_end_of_fn_calls
// and put parent_end_of_fn_calls at the end of it. It is needed as at linking stage the order of parent_end_of_fn_calls
// needs to be reversed.
void
Compiler::recursive_compile(FunctionPart &part, StackScope &scope, std::shared_ptr<BaseAction> &node, bool is_main,
                            int &do_not_push_scope, int user_nested_fn_call, int function_call_nesting,
                            int stack_frame_nesting) {
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
                scope.push(num_b, var_d.var_id, var_d.var_type);
            }
                break;
            case ActionType::FunctionCall: {
                scope.push_scope();
                auto & fn_call = *static_cast<FunctionCallAction*>(node.get());

                main_part.emplace_back(ByteCode::PUSH_STACK_FRAME);
                stack_frame_nesting++;

                if (fn_call.fn_type == FunctionType::UserFunction) {
                    // return pos arg
                    main_part.emplace_back(ByteCode::PUSH);
                    put_4_num(main_part, 4);
                    part.calls_from_custom.emplace_back(main_part.size());
                    put_4_num(main_part, 0);
                    scope.push(4, -1, VariableType::UINT);

                    if (!user_nested_fn_call) {
                        user_nested_fn_call++;
                        part.parent_end_of_fn_calls.emplace_back();
                    }
                }

                for (int i = 0; i < fn_call.arguments.size(); i++) {
                    auto & arg = fn_call.arguments[i];
                    switch (arg->act_type) {
                        case ActionType::VariableCall: {
                            auto & var_call = *static_cast<VariableCall*>(arg.get());
                            auto [data, scope_level] = scope.get_var(var_call.var_id);

                            uint32_t var_size = std::get<0>(data);

                            uint32_t var_pos = scope.get_pos(std::get<1>(data));

                            //if var is reference, then function will modify stack pos directly.
                            //if ref of b_any then must also place type information

                            if (arg_is_ref(fn_call, i)) {
                                if (fn_call.required_arguments[i] == VariableType::NUMERIC_ANY) {
                                    main_part.emplace_back(ByteCode::PUSH);
                                    put_4_num(main_part, 4);
                                    put_4_num(main_part, (uint32_t)var_call.type);
                                    scope.push(4, -1, VariableType::UINT);
                                }

                                //Will push absolute pos while popping prev 2 numbers.
                                main_part.emplace_back(ByteCode::GET_ABSOLUTE_POS);
                                put_4_num(main_part, var_pos);
                                put_4_num(main_part, stack_frame_nesting);

                                scope.push(4, -1, VariableType::UINT);
                            //if not ref, then make copy of stack variable
                            } else {
                                if (fn_call.required_arguments[i] == VariableType::NUMERIC_ANY) {
                                    main_part.emplace_back(ByteCode::PUSH);
                                    put_4_num(main_part, 4);
                                    put_4_num(main_part, (uint32_t)var_call.type);
                                    scope.push(4, -1, VariableType::UINT);
                                }

                                main_part.emplace_back(ByteCode::COPY_PUSH);
                                put_4_num(main_part, var_pos);
                                put_4_num(main_part, var_size);
                                put_4_num(main_part, stack_frame_nesting);
                                scope.push(4, -1, VariableType::UINT);
                            }
                        }
                            break;
                        case ActionType::FunctionCall: {
                            if (fn_call.required_arguments[i] == VariableType::NUMERIC_ANY) {
                                auto &arg_fn_call = *static_cast<FunctionCallAction*>(arg.get());
                                main_part.emplace_back(ByteCode::PUSH);
                                put_4_num(main_part, 4);
                                put_4_num(main_part, (uint32_t)arg_fn_call.return_type);
                                scope.push(4, -1, VariableType::UINT);
                            }

                            //If nested function call, then just recursively process it, as returned value will be on
                            //top of the stack.
                            auto rec_arg = arg;
                            recursive_compile(part, scope, rec_arg, false, do_not_push_scope, user_nested_fn_call,
                                              function_call_nesting + 1, stack_frame_nesting);
                        }
                            break;
                        //If number const, then just push value saved in code to stack.
                        case ActionType::NumericConst: {
                            auto & num_call = *static_cast<NumericConst*>(arg.get());
                            if (fn_call.required_arguments[i] == VariableType::NUMERIC_ANY) {
                                main_part.emplace_back(ByteCode::PUSH);
                                put_4_num(main_part, 4);
                                put_4_num(main_part, (uint32_t)num_call.type);
                                scope.push(4, -1, VariableType::UINT);
                            }

                            main_part.push_back(ByteCode::PUSH);
                            put_4_num(main_part, 4);
                            put_4_num(main_part, num_call.value);
                            scope.push(4, -1, VariableType::INT);
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
                        scope.push(needed_byte_len, 0, VariableType::UINT);

                        generate_code_to_return_var_from_builtin(scope, needed_byte_len, main_part, popped);
                    }
                    if (!popped) { free_scope(scope, main_part);}
                } else {
                    if (fn_call.return_type != VariableType::VOID) {
                        scope.push_one_scope_above(type_size(fn_call.return_type), -1, VariableType::UINT);
                    }
                    scope.pop_scope();

                    main_part.emplace_back(ByteCode::GOTO);
                    part.calls_to_custom.emplace_back(main_part.size(), fn_call.fn_id);
                    put_4_num(main_part, 0);

                    part.parent_end_of_fn_calls.back().emplace_back(main_part.size());

                    user_nested_fn_call--;
                }
                main_part.emplace_back(ByteCode::POP_STACK_FRAME);
                stack_frame_nesting--;

                if (fn_call.return_type != VariableType::VOID && !function_call_nesting) {
                    auto num = type_size(fn_call.return_type);

                    main_part.emplace_back(ByteCode::POP);
                    put_4_num(main_part, num);

                    scope.scope.back().erase(scope.scope.back().end());
                }
            }
                break;
            case ActionType::FunctionDeclaration:
                break;
            case ActionType::ForLoop:
                break;
            case ActionType::WhileLoop: {
                auto & while_act = *static_cast<WhileLoop*>(node.get());

                //save pos value of start to return if expr is true
                uint32_t while_loop_start_pos = main_part.size();

                //generate code for expression
                recursive_compile(part, scope, while_act.expression, is_main, do_not_push_scope, 0, 1, stack_frame_nesting);

                //check expression
                main_part.emplace_back(ByteCode::COND_GOTO);
                uint32_t false_expr_goto_pos = main_part.size();
                part.relative_gotos_inside_fn.emplace_back(false_expr_goto_pos);
                put_4_num(main_part, 0);

                //expression result will be removed by the cond_goto.
                scope.scope.back().pop_back();

                auto local_scope = StackScope{scope};
                //generate code for body of while loop
                recursive_compile(part, local_scope, while_act.body, is_main, do_not_push_scope, 0, 0, stack_frame_nesting);

                //go back to the start
                main_part.emplace_back(ByteCode::GOTO);
                part.relative_gotos_inside_fn.emplace_back(main_part.size());
                put_4_num(main_part, while_loop_start_pos);

                //if expression is false, it will go
                *(uint32_t*)&main_part[false_expr_goto_pos] = main_part.size();
            }
                break;
            case ActionType::IfStatement: {
                auto & if_st = *static_cast<IfStatement*>(node.get());

                //Generate code for argument
                switch (if_st.expression->act_type) {
                    case ActionType::VariableCall: {
                        auto & var_call = *static_cast<VariableCall*>(if_st.expression.get());
                        auto [data, scope_level] = scope.get_var(var_call.var_id);

                        uint32_t var_size = std::get<0>(data);

                        int var_pos = scope.get_pos(std::get<1>(data));

                        main_part.emplace_back(ByteCode::COPY_PUSH);
                        put_4_num(main_part, var_pos);
                        put_4_num(main_part, var_size);
                        put_4_num(main_part, stack_frame_nesting);
                        scope.push(4, -1, VariableType::UINT);
                    }
                        break;
                    case ActionType::FunctionCall: {
                        auto arg = if_st.expression;
                        recursive_compile(part, scope, arg, false, do_not_push_scope, 0, 1, stack_frame_nesting);
                    }
                        break;
                }
                //Argument will be popped by COND_GOTO
                scope.scope.back().pop_back();
                main_part.emplace_back(ByteCode::COND_GOTO);
                part.relative_gotos_inside_fn.emplace_back(main_part.size());
                uint32_t p_to_false_expr = main_part.size();
                put_4_num(main_part, 4);

                auto local_scope = StackScope{scope};

                // generate code for true branch
                auto true_br = if_st.true_branch;
                recursive_compile(part, local_scope, true_br, false, do_not_push_scope, 0, 0, stack_frame_nesting);
                // goto to jump over the code of the false branch
                main_part.emplace_back(ByteCode::GOTO);
                part.relative_gotos_inside_fn.emplace_back(main_part.size());
                uint32_t p_to_true_expr_end = main_part.size();
                put_4_num(main_part, 4);

                local_scope = StackScope{scope};

                auto true_br_end = main_part.size();
                // generate code for false branch
                auto false_br = if_st.false_branch;
                recursive_compile(part, local_scope, false_br, false, do_not_push_scope, 0, 0, stack_frame_nesting);
                uint32_t end_br_pos = main_part.size();

                // changes pos of the failed expr goto to the end of true branch pos.
                *((uint32_t*)&main_part[p_to_false_expr]) = true_br_end;
                // changes pos of the goto after true branch to end of false branch.
                *((uint32_t*)&main_part[p_to_true_expr_end])  = end_br_pos;
            }
                break;
            case ActionType::StartLogicBlock:
                if (!do_not_push_scope) {
                    scope.push_scope();
                } else {do_not_push_scope--;}
                break;
            case ActionType::EndLogicBlock: {
                free_scope(scope, main_part);
            }
                break;
            //if there is a return call then we assume that there is a return positional element at the bottom of the stack frame.
            case ActionType::ReturnCall: {
                //if call from main then just stop execution
                if (is_main) {
                    main_part.emplace_back(ByteCode::GOTO);
                    put_4_num(main_part, UINT32_MAX);
                    break;
                }

                auto local_scope = StackScope(scope);

                auto & ret_call = *static_cast<ReturnCall*>(node.get());

                if (ret_call.argument.get() != nullptr) {
                    int needed_byte_len;
                    int return_val_pos;

                    switch (ret_call.argument->act_type) {
                        case ActionType::VariableCall: {
                            auto &var_call = *static_cast<VariableCall *>(ret_call.argument.get());
                            auto return_data = local_scope.get_var(var_call.var_id);
                            auto first_scope_var = local_scope.get_min_pos_var_of_scope();

                            needed_byte_len = std::get<0>(return_data.first);
                            return_val_pos = local_scope.get_pos(var_call.var_id);
                        }
                            break;
                        case ActionType::FunctionCall: {
                            auto &fn_call = *static_cast<FunctionCallAction*>(ret_call.argument.get());

                            return_val_pos = local_scope.get_total();
                            needed_byte_len = type_size(fn_call.return_type);

                            auto in_arg = ret_call.argument;
                            recursive_compile(part, local_scope, in_arg, is_main, do_not_push_scope,
                                              user_nested_fn_call, function_call_nesting + 1, stack_frame_nesting);
                        }
                            break;
                        case ActionType::StringConst:
                            throw std::logic_error("Not implemented.");
                        case ActionType::NumericConst: {
                            auto &num_const = *static_cast<NumericConst*>(ret_call.argument.get());
                            return_val_pos = local_scope.get_current_total();
                            needed_byte_len = type_size(num_const.type);

                            main_part.emplace_back(ByteCode::PUSH);
                            put_4_num(main_part, 4);
                            put_4_num(main_part, num_const.value);
                        }
                            break;
                    }

                    prepare_return_from_user_fn(main_part, local_scope, needed_byte_len, return_val_pos);
                }

                //return pos will be cleaned up by rel_goto
                auto num = local_scope.get_current_total() - 4;
                if (num > 0) {
                    main_part.emplace_back(ByteCode::POP);
                    put_4_num(main_part, num);
                }

                main_part.emplace_back(ByteCode::REL_GOTO);
                local_scope.pop_scope();
            }
                break;
        }

        node = node->next_action;
    }
}

void
Compiler::prepare_return_from_user_fn(std::vector<ByteCode> &main_part, StackScope &local_scope, int needed_byte_len,
                                      int return_val_pos) {
    if (needed_byte_len != 0) {
        // 1. swap ret_pos (first) with return var (whatever)
        // 2. swap ret_pos (whatever) to the place directly below ret_pos
        // 3. pop scope without popping return var and ret_pos
        // 4. execute call REL_GOTO. REL_GOTO will pop return position after execution.

        main_part.emplace_back(ByteCode::SWAP);
        put_4_num(main_part, 0);
        put_4_num(main_part, return_val_pos);
        put_4_num(main_part, needed_byte_len);

        //if positional argument is not already at the position
        if (return_val_pos != 4) {
            //I guess it can be replaced with set commands
            main_part.emplace_back(ByteCode::SWAP);
            put_4_num(main_part, needed_byte_len);
            put_4_num(main_part, return_val_pos);
            put_4_num(main_part, 4);
        }

        local_scope.push_one_scope_above(needed_byte_len, -1, VariableType::UINT);
        local_scope.scope.back().erase(local_scope.scope.back().begin());
    }
}

void
Compiler::generate_code_to_return_var_from_builtin(StackScope &scope, int needed_byte_len, std::vector<ByteCode> &bcode,
                                                   bool &popped) {
    bcode.emplace_back(ByteCode::SWAP);
    put_4_num(bcode, 0);
    put_4_num(bcode, scope.get_current_total()-needed_byte_len);
    put_4_num(bcode, needed_byte_len);

    int scope_total = scope.get_current_total();
    if (needed_byte_len > scope_total) {
        bcode.emplace_back(ByteCode::PUSH);
        put_4_num(bcode, 4);
        put_4_num(bcode, needed_byte_len - scope_total);
        scope.push(needed_byte_len - scope_total, 0, VariableType::INT);
    }

    scope_total = scope.get_current_total();

    if (scope_total != needed_byte_len) {
        bcode.emplace_back(ByteCode::POP);
        put_4_num(bcode, scope_total - needed_byte_len);
        scope.push_one_scope_above(needed_byte_len, -1, VariableType::INT);
    }
    scope.pop_scope();
    popped = true;
}

void Compiler::free_scope(StackScope &scope, std::vector<ByteCode> &bcode) {
    if (scope.scope.size() > 0) {
        uint32_t scope_total = scope.get_current_total();
        scope.pop_scope();
        if (scope_total > 0) {
            bcode.emplace_back(ByteCode::POP);
            put_4_num(bcode, scope_total);
        }
    }
}


void Compiler::display_code(std::vector<ByteCode> &code, int &num, std::vector<int64_t> &delimiters) {
    int del_pos = 0;
    for (int i = 0; i < code.size(); i++) {
        if (i >= delimiters[del_pos]) {
            std::cout << "\n";del_pos++;
        }
        switch (code[i]) {
            case ByteCode::PUSH: {
                std::cout << num << ". PUSH " << *((int32_t*)&code[++i]);
                auto num_b = *(uint32_t*)&code[i];
                i+=4;
                for (int ii = 0; ii < num_b; ii+=4) {
                    std::cout << " " << *((int32_t*)&code[i+ii]);
                }
                std::cout << "\n";
                i+=num_b-1; num+=5+num_b;
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
                std::cout << num << ". COPY_PUSH " << *((int32_t*)&code[++i]) << " "; i+=4;
                std::cout << *((uint32_t*)&code[i]) << " "; i+=4;
                std::cout << *((int32_t*)&code[i]) << "\n"; i+=3;
                num+=13;
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
            case ByteCode:: COND_GOTO: {
                std::cout << num << ". COND_GOTO " << *((int32_t*)&code[++i]) << "\n";
                i+=3; num+=5;
            }
                break;
            case ByteCode::PUSH_STACK_FRAME:
                std::cout << num << ". PUSH_STACK_FRAME\n"; num++;
                break;
            case ByteCode::POP_STACK_FRAME:
                std::cout << num << ". POP_STACK_FRAME\n"; num++;
                break;
            case ByteCode::GET_ABSOLUTE_POS:
                std::cout << num << ". GET_ABSOLUTE_POS";
                std::cout << " " << *((int32_t*)&code[++i]); i+=4;
                std::cout << " " << *((int32_t*)&code[i]); i+=3;
                std::cout << "\n";
                num+=9;
                break;
        }
    }
}

std::vector<ByteCode> Compiler::compile_to_bytecode(std::vector<FunctionPart> &parts, const Options &options) {
    if (options.optimization.batch_byte_words) {for (auto &part: parts) { batch_byte_words(part); }}

    link_code_parts(parts);

    return concat_code(parts);
}

std::pair<bool, uint32_t*> is_in_any(uint32_t pos, FunctionPart & part) {
    for (auto & litem: part.parent_end_of_fn_calls) {
        for (auto & item: litem) {
            if (item == pos) {
                return {true, &item};
            }
        }
    }
    for (auto & item: part.calls_to_custom) {
        if (item.first == pos) {
            return {true, &item.first};
        }
    }
    for (auto & item: part.calls_from_custom) {
        if (item == pos) {
            return {true, &item};
        }
    }
    return {false, nullptr};
}

//after deletion of code the positions of values (and relative values themselves) are now point at wrong locations, and
//must be fixed
void norm_items(uint32_t start_pos, uint32_t removed, FunctionPart & part) {
    if (!removed) { return;}
    for (auto & item: part.relative_gotos_inside_fn) {
        //fixing the number of the position of the value in code.
        if (item > start_pos) {
            item -= removed;
        }
        //whenever or not the position of the value was fixed or not, we need
        //to check if the relative position needs to be fixed.
        if (*(uint32_t*)&part.fn_code[item] >= start_pos) {
            *(uint32_t *)&part.fn_code[item] -= removed;
        }
    }
    for (auto & litem: part.parent_end_of_fn_calls) {
        for (auto & item: litem) {
            if (item > start_pos) {
                item -= removed;
            }
        }
    }
    for (auto & item: part.calls_to_custom) {
        if (item.first > start_pos) {
            item.first -= removed;
        }
    }
    for (auto & item: part.calls_from_custom) {
        if (item > start_pos) {
            item -= removed;
        }
    }
}

void Compiler::batch_byte_words(FunctionPart &part) {
    uint32_t cur = 0;

    ByteCode last_command = ByteCode::REL_GOTO;
    int start_command = -1;

    std::vector<uint8_t> data{};
    std::vector<std::pair<uint32_t*, uint32_t>> additional_data;

    auto & bcode = part.fn_code;

    while (cur < bcode.size()) {
        auto word = bcode[cur];

        if (last_command != word) {
            if (word == ByteCode::PUSH) {
                start_command = cur;
                last_command = word;
                continue;
            }

            if (!data.empty()) {
                uint32_t fused_number = data.size();
                auto fused_size = 1 + 4 + fused_number;

                auto removed_num = cur - start_command - fused_size;

                cur -= removed_num;

                bcode.erase(bcode.begin()+start_command+fused_size,
                            bcode.begin()+start_command+fused_size+removed_num);

                *(uint32_t*)&bcode[start_command+1] = fused_number;

                int i = start_command+5;
                for (auto item: data) {
                    bcode[i] = (ByteCode)item;
                    i++;
                }

                data.clear();
                last_command = word;

                norm_items(start_command, removed_num, part);

                for (auto & item: additional_data) {
                    *item.first = start_command + 1 + 4 + item.second;
                }
                additional_data.clear();

                continue;
            }
        }

        switch (word) {
            case ByteCode::PUSH: {
                cur++;
                auto num = *(uint32_t*)&part.fn_code[cur];
                cur+=4;

                auto res = is_in_any(cur, part);
                if (res.first) {additional_data.emplace_back(res.second, data.size());}

                for (int i = 0; i < num; i++) {
                    data.emplace_back((uint8_t)part.fn_code[cur]);
                    cur++;
                }
            }
                break;
            //TODO do the same optimization for pop's?
            case ByteCode::POP: {
                cur += 5;
            }
                break;
            case ByteCode::SWAP:             cur+=13;break;
            case ByteCode::COPY_PUSH:        cur+=13;break;
            case ByteCode::BUILTIN_CALL:     cur+=5;break;
            case ByteCode::GOTO:             cur+=5;break;
            case ByteCode::COND_GOTO:        cur+=5;break;
            case ByteCode::REL_GOTO:         cur++;break;
            case ByteCode::GET_ABSOLUTE_POS: cur+=9;break;
            case ByteCode::PUSH_STACK_FRAME: cur++;break;
            case ByteCode::POP_STACK_FRAME:  cur++;break;
        }
    }
}

void Compiler::link_code_parts(std::vector<FunctionPart> &parts) {
    //calculate where a part will start in the code
    uint32_t total_size = 0;
    for (auto & part: parts) {
        part.len_before += total_size;
        total_size += part.fn_code.size();
    }

    for (auto & part: parts) {
        reverse_nested_user_fn_return_calls(part);
        connect_function_calls_to_code_locations(parts, part);
        fix_relative_gotos_inside_fns(part);
    }
}

void Compiler::reverse_nested_user_fn_return_calls(FunctionPart &part) {
    for (int i = 0; i < part.parent_end_of_fn_calls.size(); i++) {
        for (int ii = part.parent_end_of_fn_calls[i].size()-1; ii >= 0; ii--) {
            part.parent_end_of_fn_call.emplace_back(
                    part.parent_end_of_fn_calls[i][ii]);
        }
    }
}

//TODO map this
void Compiler::connect_function_calls_to_code_locations(const std::vector<FunctionPart> &parts, FunctionPart &part) {
    for (int i = 0; i < part.calls_to_custom.size(); i++) {
        uint32_t fn_call_pos;
        for (int _i = parts.size()-1; _i >= 0; _i--) {
            auto & _part = parts[_i];
            if (_part.id == part.calls_to_custom[i].second) {
                fn_call_pos = _part.len_before; break;
            }
        }

        *(uint32_t*)&part.fn_code[part.calls_to_custom[i].first] = fn_call_pos;
        *(uint32_t*)&part.fn_code[part.calls_from_custom[i]] = part.len_before + part.parent_end_of_fn_call[i];
    }
}

void Compiler::fix_relative_gotos_inside_fns(FunctionPart &part) {
    for (auto & pos: part.relative_gotos_inside_fn) {
        uint32_t rel_pos = *((uint32_t*)&part.fn_code[pos]);
        rel_pos += part.len_before;
        *((uint32_t*)&part.fn_code[pos]) = rel_pos;
    }
}

std::vector<ByteCode> Compiler::concat_code(std::vector<FunctionPart> &parts) {
    std::vector<ByteCode> to_return{};
    to_return.reserve(parts.back().len_before + parts.back().fn_code.size());

    for (auto & part: parts) {
        to_return.insert(to_return.end(), part.fn_code.begin(), part.fn_code.end());
    }
    return to_return;
}