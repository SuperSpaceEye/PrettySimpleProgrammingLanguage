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
    std::pair<int, int> pos;
};

//access through dynamic cast
struct FunctionCallAction: public BaseAction {
    FunctionType fn_type;
    int fn_id;
    int name_id;
    VariableType return_type;
    std::vector<std::shared_ptr<BaseAction>> arguments;
    std::vector<VariableType> required_arguments;
};

struct IfStatement: public BaseAction {
    std::shared_ptr<BaseAction> expression;
    std::shared_ptr<BaseAction> true_branch;
    std::shared_ptr<BaseAction> false_branch;
};

struct WhileLoop: public BaseAction {
    std::shared_ptr<BaseAction> expression;
    std::shared_ptr<BaseAction> body;
};

struct ForLoop: public BaseAction {
    std::shared_ptr<BaseAction> before_logic;
    std::shared_ptr<BaseAction> expression;
    std::shared_ptr<BaseAction> post_logic;
    std::shared_ptr<BaseAction> body;
};

struct ReturnCall: public BaseAction {
    std::shared_ptr<BaseAction> argument = nullptr;
};

struct FunctionDeclaration: public BaseAction {
    int fn_id;
    bool is_inline;
    VariableType return_type;
    //type, is_ref, id
    std::vector<std::tuple<VariableType, bool, int>> arguments;
};

struct VariableDeclaration: public BaseAction {
    VariableType var_type;
    int var_id;
    bool reference = false;
};

struct VariableCall: public BaseAction {
    int var_id;
    VariableType type;
};

struct NumericConst: public BaseAction {
    VariableType type;
    //Not actual type, just a 4 byte space to represent it.
    uint32_t value;
};

struct StringConst: public BaseAction {
    std::vector<char> data;
};

struct ArrayVariable: public VariableDeclaration {
    //if the type is array of arrays of ... of type.
    std::vector<VariableType> types;
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_ACTIONS_H
