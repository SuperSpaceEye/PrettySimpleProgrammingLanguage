//
// Created by spaceeye on 10.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_COMMONFNS_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_COMMONFNS_H

#include <string>
#include <exception>
#include <stdexcept>

#include "VariableType.h"

//row col string
inline std::string rcs(std::pair<int, int> pos) {
    return "Row " + std::to_string(pos.first) +
           " Col " + std::to_string(pos.second) + ".";
}

inline std::string type_to_str(VariableType type) {
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

inline bool var_type_is_numeric(VariableType type) {
    switch (type) {
        case VariableType::INT:
        case VariableType::UINT:
        case VariableType::FLOAT:
            return true;
        default: return false;
    }
}

#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_COMMONFNS_H
