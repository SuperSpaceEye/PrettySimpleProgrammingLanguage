//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_TRANSPILER_H
#define ANOTHERPROGRAMMINGLANGUAGE_TRANSPILER_H

#include <vector>
#include <string>
#include <iostream>

#include "Tokens.h"

class WordRegister {
public:
    std::vector<std::string> names;

    int register_word(std::string & word);
    std::string get_word(Token token);

    WordRegister()=default;
};

struct TranspilerResult {
    std::vector<Token> tokens;
    WordRegister wreg;
};

class Transpiler {
private:
    static std::vector<Token> get_token(std::string &word, WordRegister &wreg);
    static void add_token(std::vector<Token> & main_tokens, std::vector<Token> to_emplace_tokens);

    static void transpile(TranspilerResult &to_return, std::string &word, const std::basic_string<char> &data);
public:
    static TranspilerResult transpile_data(const std::vector<std::string>& str_data);
    static void display_tokens(TranspilerResult & result);
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_TRANSPILER_H
