//
// Created by spaceeye on 08.12.22.
//

#include "../Parser.h"

std::string Parser::type_to_str(VariableType type) {
    switch (type) {
        case VariableType::VOID:   return "void";
        case VariableType::NUMERIC_ANY:  return "numeric_any";
        case VariableType::INT:    return "int";
        case VariableType::UINT:   return "uint";
        case VariableType::FLOAT:  return "float";
        case VariableType::ARRAY:  return "array";
        case VariableType::STRING: return "string";
        default: throw std::logic_error("Shouldn't happen. type_to_str.");
    }
}

bool Parser::var_type_is_numeric(VariableType type) {
    switch (type) {
        case VariableType::INT:
        case VariableType::UINT:
        case VariableType::FLOAT:
            return true;
        default: return false;
    }
}