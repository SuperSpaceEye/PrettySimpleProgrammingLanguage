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

    INC,
    DEC,

    AND,
    OR,
    EQUALS,
    NOT_EQUALS,
    NOT,
    MORE_EQUALS,
    LESS_EQUALS,
    MORE,
    LESS,

    PRINT,
    TO_INT,
    TO_UINT,
    TO_FLOAT,


    //TODO

    __COUNT,
};

const std::vector<std::string> builtin_functions_id_names {
        //TEMP FUNCTIONS BEFORE I IMPLEMENT PROPER ALGEBRAIC FUNCTIONS
        "asgn",
        "add",
        "sub",
        "mul",
        "div",

        "asgn_add",
        "sub_add",
        "mul_add",
        "div_add",

        "inc",
        "dec",

        "and",
        "or",
        "eq",
        "not_eq",
        "not",
        "more_eq",
        "less_eq",
        "more",
        "less",
        //END OF TEMP FUNCTIONS
        "print",
        "to_int",
        "to_uint",
        "to_float"

};

const int builtin_IDS_count = (int)BuiltinIDS::__COUNT;

#endif //ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
