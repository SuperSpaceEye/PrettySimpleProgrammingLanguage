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
    std::vector<std::pair<Token, std::pair<int, int>>> tokens;
    WordRegister wreg;
};

class Transpiler {
private:
    static std::vector<std::pair<Token, std::pair<int, int>>>
    get_token(std::string &word, WordRegister &wreg, int row, int chrp);
    static void add_token(std::vector<std::pair<Token, std::pair<int, int>>> &main_tokens, std::vector<std::pair<Token, std::pair<int, int>>> to_emplace_tokens);

    static void transpile(TranspilerResult &to_return, std::string &word, const std::basic_string<char> &data);
public:
    static TranspilerResult transpile_data(const std::vector<std::string>& str_data);
    static void display_tokens(TranspilerResult & result);
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_TRANSPILER_H
