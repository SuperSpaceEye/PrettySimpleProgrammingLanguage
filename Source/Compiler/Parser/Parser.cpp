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
                if (tokens[++i] == Token::INLINE) {is_inline = true;} else {--i;}

                VariableType return_type;

                return_type = convert_token_type(tokens[++i]);

                int id = get_id(tokens, i);

                reg.register_function(id);

                //var type, is_reference, var id
                std::vector<std::tuple<VariableType, bool, int>> arguments;

                if (tokens[++i] != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function declaration."};}

                while (tokens[++i] != Token::RIGHT_CIRCLE_BRACKET) {
                    check_token_is_valid_argument(tokens[i], i);

                    auto var_type = convert_token_type(tokens[i]);
                    auto is_ref = false; if (tokens[++i] == Token::REF) {is_ref=true;} else {--i;}
                    auto var_id = get_id(tokens, i);

                    reg.register_variable(var_id);

                    arguments.emplace_back(var_type, is_ref, var_id);
                }

                root = std::make_shared<FunctionDeclaration>(FunctionDeclaration{BaseAction{ActionType::FunctionCall},
                                                                                 id, is_inline, return_type, });
            }
                break;
            case Token::VAR:
                //TODO
                if (logic_indentation == 0) {throw std::logic_error{"Can't declare global variable."};}
                if (tokens[++i] == Token::ARRAY) {} else {
                    auto var_type = convert_token_type(tokens[i]);

                    //TODO
                    if (tokens[++i] != Token::REF) {} else {--i;}

                    auto var_id = get_id(tokens, i);
                    reg.register_variable(var_id);

                    //TODO
                    auto new_root = std::make_shared<Variable>();
                }
                break
            case Token::RETURN:
                if (logic_indentation == 0) {throw std::logic_error{"Invalid token"};}
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

            case Token::ARRAY:
            case Token::INT:
            case Token::UINT:
            case Token::FLOAT:
            case Token::VOID:
            case Token::COMMENT:
            case Token::INLINE:
                throw std::logic_error{"Invalid token"};
            default:
                throw std::logic_error{"Shouldn't happen."};
        }
    }
}

int Parser::get_id(const std::vector<Token> &tokens, int &i) {
    if (tokens[++i] != Token::UNK_WORD) {throw std::logic_error{"Invalid function declaration."};}
    return (int)tokens[++i];
}

VariableType Parser::convert_token_type(Token token) {
    switch (token) {
        case Token::VOID:  return VariableType::VOID; break;
        case Token::INT:   return VariableType::INT; break;
        case Token::UINT:  return VariableType::UINT; break;
        case Token::FLOAT: return VariableType::FLOAT; break;
        case Token::ARRAY: return VariableType::ARRAY; break;
        default:
            throw std::logic_error{"Invalid token in variable type."};
    }
}

void Parser::check_token_is_valid_argument(Token token, int &i) {
    switch (token) {
        case Token::INT:
        case Token::UINT:
        case Token::FLOAT:
        case Token::ARRAY:
            break;
        case Token::COMMA:
            ++i;
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
    for (auto & ids: function_ids) {
        if (ids.first == id) {throw std::logic_error{"Function with id already exists."};}
    }

    for (auto & ids: variable_ids) {
        if (ids.first == id) {
//            throw std::logic_error{"Variable with id already exists."};
            return;
        }
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
