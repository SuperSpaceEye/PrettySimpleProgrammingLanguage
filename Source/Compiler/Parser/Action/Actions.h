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
    std::shared_ptr<BaseAction> next_action = nullptr;
};

//access through dynamic cast
struct FunctionCallAction: public BaseAction {
    FunctionType fn_type;
    int fn_id;
    int name_id;
    VariableType return_type;
    std::vector<std::shared_ptr<BaseAction>> arguments;
};

struct FunctionDeclaration: public BaseAction {
    int fn_id;
    bool is_inline;
    VariableType return_type;
    //type, is_ref, id
    std::vector<std::tuple<VariableType, bool, int>> arguments;
};

struct Variable: public BaseAction {
    VariableType var_type;
    int var_id;
    bool is_declaration = false;
    bool reference = false;
};

struct NumericConst: public BaseAction {
    VariableType type;
    //Not actual type, just a 4 byte space to represent it.
    uint32_t value;
};

struct ArrayVariable: public Variable {
    //if the type is array of arrays of ... of type.
    std::vector<VariableType> types;
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_ACTIONS_H
