//
// Created by spaceeye on 10.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_IDREGISTER_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_IDREGISTER_H

#include <iostream>
#include <exception>
#include <memory>

#include "CommonFNS.h"
#include "BuiltinFunctions.h"
#include "../Transpiler/Transpiler.h"
#include "Action/Actions.h"

class IdRegister {
public:
    std::vector<std::tuple<int, std::string, FunctionDeclaration*>> function_ids;
    std::vector<std::pair<int, std::string>> variable_ids;
    std::vector<int> builtins;
    WordRegister & wreg;

    IdRegister(WordRegister & wreg): wreg(wreg) {
        for (int i = 0; i < wreg.names.size(); i++) {
            auto & item = wreg.names[i];
            for (auto & bitem: builtin_functions_id_names) {
                if (std::get<0>(bitem) == item) {
                    function_ids.emplace_back(i, std::get<0>(bitem), nullptr);
                    builtins.emplace_back(i);
                }
            }
        }
    };

    void register_function(int id, FunctionDeclaration *fn_dec);
    void register_variable(int id);

    bool check_exists(int id);

    bool check_function(int id);
    bool check_variable(int id);

    bool is_builtin_fn(int id);

    std::pair<int, const std::tuple<std::string, VariableType, std::vector<std::pair<VariableType, bool>>>&> get_builtin(int id);
    FunctionDeclaration * get_user_def_fn(int id);

    std::string id_to_string(int id) {
        for (auto reg: function_ids) {if (std::get<0>(reg) == id) { return "fn \"" + std::get<1>(reg) + "\"";}}
        for (auto reg: variable_ids) {if (reg.first == id) { return "var \"" + reg.second + "\"";}}
        return "unknown id";
    }
};

#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_IDREGISTER_H
