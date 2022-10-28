//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_TRANSPILER_H
#define ANOTHERPROGRAMMINGLANGUAGE_TRANSPILER_H

#include <vector>
#include <string>

#include "Tokens.h"

class Transpiler {
private:
    static Token get_token(std::string &word);
public:
    static std::vector<Token> transpile_data(const std::vector<std::string>& str_data);
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_TRANSPILER_H
