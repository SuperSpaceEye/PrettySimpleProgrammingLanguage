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
    int row = 1;
    int chrp = 0;

    int tchrp = 0;
    bool started_token = false;

    bool comment = false;
    for (int i = 0; i < data.length(); i++) {
        auto chr = std::string{data[i]};

        chrp++;
        if (!started_token) {
            tchrp = chrp;
            started_token = true;
        }

        if (chr == "\n") {
            row++;
            chrp = 0;
        }

        if (chr == "\n" && comment) {
            comment = false;
            started_token = false;
            continue;
        }

        if (comment) {
            started_token = false;
            continue;
        }

        if (is_empty_char(chr)) {
            if (!word.empty()) {
                add_token(to_return.tokens, get_token(word, to_return.wreg, row, tchrp));
            }
            started_token = false;
            continue;
        }

        if (chr == ";") {
            if (!word.empty()) { add_token(to_return.tokens, get_token(word, to_return.wreg, row, tchrp)); started_token = false;}
            to_return.tokens.push_back({Token::END_COMMAND, {row, chrp}});
            continue;
        }

        if (chr == "#") {
            comment = true;
            continue;
        }

        if (chr == "\"") {
            to_return.tokens.push_back({Token::STR_BRACKET, {row, tchrp}});
            while (i < data.length() && data[++i] != *"\"") {
                if (data[i] == *"\\") {
                    if (i + 1 < data.length()) {
                        if (data[i+1] == *"n") {
                            to_return.tokens.push_back({(Token)(*"\n"+num_tokens), {row, tchrp}}); i++;
                        } else if (data[i+1] == *"\"") {
                            to_return.tokens.push_back({(Token)(*"\""+num_tokens), {row, tchrp}}); i++;
                        } else if (data[i+1] == *"\\") {
                            to_return.tokens.push_back({(Token)(*"\\"+num_tokens), {row, tchrp}}); i++;
                        } else {
                            to_return.tokens.push_back({(Token)(*"\\"+num_tokens), {row, tchrp}});
                        }
                    }
                } else {
                    to_return.tokens.push_back({(Token)(data[i]+num_tokens), {row, tchrp}});
                }
            }
            to_return.tokens.push_back({Token::STR_BRACKET, {row, tchrp}});
            started_token = false;
            continue;
        }

        if (is_logic_char(chr)) {
            if (!word.empty()) {
                add_token(to_return.tokens, get_token(word, to_return.wreg, row, tchrp));
                started_token = false;
                tchrp = chrp;
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
                        add_token(to_return.tokens, get_token(temp_word, to_return.wreg, row, chrp));
                        word = "";
                        i++;
                        continue;
                    }
                }
            }
            add_token(to_return.tokens, get_token(word, to_return.wreg, row, chrp));
            started_token = false;
        continue;
        }

        word += chr;
    }
}

std::vector<std::pair<Token, std::pair<int, int>>>
Transpiler::get_token(std::string &word, WordRegister &wreg, int row, int chrp) {
    std::vector<std::pair<Token, std::pair<int, int>>> to_return;

    int reserved_result;
    is_number_result number_result;

    reserved_result = try_find_reserved_token(word);
    if (reserved_result != -1) {
        to_return.push_back({(Token)(reserved_result), {row, chrp}});
        goto end;
    }

    number_result = is_number(word);
    if (number_result.is_number) {
        to_return.push_back({Token::NUMBER, {row, chrp}});
        if (number_result.had_dot) {
            to_return.push_back({Token::FLOAT, {row, chrp}});
            auto num = std::stof(word);
            to_return.push_back({*(Token*)&num, {row, chrp}});
        } else {
            to_return.push_back({Token::INT, {row, chrp}});
            auto num = std::stoi(word);
            to_return.push_back({*(Token*)&num, {row, chrp}});
        }
        goto end;
    }

    to_return.push_back({Token::UNK_WORD, {row, chrp}});
    to_return.push_back({(Token)wreg.register_word(word), {row, chrp}});

    end:
    word = "";
    return to_return;
}

void Transpiler::add_token(std::vector<std::pair<Token, std::pair<int, int>>> &main_tokens, std::vector<std::pair<Token, std::pair<int, int>>> to_emplace_tokens) {
    main_tokens.insert(main_tokens.end(), to_emplace_tokens.begin(), to_emplace_tokens.end());
}

void Transpiler::display_tokens(TranspilerResult &result) {
    for (int i = 0; i < result.tokens.size(); i++) {
        auto token = result.tokens[i].first;
        if (token == Token::NUMBER) {
            switch (result.tokens[i+1].first) {
                case Token::INT:
                    std::cout << (int32_t) result.tokens[i + 2].first;
                    break;
                case Token::UINT:
                    std::cout << (uint32_t) result.tokens[i + 2].first;
                    break;
                case Token::FLOAT:
                    std::cout << (float) result.tokens[i + 2].first;
                    break;
            }
            i += 2;
            continue;
        }

        if (token == Token::COMMA) {
            std::cout << ", ";
        }

        if (token == Token::UNK_WORD) {
            std::cout << result.wreg.get_word(result.tokens[i+1].first) << " ";
            i++;
            continue;
        }

        if (token == Token::STR_BRACKET) {
            std::cout << "\"";
            while (result.tokens[++i].first != Token::STR_BRACKET) {
                std::cout << (char)((int)result.tokens[i].first-num_tokens);
            }
            std::cout << "\"";
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