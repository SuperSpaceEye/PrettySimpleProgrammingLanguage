//
// Created by spaceeye on 29.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_BASEACTION_H
#define ANOTHERPROGRAMMINGLANGUAGE_BASEACTION_H

#include <vector>
#include "OperationType.h"
#include "../VariableType.h"

struct BaseAction {
    ActionType act_type;
    BaseAction * left_action = nullptr;
    BaseAction * right_action = nullptr;
};

//access through dynamic cast
struct FunctionCallAction: public BaseAction {
    FunctionType fn_type;
    int fn_id;
    VariableType return_type;
    std::vector<BaseAction*> arguments;
};

struct FunctionDeclaration: public BaseAction {
    int fn_id;
    bool is_inline;
    VariableType return_type;
    std::vector<std::pair<VariableType, int>> arguments;
};

struct Variable: public BaseAction {
    VariableType var_type;
    int var_id;
    bool id_declaration = false;
    bool reference = false;
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_BASEACTION_H
