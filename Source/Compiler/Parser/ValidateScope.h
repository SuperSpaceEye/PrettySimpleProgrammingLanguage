//
// Created by spaceeye on 10.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_VALIDATESCOPE_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_VALIDATESCOPE_H

#include <iostream>
#include <exception>
#include <memory>
#include <vector>

#include "IdRegister.h"
#include "CommonFNS.h"
#include "../OptionsContainer.h"
#include "BuiltinFunctions.h"
#include "Action/Actions.h"
#include "../Transpiler/Transpiler.h"
#include "VariableType.h"
#include "IdRegister.h"

struct ValidateScope {
    std::vector<std::vector<std::tuple<int, VariableType>>> var_scope;
    std::vector<std::vector<std::tuple<int, std::vector<std::pair<VariableType, bool>>, VariableType>>> fn_scope{{}};
    void push_scope() {
        var_scope.emplace_back();
        fn_scope.emplace_back();
    }
    void pop_scope() {
        var_scope.pop_back();
        fn_scope.pop_back();
    }
    void push_id(int id, VariableType type) {
        var_scope.back().emplace_back(id, type);
    }
    void push_fn(int id, const std::vector<std::pair<VariableType, bool>>& arguments_types, VariableType return_type) {
        fn_scope.back().emplace_back(id, arguments_types, return_type);
    }
    bool check_id(int id) {
        for (int i = var_scope.size()-1; i >= 0; i--) {
            for (auto & sid: var_scope[i]) {
                if (std::get<0>(sid) == id) {return true;}
            }
        }
        return false;
    }
    bool check_fn_id(int id) {
        for (int i = fn_scope.size()-1; i >= 0; i--) {
            for (auto & sid: fn_scope[i]) {
                if (std::get<0>(sid) == id) {return true;}
            }
        }
        return false;
    }
    std::tuple<bool, const std::vector<std::pair<VariableType, bool>>&, VariableType> get_fn(int id) {
        if (!check_fn_id(id)) {throw std::logic_error("Function was not declared in this scope.");}
        for (int i = fn_scope.size()-1; i >= 0; i--) {
            for (auto & fn: fn_scope[i]) {
                if (std::get<0>(fn) == id) {
                    return {true, std::get<1>(fn), std::get<2>(fn)};
                }
            }
        }
        throw std::logic_error("Shouldn't happen. get_fn.");
    }
    VariableType get_var_type(int id) {
//        if (!check_id(id)) {throw std::logic_error("Variable was not declared in this scope.");}
        for (int i = var_scope.size()-1; i >= 0; i--) {
            for (auto & sid: var_scope[i]) {
                if (std::get<0>(sid) == id) {return std::get<1>(sid);}
            }
        }
        throw std::logic_error("Variable was not declared in this scope.");
    }
    bool check_id_in_lscope(int id) {
        for (auto & sid: var_scope.back()) {
            if (std::get<0>(sid) == id) {return true;}
        }
        return false;
    }

    void populate_builtins(IdRegister & reg) {
        for (int id: reg.builtins) {
            auto bnf = reg.get_builtin(id);

            std::vector<std::pair<VariableType, bool>> args;
            for (auto & arg: std::get<2>(bnf.second)) {args.emplace_back(arg.first, arg.second);}

            auto ret_type = std::get<1>(bnf.second);

            push_fn(id, args, ret_type);
        }
    }
};

#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_VALIDATESCOPE_H
