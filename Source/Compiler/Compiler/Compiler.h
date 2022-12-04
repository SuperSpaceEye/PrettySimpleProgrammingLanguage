//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H
#define ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H

#include <vector>
#include <string>

#include "ByteCode.h"
#include "../Transpiler/Transpiler.h"
#include "../Parser/Parser.h"

struct StackScope {
    //byte_len, id, type
    std::vector<std::vector<std::tuple<uint32_t, uint32_t, VariableType>>> scope{{}};

    void push_scope() {scope.emplace_back();}
    std::vector<std::tuple<uint32_t, uint32_t, VariableType>> pop_scope() {
        auto temp_scope = scope.back();
        scope.pop_back();
        return temp_scope;
    }

    void push(uint32_t num, uint32_t id, VariableType type) {
        scope.back().emplace_back(num, id, type);
    }
    void push_one_scope_above(uint32_t num, uint32_t id, VariableType type) {
        scope[scope.size()-2].emplace_back(num, id, type);
    }
    //returns var and scope level
    std::pair<std::tuple<uint32_t, uint32_t, VariableType>, int> get_var(int id) {
        for (int i = scope.size()-1; i >= 0; i--) {
            for (auto & item: scope[i]) {
                if (std::get<1>(item) == id) {return {item, i};}
            }
        }
        throw std::logic_error("Shouldn't happen.");
    }
    std::tuple<uint32_t, uint32_t, VariableType> get_min_pos_var_of_scope() {
        return scope.back()[0];
    }
    int get_current_total() {
        int total = 0;
        for (auto item: scope.back()) {
            total += std::get<0>(item);
        }
        return total;
    }

    int get_total() {
        int total = 0;
        for (auto & _scope: scope) {
            for (auto & item: _scope) {
                total += std::get<0>(item);
            }
        }
        return total;
    }

    int get_pos(int id) {
        uint32_t pos = 0;
        for (auto & _scope: scope) {
            for (auto & item: _scope) {
                if (std::get<1>(item) == id) {
                    return pos;
                }

                pos += std::get<0>(item);
            }
        }

        throw std::logic_error("Shouldn't happen.");
    }

    StackScope()=default;
    StackScope(const StackScope & st)= default;
};

struct FunctionPart {
    // first token is entrance point to function
    std::vector<ByteCode> fn_code;

    // indexes in code with goto's to custom functions (must be resolved)
    // pos in code, id of fn
    std::vector<std::pair<uint32_t, int>> calls_to_custom;
    // indexes with goto's from custom functions (must be resolved)
    // pos in code
    std::vector<uint32_t> calls_from_custom;
    std::vector<uint32_t> parent_end_of_fn_call;
    std::vector<std::vector<uint32_t>> parent_end_of_fn_calls;

    std::vector<uint32_t> relative_gotos_inside_fn;

    int len_before = 0;
    int id = -1;
};

class Compiler {
    static std::vector<FunctionPart> compile_(TreeResult &tree_res);
    static void display_code(std::vector<ByteCode> &code, int &num, std::vector<int64_t> &delimiters);
    static void
    recursive_compile(FunctionPart &part, StackScope &scope, std::shared_ptr<BaseAction> &node, bool is_main,
                      int &do_not_push_scope, int user_nested_fn_call, int function_call_nesting,
                      int stack_frame_nesting);
    static void free_scope(StackScope &scope, std::vector<ByteCode> &bcode);

    static void
    generate_code_to_return_var_from_builtin(StackScope &scope, int needed_byte_len, std::vector<ByteCode> &bcode,
                                             bool &popped);

    static std::vector<ByteCode> construct_program(std::vector<FunctionPart> & parts);
    static void batch_pop_push(FunctionPart & part);
    static void link_code_parts(std::vector<FunctionPart> &parts);
    static std::vector<ByteCode> concat_code(std::vector<FunctionPart> &parts);
public:
    static std::vector<ByteCode> compile(const std::vector<std::string> &str_data, bool debug);

    static void
    prepare_return_from_user_fn(std::vector<ByteCode> &main_part, StackScope &local_scope, int needed_byte_len,
                                int return_val_pos);

    static void reverse_nested_user_fn_return_calls(FunctionPart &part);

    static void
    connect_function_calls_to_code_locations(const std::vector<FunctionPart> &parts, FunctionPart &part);

    static void fix_relative_gotos_inside_fns(FunctionPart &part);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H
