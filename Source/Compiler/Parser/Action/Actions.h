//
// Created by spaceeye on 29.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_ACTIONS_H
#define ANOTHERPROGRAMMINGLANGUAGE_ACTIONS_H

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
    bool is_declaration = false;
    bool reference = false;
};

struct ArrayVariable: public Variable {
    //if the type is array of arrays of ... of type.
    std::vector<VariableType> types;
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_ACTIONS_H
