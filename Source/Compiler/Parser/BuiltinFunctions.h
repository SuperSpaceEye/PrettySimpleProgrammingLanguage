//
// Created by spaceeye on 01.11.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
#define ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H

#include <vector>
#include <string>

enum class BuiltinIDS{
    ASGN,
    ADD,
    SUB,
    MUL,
    DIV,

    ASGN_ADD,
    SUB_ADD,
    MUL_ADD,
    DIV_ADD,

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

    //TODO

    __COUNT,
};

//name, return type, arguments (type, ref)
const std::vector<std::tuple<std::string, VariableType, std::vector<std::pair<VariableType, bool>>>> builtin_functions_id_names{
//        TEMP FUNCTIONS BEFORE I IMPLEMENT PROPER ALGEBRAIC FUNCTIONS
        {"asgn",     VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"add",      VariableType::B_ANY, {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"sub",      VariableType::B_ANY, {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"mul",      VariableType::B_ANY, {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"div",      VariableType::B_ANY, {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},

        {"asgn_add", VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"sub_add",  VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"mul_add",  VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"div_add",  VariableType::VOID,  {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},

        {"and",      VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"or",       VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"eq",       VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"not_eq",   VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"not",      VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"more_eq",  VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"less_eq",  VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"more",     VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
        {"less",     VariableType::INT,   {{VariableType::B_ANY,  true}, {VariableType::B_ANY, true}}},
//        END OF TEMP FUNCTIONS
        {"concat",   VariableType::VOID,  {{VariableType::STRING, true}, {VariableType::STRING, true}}},
        {"to_str",   VariableType::STRING,{{VariableType::B_ANY,  true}}},
        {"print",    VariableType::VOID,  {{VariableType::STRING, true}}},
        {"to_int",   VariableType::INT,   {{VariableType::B_ANY,  true}}},
        {"to_uint",  VariableType::UINT,  {{VariableType::B_ANY,  true}}},
        {"to_float", VariableType::FLOAT, {{VariableType::B_ANY,  true}}},
        {"return",   VariableType::VOID,  {{VariableType::B_ANY,  true}}},
        {"input",    VariableType::STRING,{{VariableType::STRING, true}}},
        {"print_num",VariableType::VOID,  {{VariableType::INT,    true}}},
};

const int builtin_IDS_count = (int)BuiltinIDS::__COUNT;

#endif //ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
