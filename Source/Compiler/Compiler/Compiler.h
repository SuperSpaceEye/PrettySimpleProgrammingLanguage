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
    //byte_len, pos, id, type
    std::vector<std::vector<std::tuple<uint32_t, uint32_t, uint32_t, VariableType>>> scope{{}};

    //user function can have scopes inside it. To properly return from it, all allocated stack values
    //in scope should be removed.
    std::vector<int> scope_level{0};

    void push_scope() {scope.emplace_back();}
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t, VariableType>> pop_scope() {
        auto temp_scope = scope.back();
        scope.pop_back();
        return temp_scope;
    }
    void push_scope_level() {scope_level.emplace_back(scope.size());}
    void pop_scope_level() {scope_level.pop_back();}
    int get_current_scope_level() {return scope.size();}
    int get_entered_scope_level() {return scope_level.back();}
    int collapse_to_scope_level(){
        int times = 0;
        for (int scope_l = scope.size()-1; scope_l > scope_level.back()-1; scope_l--) {
            scope[scope_l-1].insert(scope[scope_l-1].end(), scope[scope_l].begin(), scope[scope_l].end());
            scope.pop_back();
            times++;
        }
        scope_level.pop_back();
        return times;
    }
    void push(uint32_t num, uint32_t pos, uint32_t id, VariableType type) {
        scope.back().emplace_back(num, pos, id, type);
    }
    void push_one_scope_above(uint32_t num, uint32_t pos, uint32_t id, VariableType type) {
        scope[scope.size()-2].emplace_back(num, pos, id, type);
    }
    void push_one_scope_above(std::tuple<uint32_t, uint32_t, uint32_t, VariableType> var) {
        scope[scope.size() - 2].push_back(var);

    }
    //returns var and scope level
    std::pair<std::tuple<uint32_t, uint32_t, uint32_t, VariableType>, int> get_var(int id) {
        for (int i = scope.size()-1; i >= 0; i--) {
            for (auto & item: scope[i]) {
                volatile int ahaha = std::get<2>(item);
                if (std::get<2>(item) == id) {return {item, i};}
            }
        }
        throw std::logic_error("Shouldn't happen.");
    }
    std::tuple<uint32_t, uint32_t, uint32_t, VariableType> get_min_pos_var_of_scope() {
        return scope.back()[0];
    }
    void delete_from_scope(int id) {
        for (int i = 0; i < scope.back().size(); i++) {
            auto & item = scope.back()[i];
            if (std::get<2>(item) == id) {
                scope.back().erase(scope.back().begin() + i);
                return;
            }
        }
    }
    int get_current_total() {
        int total = 0;
        for (auto item: scope.back()) {
            total += std::get<0>(item);
        }
        return total;
    }
    int get_total_scope(int level) {
        int total = 0;
        for (auto item: scope[level]) {
            total += std::get<0>(item);
        }
        return total;
    }

    int get_total_between(int start, int stop) {
        int total = 0;
        for (int i = start; i < stop; i++) {
            for (auto item: scope[i]) {
                total += std::get<0>(item);
            }
        }
        return total;
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

    std::vector<uint32_t> relative_gotos_inside_fn;

    int len_before = 0;
    int id = -1;
};

class Compiler {
    static std::vector<FunctionPart> compile_(TreeResult &tree_res);
    static void display_code(std::vector<ByteCode> &code, int &num);
    static void
    recursive_compile(FunctionPart &part, StackScope &scope, std::shared_ptr<BaseAction> &node, bool is_main,
                      int &do_not_push_scope);
    static void free_scope(StackScope &scope, std::vector<ByteCode> &bcode);

    static void
    generate_code_to_return_var_from_scope(StackScope &scope, int needed_byte_len, std::vector<ByteCode> &bcode,
                                           bool &popped);

    static std::vector<ByteCode> construct_program(std::vector<FunctionPart> & parts);
    static void optimize(FunctionPart & part);
    static void link_code_parts(std::vector<FunctionPart> &parts);
    static std::vector<ByteCode> concat_code(std::vector<FunctionPart> & parts);
public:
    static std::vector<ByteCode> compile(const std::vector<std::string> &str_data, bool debug);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H
