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
    void free_var_from_popping(int id) {
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

class Compiler {
    static std::vector<ByteCode> compile_(ASTCreationResult & ast);
    static void display_code(std::vector<ByteCode> & code);
    static void recursive_compile(std::vector<ByteCode> &bcode, StackScope &scope, int stack_size, std::shared_ptr<BaseAction> &node);
public:
    static std::vector<ByteCode> compile(const std::vector<std::string> &str_data, bool debug);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H
