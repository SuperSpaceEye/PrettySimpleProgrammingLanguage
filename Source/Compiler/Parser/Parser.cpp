//
// Created by spaceeye on 28.10.22.
//

#include "Parser.h"

ASTCreationResult Parser::create_ast(TranspilerResult &t_result) {
    ASTCreationResult to_return{};
    IdRegister reg{t_result.wreg};
    std::vector<Token> & tokens = t_result.tokens;

    int logic_indentation = 0;
    bool function_declaration = false;

    std::shared_ptr<BaseAction> root;

    recursive_create_ast(tokens, logic_indentation, function_declaration, to_return, reg, root, 0, tokens.size());

    return to_return;
}

void Parser::recursive_create_ast(const std::vector<Token> &tokens, int logic_indentation, bool &function_declaration,
                                  ASTCreationResult &to_return, IdRegister &reg, std::shared_ptr<BaseAction> &root,
                                  int begin_num, int end_num) {
    for (int i = begin_num; i < end_num; i++) {
        auto token = tokens[i];

        switch (token) {
            case Token::FUNCTION: {
                if (logic_indentation != 0) { throw std::logic_error{"Functions should be declared at top level."}; }
                function_declaration = true;

                bool is_inline = false;
                if (tokens[i++] == Token::INLINE) {is_inline = true;} else {i--;}

                VariableType return_type;
                switch (tokens[i++]) {
                    case Token::VOID:  return_type = VariableType::VOID; break;
                    case Token::INT:   return_type = VariableType::INT; break;
                    case Token::UINT:  return_type = VariableType::UINT; break;
                    case Token::FLOAT: return_type = VariableType::FLOAT; break;
                    case Token::ARRAY: return_type = VariableType::ARRAY; break;
                    default:
                        throw std::logic_error{"Invalid function declaration."};
                }

                if (tokens[i++] != Token::UNK_WORD) {throw std::logic_error{"Invalid function declaration."};}
                int id = (int)tokens[i++];

                reg.register_function(id);

                std::vector<std::pair<VariableType, int>> arguments;

                if (tokens[i++] != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function declaration."};}

                auto token = tokens[i++];
                while (token != Token::RIGHT_CIRCLE_BRACKET) {
                    check_token_is_valid_argument(token);

                    token = tokens[i++];
                }

                root = std::make_shared<FunctionDeclaration>(FunctionDeclaration{BaseAction{ActionType::FunctionCall},
                                                                                 id, is_inline, return_type, });

            }
                break;
            case Token::RETURN:
                break;
            case Token::VOID:
                break;
            case Token::ARRAY:
                break;
            case Token::INT:
                break;
            case Token::UINT:
                break;
            case Token::FLOAT:
                break;
            case Token::IF:
                break;
            case Token::WHILE:
                break;
            case Token::FOR:
                break;
            case Token::ASGN:
                break;
            case Token::ADD:
                break;
            case Token::SUB:
                break;
            case Token::MULT:
                break;
            case Token::DIV:
                break;
            case Token::ADD_ASGN:
                break;
            case Token::SUB_ASGN:
                break;
            case Token::MULT_ASGN:
                break;
            case Token::DIV_ASGN:
                break;
            case Token::INC:
                break;
            case Token::DEC:
                break;
            case Token::AND:
                break;
            case Token::OR:
                break;
            case Token::EQUALS:
                break;
            case Token::NOT_EQUALS:
                break;
            case Token::NOT:
                break;
            case Token::MORE_OR_EQUAL:
                break;
            case Token::LESS_OR_EQUAL:
                break;
            case Token::MORE:
                break;
            case Token::LESS:
                break;
            case Token::LEFT_CIRCLE_BRACKET:
                break;
            case Token::RIGHT_CIRCLE_BRACKET:
                break;
            case Token::LEFT_BOX_BRACKET:
                break;
            case Token::RIGHT_BOX_BRACKET:
                break;
            case Token::BEGIN_LOGIC_BLOCK:
                logic_indentation++;
                break;
            case Token::END_LOGIC_BLOCK:
                logic_indentation--;
                break;
            case Token::END_COMMAND:
                break;
            case Token::COMMA:
                break;
            case Token::UNK_WORD:
                break;
            case Token::NUMBER:
                break;

            case Token::COMMENT:
            case Token::INLINE:
                throw std::logic_error{"Invalid token"};
            default:
                throw std::logic_error{"Shouldn't happen."};
        }
    }
}

void Parser::check_token_is_valid_argument(Token token) {
    switch (token) {
        case Token::INT:
        case Token::UINT:
        case Token::FLOAT:
        case Token::ARRAY:
            break;
        default:
            throw std::logic_error{"Invalid function argument declaration."};
    }
}

void IdRegister::register_function(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {throw std::logic_error{"Variable with id already exists."};}
    }

    for (auto & ids: function_ids) {
        if (ids.first == id) {throw std::logic_error{"Function with id already exists."};}
    }

    function_ids.emplace_back(id, wreg.get_word((Token)id));
}

void IdRegister::register_variable(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {throw std::logic_error{"Variable with id already exists."};}
    }

    for (auto & ids: function_ids) {
        if (ids.first == id) {throw std::logic_error{"Function with id already exists."};}
    }

    variable_ids.emplace_back(id, wreg.get_word((Token)id));
}

void IdRegister::check_function(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {throw std::logic_error{"Called function is actually a variable."};}
    }

    for (auto & ids: function_ids) {
        if (ids.first == id) { return;}
    }

    throw std::logic_error{"No function was declared."};
}

void IdRegister::check_variable(int id) {
    for (auto & ids: function_ids) {
        if (ids.first == id) {throw std::logic_error{"Function with id wasn't declared."};}
    }

    for (auto & ids: variable_ids) {
        if (ids.first == id) { return;}
    }
    throw std::logic_error{"Variable with id wasn't declared."};
}
