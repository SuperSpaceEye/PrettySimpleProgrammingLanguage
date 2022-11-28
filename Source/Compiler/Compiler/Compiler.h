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
    //num, pos, id, type
    std::vector<std::vector<std::tuple<uint32_t, uint32_t, uint32_t, VariableType>>> scope{{}};

    void push_scope() {scope.emplace_back();}
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t, VariableType>> pop_scope() {
        auto temp_scope = scope.back();
        scope.pop_back();
        return temp_scope;
    }
    void push(uint32_t num, uint32_t pos, uint32_t id, VariableType type) {
        scope.back().emplace_back(num, pos, id, type);
    }
    void push_one_scope_above(uint32_t num, uint32_t pos, uint32_t id, VariableType type) {
        scope[scope.size()-2].emplace_back(num, pos, id, type);
    }
    std::tuple<uint32_t, uint32_t, uint32_t, VariableType> get_var(int id) {
        for (int i = scope.size()-1; i > 0; i--) {
            for (auto & item: scope[i]) {
                if (std::get<2>(item) == id) {return item;}
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
};

struct CustomFunctionData {
    //start of the position of the value in code that user function will use
    //to goto back to parent fn
    uint32_t pos_from_function_val;
    //start of the position of the value in code that parent function will use to
    //goto to the user function
    uint32_t pos_to_function_val;
};

struct FunctionPart {
    // first token is entrance point to function
    std::vector<ByteCode> main_part;
    // code that should be executed after returned to parent function.
    std::vector<ByteCode> return_part;
    // indexes with goto's to custom functions (must be resolved)
    std::vector<CustomFunctionData> custom_function_calls;

    int id;
};

class Compiler {
    static std::vector<FunctionPart> compile_(TreeResult &tree_res);
    static void display_code(std::vector<ByteCode> & code);
    static void
    recursive_compile(FunctionPart &part, StackScope &scope, int &stack_size, std::shared_ptr<BaseAction> &node,
                      bool is_main, int &do_not_push_scope);
    static void free_scope(StackScope &scope, std::vector<ByteCode> &bcode, int &stack_size);

    static void
    generate_code_to_return_var_from_scope(StackScope &scope, int needed_byte_len, std::vector<ByteCode> &bcode,
                                           int &stack_size, bool &popped);

    static void configure_main_fn(FunctionPart & part) {}
    static void link_custom_functions(std::vector<FunctionPart> & parts);
    static std::vector<ByteCode> concat_code(std::vector<FunctionPart> & parts);
    //TODO
    static std::vector<ByteCode> optimize(std::vector<ByteCode> & code) {return code;}
public:
    static std::vector<ByteCode> compile(const std::vector<std::string> &str_data, bool debug);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H
