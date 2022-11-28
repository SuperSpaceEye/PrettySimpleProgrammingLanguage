//
// Created by spaceeye on 29.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_VARIABLETYPE_H
#define ANOTHERPROGRAMMINGLANGUAGE_VARIABLETYPE_H

enum class VariableType {
    VOID,
    //ONLY FOR 4 BYTE TYPES. first 4 bytes are value and second 4 bytes are
    B_ANY,
    INT,
    UINT,
    FLOAT,
    ARRAY,
    STRING
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_VARIABLETYPE_H
