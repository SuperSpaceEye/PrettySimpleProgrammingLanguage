//
// Created by spaceeye on 01.11.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
#define ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H

#include <vector>

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

    //TODO

    __COUNT,
};

const int builtin_IDS_count = (int)BuiltinIDS::__COUNT;

#endif //ANOTHERPROGRAMMINGLANGUAGE_BUILTINFUNCTIONS_H
