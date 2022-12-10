//
// Created by spaceeye on 10.12.22.
//

#include "../IdRegister.h"

void IdRegister::register_function(int id, FunctionDeclaration *fn_dec) {
    for (auto & ids: variable_ids) {
        if (std::get<0>(ids) == id) {throw std::logic_error{"Variable with id already exists."};}
    }

    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) {throw std::logic_error{"Function with id already exists."};}
    }

    function_ids.emplace_back(id, wreg.get_word((Token)id), fn_dec);
}

void IdRegister::register_variable(int id) {
    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) {throw std::logic_error{"Function with id already exists."};}
    }

    for (auto & ids: variable_ids) {
        if (ids.first == id) {
            return;
        }
    }

    variable_ids.emplace_back(id, wreg.get_word((Token)id));
}

bool IdRegister::check_function(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {return false;}
    }

    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) { return true;}
    }

    throw std::logic_error{"No function was declared."};
}

bool IdRegister::check_exists(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {return true;}
    }

    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) { return true;}
    }

    return false;
}

bool IdRegister::check_variable(int id) {
    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) {return false;}
    }

    for (auto & ids: variable_ids) {
        if (ids.first == id) { return true;}
    }
    throw std::logic_error{"VariableDeclaration with id wasn't declared."};
}

bool IdRegister::is_builtin_fn(int id) {
    for (auto item: builtins) {
        if (item == id) {
            return true;
        }
    }
    return false;
}

std::pair<int, const std::tuple<std::string, VariableType, std::vector<std::pair<VariableType, bool>>>&>
IdRegister::get_builtin(int id) {
    for (auto & item: function_ids) {
        if (std::get<0>(item) == id) {
            for (int i = 0; i < builtin_functions_id_names.size(); i++) {
                auto & bitem = builtin_functions_id_names[i];
                if (std::get<0>(bitem) == std::get<1>(item)) {
                    return {i, builtin_functions_id_names[i]};
                }
            }
        }
    }
    throw std::logic_error("shouldn't happen");
}

FunctionDeclaration *IdRegister::get_user_def_fn(int id) {
    for (auto & item: function_ids) {
        if (std::get<0>(item) == id) {
            return std::get<2>(item);
        }
    }
    return nullptr;
}