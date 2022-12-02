//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_TOKENS_H
#define ANOTHERPROGRAMMINGLANGUAGE_TOKENS_H

#include <string>
#include <vector>
#include <array>
#include <cstring>

const int num_tokens = 43;
const int num_words = 41;

enum class Token {
    FUNCTION,
    VAR,
    INLINE,

    VOID,
    ARRAY,
    INT,
    UINT,
    FLOAT,
    STRING,
    IF,
    ELSE,
    ENDIF,
    WHILE,
    FOR,

    ASGN,

    ADD,
    SUB,
    MULT,
    DIV,

    ADD_ASGN,
    SUB_ASGN,
    MULT_ASGN,
    DIV_ASGN,
    INC,
    DEC,

    AND,
    OR,
    EQUALS,
    NOT_EQUALS,
    NOT,
    MORE_OR_EQUAL,
    LESS_OR_EQUAL,
    MORE,
    LESS,

    LEFT_CIRCLE_BRACKET,
    RIGHT_CIRCLE_BRACKET,
    LEFT_BOX_BRACKET,
    RIGHT_BOX_BRACKET,
    BEGIN_LOGIC_BLOCK,
    END_LOGIC_BLOCK,

    STR_BRACKET,

    REF,
    END_COMMAND,
    COMMENT,
    COMMA,


    UNK_WORD,
    NUMBER
};

const std::vector<std::string> reserved_tokens {
    "fn",
    "var",
    "inline",

    "void",
    "array",
    "int",
    "uint",
    "float",
    "string",
    "if",
    "else",
    "endif",
    "while",
    "for",

    "=",

    "+",
    "-",
    "*",
    "/",

    "+=",
    "-=",
    "*=",
    "/=",
    "++",
    "--",

    "&&",
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

    "\"",

    "&",
    ";",
    "#",
    ","
};

const std::vector<std::pair<std::string, Token>> reserved_two_char_logic_tokens {
        {"&&", Token::AND},
        {"||", Token::OR},
        {"==", Token::EQUALS},
        {"!=", Token::NOT_EQUALS},
        {">=", Token::MORE_OR_EQUAL},
        {"<=", Token::LESS_OR_EQUAL},

        {"+=", Token::ADD_ASGN},
        {"-=", Token::SUB_ASGN},
        {"*=", Token::MULT_ASGN},
        {"/=", Token::DIV_ASGN},
        {"++", Token::INC},
        {"--", Token::DEC},
};

const std::vector<std::pair<std::string, Token>> reserved_one_char_logic_tokens {
        {"=", Token::ASGN},
        {"+", Token::ADD},
        {"-", Token::SUB},
        {"*", Token::MULT},
        {"/", Token::DIV},
        {"&", Token::REF},
        {"!", Token::NOT},
        {">", Token::MORE},
        {"<", Token::LESS},
        {"(", Token::LEFT_CIRCLE_BRACKET},
        {")", Token::RIGHT_CIRCLE_BRACKET},
        {"[", Token::LEFT_BOX_BRACKET},
        {"]", Token::RIGHT_BOX_BRACKET},
        {"{", Token::BEGIN_LOGIC_BLOCK},
        {"}", Token::END_LOGIC_BLOCK},
        {";", Token::END_COMMAND},
        {"#", Token::COMMENT},
        {",", Token::COMMA},
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
        ";",
        "#",
        ","
};

const std::vector<std::string> numeric_chars {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

int try_find_reserved_token(std::string & word);

bool is_empty_char(const std::string & chr);

bool is_logic_char(const std::string & chr);

bool is_two_char_logic_word(const std::string & chr);

bool is_one_char_logic_word(const std::string & chr);

bool is_numeric(const std::string & chr);

struct is_number_result {
    bool is_number = false;
    bool had_dot = false;
};

is_number_result is_number(const std::string & word);
#endif //ANOTHERPROGRAMMINGLANGUAGE_TOKENS_H
