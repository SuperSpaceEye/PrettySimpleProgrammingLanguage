//
// Created by spaceeye on 28.10.22.
//

#include "Tokens.h"

int try_find_reserved_token(std::string & word) {
    for (int i = 0; i < reserved_tokens.size(); i++) {
        auto & rword = reserved_tokens[i];
        if (rword == word) {
            return i;
        }
    }
    return -1;
}

bool is_empty_char(const std::string & chr) {
    if (chr == " " || chr == "\n") {
        return true;
    }
    return false;
}

bool is_logic_char(const std::string & chr) {
    for (auto & lchr: logic_chars) {
        if (lchr == chr) {
            return true;
        }
    }
    return false;
}

bool is_two_char_logic_word(const std::string & chr) {
    for (auto & lchr: reserved_two_char_logic_tokens) {
        if (lchr.first == chr) {
            return true;
        }
    }
    return false;
}

bool is_one_char_logic_word(const std::string & chr) {
    for (auto & lchr: reserved_one_char_logic_tokens) {
        if (lchr.first == chr) {
            return true;
        }
    }
    return false;
}

bool is_numeric(const std::string & chr) {
    for (auto & nchr: numeric_chars) {
        if (nchr == chr) {
            return true;
        }
    }
    return false;
}

is_number_result is_number(const std::string & word) {
    is_number_result res;

    if (word.length() < 1) {
        return res;
    }

    int i = 0;

    if (word[0] == *"-") {
        i = 1;
    }

    for (;i < word.length(); i++) {
        //for floats
        if (word[i] == *"." && !res.had_dot) {
            res.had_dot = true;
            continue;
            //if already has float, then not a number
        } else if (word[i] == *"." && res.had_dot) {
            return res;
        }

        if (!is_numeric(std::string{word[i]})) {
            return res;
        }
    }
    res.is_number = true;
    return res;
}