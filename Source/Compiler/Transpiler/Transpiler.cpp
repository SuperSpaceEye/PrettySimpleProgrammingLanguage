//
// Created by spaceeye on 25.10.22.
//

#include "Transpiler.h"

TranspilerResult Transpiler::transpile_data(const std::vector<std::string>& str_data) {
    TranspilerResult to_return;
    std::string word;

    //TODO
    auto & data = str_data[0];

    transpile(to_return, word, data);

    return to_return;
}

void
Transpiler::transpile(TranspilerResult &to_return, std::string &word, const std::basic_string<char> &data) {
    bool comment = false;
    for (int i = 0; i < data.length(); i++) {
        auto chr = std::string{data[i]};

        if (chr == "\n" && comment) {
            comment = false;
            continue;
        }

        if (comment) {
            continue;
        }

        if (is_empty_char(chr)) {
            if (!word.empty()) {
                add_token(to_return.tokens, get_token(word, to_return.wreg));
            }
            continue;
        }

        if (chr == ";") {
            if (!word.empty()) {add_token(to_return.tokens, get_token(word, to_return.wreg));}
            to_return.tokens.emplace_back(Token::END_COMMAND);
            continue;
        }

        if (chr == "#") {
            comment = true;
            continue;
        }

        if (is_logic_char(chr)) {
            if (!word.empty()) {
                add_token(to_return.tokens, get_token(word, to_return.wreg));
            }

            word += chr;

            if (i + 1 < data.length()) {
                // if it is a {-number}
                if (chr == "-" && is_numeric(std::string{data[i + 1]})) {
                    continue;
                }

                if (is_logic_char(std::string{data[i + 1]})) {
                    auto temp_word = word + data[i + 1];
                    if (is_two_char_logic_word(temp_word)) {
                        add_token(to_return.tokens, get_token(temp_word, to_return.wreg));
                        word = "";
                        i++;
                        continue;
                    }
                }
            }
        add_token(to_return.tokens, get_token(word, to_return.wreg));
        continue;
        }

        word += chr;
    }
}

std::vector<Token> Transpiler::get_token(std::string &word, WordRegister &wreg) {
    std::vector<Token> to_return;

    int reserved_result;
    is_number_result number_result;

    reserved_result = try_find_reserved_token(word);
    if (reserved_result != -1) {
        to_return.emplace_back((Token)(reserved_result));
        goto end;
    }

    number_result = is_number(word);
    if (number_result.is_number) {
        if (number_result.had_dot) {
            to_return.emplace_back(Token::NUMBER);
            to_return.emplace_back(Token::FLOAT);
            to_return.emplace_back((Token)(std::stof(word)));
        } else {
            to_return.emplace_back(Token::NUMBER);
            to_return.emplace_back(Token::INT);
            to_return.emplace_back((Token)(std::stoi(word)));
        }
        goto end;
    }

    to_return.emplace_back(Token::UNK_WORD);
    to_return.emplace_back((Token)wreg.register_word(word));

    end:
    word = "";
    return to_return;
}

void Transpiler::add_token(std::vector<Token> &main_tokens, std::vector<Token> to_emplace_tokens) {
    main_tokens.insert(main_tokens.end(), to_emplace_tokens.begin(), to_emplace_tokens.end());
}

void Transpiler::display_tokens(TranspilerResult &result) {
    for (int i = 0; i < result.tokens.size(); i++) {
        auto token = result.tokens[i];
        if (token == Token::NUMBER) {
            switch (result.tokens[i+1]) {
                case Token::INT:
                    std::cout << (int32_t) result.tokens[i + 2];
                    break;
                case Token::UINT:
                    std::cout << (uint32_t) result.tokens[i + 2];
                    break;
                case Token::FLOAT:
                    std::cout << (float) result.tokens[i + 2];
                    break;
            }
            i += 2;
            continue;
        }

        if (token == Token::UNK_WORD) {
            std::cout << result.wreg.get_word(result.tokens[i+1]) << " ";
            i++;
            continue;
        }

        if ((int)token <= num_words) {
            std::cout << reserved_tokens[(int) token] << " ";
        }
    }

    std::cout << "\nWords: ";
    for (auto & word: result.wreg.names) {
        std::cout << "\"" << word << "\" ";
    }
    std::cout << "\n";
}

int WordRegister::register_word(std::string &word) {
    if (word.empty()) {throw "bad";}
    for (int i = 0; i < names.size(); i++) {
        if (names[i] == word) {
            return i;
        }
    }

    names.emplace_back(word);
    return names.size() - 1;
}

std::string WordRegister::get_word(Token token) {
    return names[(int)token];
}