//
// Created by spaceeye on 29.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_OPERATIONTYPE_H
#define ANOTHERPROGRAMMINGLANGUAGE_OPERATIONTYPE_H

enum class ActionType {
    NoType,
    Variable,
    FunctionCall,
    FunctionDeclaration,
    ForLoop,
    WhileLoop,
    IfStatement
};

enum class FunctionType {
    UserFunction,
    BuiltinFunction
};



#endif //ANOTHERPROGRAMMINGLANGUAGE_OPERATIONTYPE_H
