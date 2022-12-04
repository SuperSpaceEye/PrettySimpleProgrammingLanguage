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

    //TODO

    __COUNT,
};

//name, return type, arguments (type, ref)
const std::vector<std::tuple<std::string, VariableType, std::vector<std::pair<VariableType, bool>>>> builtin_functions_id_names{
//        TEMP FUNCTIONS BEFORE I IMPLEMENT PROPER ALGEBRAIC FUNCTIONS
        {"asgn",     VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, false}}},
        {"add",      VariableType::B_ANY, {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"sub",      VariableType::B_ANY, {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"mul",      VariableType::B_ANY, {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"div",      VariableType::B_ANY, {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},

        {"asgn_add", VariableType::VOID,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, false}}},
        {"asgn_sub",  VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, false}}},
        {"asgn_mul",  VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, false}}},
        {"asgn_div",  VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, false}}},

        {"and",      VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"or",       VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"eq",       VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"not_eq",   VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"not",      VariableType::UINT,   {{VariableType::B_ANY,  false}}},
        {"more_eq",  VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"less_eq",  VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"more",     VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
        {"less",     VariableType::UINT,   {{VariableType::B_ANY,  false}, {VariableType::B_ANY, false}}},
//        END OF TEMP FUNCTIONS
        {"concat",   VariableType::VOID,  {{VariableType::STRING, false}, {VariableType::STRING, false}}},
        {"to_str",   VariableType::STRING,{{VariableType::B_ANY,  false}}},
        {"print",    VariableType::VOID,  {{VariableType::STRING, false}}},
        {"to_int",   VariableType::INT,   {{VariableType::B_ANY,  false}}},
        {"to_uint",  VariableType::UINT,  {{VariableType::B_ANY,  false}}},
        {"to_float", VariableType::FLOAT, {{VariableType::B_ANY,  false}}},
        {"return",   VariableType::VOID,  {{VariableType::B_ANY,  false}}},
        {"input",    VariableType::STRING,{{VariableType::STRING, false}}},
        {"print_num",VariableType::VOID,  {{VariableType::B_ANY,  false}}},
};

const int builtin_IDS_count = (int)BuiltinIDS::__COUNT;

#endif //ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
