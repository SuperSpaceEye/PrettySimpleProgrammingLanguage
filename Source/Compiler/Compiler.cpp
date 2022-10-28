//
// Created by spaceeye on 25.10.22.
//

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string>& str_data) {
    Transpiler::transpile_data(str_data);
    return std::vector<ByteCode>();
}
