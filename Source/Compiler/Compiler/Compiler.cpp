//
// Created by spaceeye on 25.10.22.
//

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string>& str_data) {
    auto res = Transpiler::transpile_data(str_data);
    Transpiler::display_tokens(res);
    return std::vector<ByteCode>();
}
