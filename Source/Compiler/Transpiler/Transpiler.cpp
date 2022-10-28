//
// Created by spaceeye on 25.10.22.
//

#include "Transpiler.h"

std::vector<Token> Transpiler::transpile_data(const std::vector<std::string>& str_data) {
    std::vector<Token> to_return;
    std::string word;

    //TODO
    auto & data = str_data[0];

    for (int i = 0; i < data.length(); i++) {
        auto chr = std::string{data[i]};

        if (is_empty_char(chr)) {
            if (!word.empty()) {
                get_token(word);
            }
            continue;
        }

        if (chr == ";") {
            get_token(word);
            continue;
        }

        if (is_logic_char(chr)) {
            if (!word.empty()) {
                get_token(word);
            }

            word += chr;

            if (i + 1 < data.length() && is_logic_char(std::string{data[i+1]})) {
                auto temp_word = word + data[i+1];
                if (get_token(temp_word) != Token::UNK_WORD) {}

            }
        }

        word += chr;
    }


    return to_return;
}

Token Transpiler::get_token(std::string &word) {

    word = "";
}