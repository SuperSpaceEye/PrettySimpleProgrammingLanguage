//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_TOKENS_H
#define ANOTHERPROGRAMMINGLANGUAGE_TOKENS_H

#include <string>
#include <vector>
#include <array>
#include <cstring>

const std::vector<std::string> reserved_tokens {
    "fn",
    "inline",
    "int",
    "uint",
    "float",
    "if",
    "while",
    "for",

    "=",

    "+",
    "-",
    "*",
    "/",

    "&",
    "||",
    "==",
    "!=",
    "!",
    ">=",
    "<=",
    ">",
    "<",

    "(",
    ")",
    "[",
    "]",
    "{",
    "}",

    ";",
};

const std::vector<std::string> logic_chars {
        "=",
        "+",
        "-",
        "*",
        "/",
        "&",
        "!",
        "|",
        ">",
        "<",
        "(",
        ")",
        "[",
        "]",
        "{",
        "}",
        ";"
};

const std::vector<std::string> numeric_chars {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

enum class Token {
    FUNCTION,
    INLINE,
    INT,
    UINT,
    FLOAT,
    IF,
    WHILE,
    FOR,

    ASSIGNMENT,
    ADDITION,
    SUBTRACTION,
    DIVISION,

    AND,
    OR,
    EQUALS,
    NOT_EQUALS,
    NOT,
    MORE_OR_EQUAL,
    LESS_OR_EQUAL,
    LESS,
    MORE,

    LEFT_CIRCLE_BRACKET,
    RIGHT_CIRCLE_BRACKET,
    LEFT_BOX_BRACKET,
    RIGHT_BOX_BRACKET,
    BEGIN_LOGIC_BLOCK,
    END_LOGIC_BLOCK,
    END_COMMAND,


    UNK_WORD,
    UNK_FUNC,
    BEGIN_ATTRIBUTES,
    END_ATTRIBUTES,
    BEGIN_CONDITIONS,
    END_CONDITIONS,
};

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

bool is_numeric(const std::string & chr) {
    for (auto & nchr: numeric_chars) {
        if (nchr == chr) {
            return true;
        }
    }
    return false;
}

bool is_number(const std::string & word) {
    bool had_dot = false;

    std::string new_word;

    int i = 0;

    if (strcmp(&word[0], "-") == 0) {
        i = 1;
    }

    for (;i < word.length(); i++) {
        //for floats
        if (strcmp(&word[i], ".") == 0 && !had_dot) {
            had_dot = true;
            continue;
        //if already has float, then not a number
        } else if (strcmp(&word[i], ".") == 0 && had_dot) {
            return false;
        }

        if (!is_numeric(std::string{word[i]})) {
            return false;
        }
    }

    return true;
}

#endif //ANOTHERPROGRAMMINGLANGUAGE_TOKENS_H
