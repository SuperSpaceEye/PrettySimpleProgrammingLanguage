//
// Created by spaceeye on 25.10.22.
//

#include "Compiler.h"

std::vector<ByteCode> Compiler::compile(const std::vector<std::string> &str_data, bool debug) {
    auto res = Transpiler::transpile_data(str_data);
    if (debug) {Transpiler::display_tokens(res);}
    auto ast = Parser::create_ast(res, debug);

    return std::vector<ByteCode>();
}
