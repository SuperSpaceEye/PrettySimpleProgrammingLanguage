//
// Created by spaceeye on 01.11.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
#define ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H

#include <vector>
#include <string>
#include <tuple>


#include "VariableType.h"

enum class BuiltinIDS{
    ASGN,
    ADD,
    SUB,
    MUL,
    DIV,

    ASGN_ADD,
    ASGN_SUB,
    ASGN_MUL,
    ASGN_DIV,

    AND,
    OR,
    EQUALS,
    NOT_EQUALS,
    NOT,
    MORE_EQUALS,
    LESS_EQUALS,
    MORE,
    LESS,

    CONCAT,
    TO_STR,
    PRINT,
    TO_INT,
    TO_UINT,
    TO_FLOAT,
    RETURN,
    INPUT,
    PRINT_NUM,
};

//name, return type, arguments (type, ref)
const std::vector<std::tuple<std::string, VariableType, std::vector<std::pair<VariableType, bool>>>> builtin_functions_id_names{
//        TEMP FUNCTIONS BEFORE I IMPLEMENT PROPER ALGEBRAIC FUNCTIONS
        {"asgn",     VariableType::VOID,        {{VariableType::NUMERIC_ANY, true},  {VariableType::NUMERIC_ANY, false}}},
        {"add",      VariableType::NUMERIC_ANY, {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"sub",      VariableType::NUMERIC_ANY, {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"mul",      VariableType::NUMERIC_ANY, {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"div",      VariableType::NUMERIC_ANY, {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},

        {"asgn_add", VariableType::VOID,        {{VariableType::NUMERIC_ANY, true},  {VariableType::NUMERIC_ANY, false}}},
        {"asgn_sub",  VariableType::VOID,       {{VariableType::NUMERIC_ANY, true},  {VariableType::NUMERIC_ANY, false}}},
        {"asgn_mul",  VariableType::VOID,       {{VariableType::NUMERIC_ANY, true},  {VariableType::NUMERIC_ANY, false}}},
        {"asgn_div",  VariableType::VOID,       {{VariableType::NUMERIC_ANY, true},  {VariableType::NUMERIC_ANY, false}}},

        {"and",      VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"or",       VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"eq",       VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"not_eq",   VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"not",      VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}}},
        {"more_eq",  VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"less_eq",  VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"more",     VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
        {"less",     VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}, {VariableType::NUMERIC_ANY, false}}},
//        END OF TEMP FUNCTIONS
        {"concat",   VariableType::VOID,        {{VariableType::STRING,      false}, {VariableType::STRING,      false}}},
        {"to_str",   VariableType::STRING,      {{VariableType::NUMERIC_ANY, false}}},
        {"print",    VariableType::VOID,        {{VariableType::STRING,      false}}},
        {"to_int",   VariableType::INT,         {{VariableType::NUMERIC_ANY, false}}},
        {"to_uint",  VariableType::UINT,        {{VariableType::NUMERIC_ANY, false}}},
        {"to_float", VariableType::FLOAT,       {{VariableType::NUMERIC_ANY, false}}},
        {"return",   VariableType::VOID,        {{VariableType::NUMERIC_ANY, false}}},
        {"input",    VariableType::STRING,      {{VariableType::STRING,      false}}},
        {"print_num",VariableType::VOID,        {{VariableType::NUMERIC_ANY, false}}},
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
