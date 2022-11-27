//
// Created by spaceeye on 28.10.22.
//

#include "../Parser.h"

ASTCreationResult Parser::create_ast(TranspilerResult &t_result, bool debug) {
    ASTCreationResult to_return{{}, IdRegister{t_result.wreg}};
    std::vector<Token> & tokens = t_result.tokens;

    int i = 0;
    while (i < tokens.size() - 1) {
        int logic_indentation = 0;
        bool function_declaration = false;

        auto root = std::make_shared<BaseAction>();
        auto beginning = std::shared_ptr{root};

        recursive_create_ast(tokens, logic_indentation, function_declaration, to_return, to_return.reg, root, 0, tokens.size(), i);

        i++;
        to_return.function_roots.emplace_back(beginning->next_action);
    }

    if (debug) {show_ast(to_return, to_return.reg);}
    return to_return;
}

void Parser::recursive_create_ast(const std::vector<Token> &tokens, int logic_indentation, bool &function_declaration,
                                  ASTCreationResult &to_return, IdRegister &reg, std::shared_ptr<BaseAction> &root,
                                  int begin_num, int end_num, int &i) {
    std::shared_ptr<BaseAction> temp_root;
    for (; i < end_num; i++) {
        auto token = tokens[i];

        switch (token) {
            case Token::FUNCTION: {
//                if (function_declaration) {throw std::logic_error("Functions cannot be declared inside another functions.");}
//                if (logic_indentation != 0) { throw std::logic_error{"Functions should be declared at top level."}; }
                function_declaration = true;

                bool is_inline = false;
                if (tokens[++i] == Token::INLINE) {is_inline = true;} else {--i;}

                VariableType return_type;

                return_type = convert_token_type(tokens[++i]);

                int id = get_id(tokens, ++i);

                if (reg.check_exists(id)) { throw std::logic_error("Id already exists");}

                //var type, is_reference, var id
                std::vector<std::tuple<VariableType, bool, int>> arguments;

                if (tokens[++i] != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function declaration."};}

                while (tokens[++i] != Token::RIGHT_CIRCLE_BRACKET) {
                    check_token_is_valid_argument(tokens[i], i);

                    auto var_type = convert_token_type(tokens[i]);
                    auto is_ref = false; if (tokens[++i] == Token::REF) {is_ref=true;} else {--i;}
                    auto var_id = get_id(tokens, ++i);

                    reg.register_variable(var_id);

                    arguments.emplace_back(var_type, is_ref, var_id);
                }

                auto temp_root = std::make_shared<FunctionDeclaration>(FunctionDeclaration{BaseAction{ActionType::FunctionDeclaration, root},
                                                                                 id, is_inline, return_type, arguments});
                reg.register_function(id, temp_root.get());
                root->next_action = temp_root;
                root = temp_root;
            }
                break;
            case Token::VAR:
                //TODO
                if (logic_indentation == 0) {throw std::logic_error{"Can't declare global variable."};}
                if (tokens[++i] == Token::ARRAY) {

                } else {
                    auto var_type = convert_token_type(tokens[i]);

                    bool is_ref = false;
                    if (tokens[++i] != Token::REF) {is_ref = true;} else {--i;}

                    auto var_id = get_id(tokens, i);
                    reg.register_variable(var_id);

                    temp_root = std::make_shared<VariableDeclaration>(VariableDeclaration{BaseAction{ActionType::VariableDeclaration},
                                                                                          var_type, var_id, is_ref});
                    root->next_action = temp_root;
                    root = temp_root;
                }
                break;
            case Token::IF:
                break;
            case Token::WHILE:
                break;
            case Token::FOR:
                break;
            case Token::LEFT_CIRCLE_BRACKET:
                break;
            case Token::RIGHT_CIRCLE_BRACKET:
                i--;
                return;
                break;
            case Token::LEFT_BOX_BRACKET:
                break;
            case Token::RIGHT_BOX_BRACKET:
                return;
                break;
            case Token::BEGIN_LOGIC_BLOCK:
                logic_indentation++;
                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::StartLogicBlock});
                root->next_action = temp_root;
                root = temp_root;
                recursive_create_ast(tokens, logic_indentation, function_declaration, to_return, reg, root, begin_num, end_num, ++i);
                break;
            case Token::END_LOGIC_BLOCK:
                logic_indentation--;
                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::EndLogicBlock});
                root->next_action = temp_root;
                root = temp_root;
                return;
                break;
            case Token::END_COMMAND:
//                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::EndAction});
//                root->next_action = temp_root;
//                root = temp_root;
                break;
            case Token::COMMA:
                return;
                break;
            case Token::UNK_WORD: {
                //TODO make logic for x, y, z = array_output_func();

                auto id = (int)tokens[++i];

                if (reg.check_function(id)) {
                    if (reg.is_builtin_fn(id)) {
                        auto bfn = reg.get_builtin(id);

                        //var type, is_reference, var id
                        std::vector<std::shared_ptr<BaseAction>> arguments;

                        if (tokens[++i] != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function call."};}

                        while (tokens[++i] != Token::RIGHT_CIRCLE_BRACKET) {
                            auto arg_root = std::make_shared<BaseAction>();
                            auto in_root = arg_root;
                            recursive_create_ast(tokens, logic_indentation, function_declaration,
                                                 to_return, reg, in_root, 0, end_num, i);
                            arguments.emplace_back(arg_root->next_action);
                        }

                        if (arguments.size() != std::get<2>(bfn.second).size()) {
                            throw std::logic_error("Too many arguments");
                        }

                        if (std::get<0>(bfn.second) != "return") {
                            auto new_root = std::make_shared<FunctionCallAction>(FunctionCallAction{
                                                                           BaseAction{ActionType::FunctionCall},
                                                                           FunctionType::BuiltinFunction,
                                                                           bfn.first,
                                                                           id,
                                                                           std::get<1>(bfn.second),
                                                                           arguments});
                            root->next_action = new_root;
                            root = new_root;
                        } else {
                            auto new_root = std::make_shared<ReturnCall>(ReturnCall{
                                    BaseAction{ActionType::ReturnCall},
                                    std::shared_ptr<BaseAction>(arguments[0])});
                            root->next_action = new_root;
                            root = new_root;
                        }
                    } else {
                        auto ufn = reg.get_user_def_fn(id);

                        //var type, is_reference, var id
                        std::vector<std::shared_ptr<BaseAction>> arguments;

                        if (tokens[++i] != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function call."};}

                        while (tokens[++i] != Token::RIGHT_CIRCLE_BRACKET) {
                            auto arg_root = std::make_shared<BaseAction>();
                            auto in_root = arg_root;
                            recursive_create_ast(tokens, logic_indentation, function_declaration,
                                                 to_return, reg, in_root, 0, end_num, i);
                            arguments.emplace_back(arg_root->next_action);
                        }

                        auto new_root = std::make_shared<FunctionCallAction>(FunctionCallAction{
                                BaseAction{ActionType::FunctionCall},
                                FunctionType::UserFunction,
                                id, id, ufn->return_type,
                                arguments});
                        root->next_action = new_root;
                        root = new_root;
                    }
                    continue;
                }

                if (reg.check_variable(id)) {
                    auto temp_root = std::make_shared<VariableCall>(VariableCall{BaseAction{ActionType::VariableCall},
                                                                                 id});
                    root->next_action = temp_root;
                    root = temp_root;
                    continue;
                } else {
                    throw std::logic_error("Undeclared token");
                }

            }
                break;
            case Token::NUMBER: {
                auto type = convert_token_type(tokens[++i]);
                auto value = tokens[++i];
                temp_root = std::make_shared<NumericConst>(NumericConst{BaseAction{ActionType::NumericConst},
                                                                       type, (uint32_t)value});
                root->next_action = temp_root;
                root = temp_root;
            }
                break;

            case Token::STR_BRACKET: {
                std::vector<char> data;
                while (tokens[++i] != Token::STR_BRACKET) {
                    data.emplace_back((char)((int)tokens[i]-num_tokens));
                }
                i++;

                temp_root = std::make_shared<StringConst>(StringConst{BaseAction{ActionType::StringConst},
                                                                      data});
                root->next_action = temp_root;
                root = temp_root;
            }
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
    return (int)tokens[++i];
}

VariableType Parser::convert_token_type(Token token) {
    switch (token) {
        case Token::VOID:  return VariableType::VOID; break;
        case Token::INT:   return VariableType::INT; break;
        case Token::UINT:  return VariableType::UINT; break;
        case Token::FLOAT: return VariableType::FLOAT; break;
        case Token::ARRAY: return VariableType::ARRAY; break;
        case Token::STRING:return VariableType::STRING;
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

bool Parser::is_math_or_logic_token(Token token) {
    switch (token) {
        case Token::ADD:
        case Token::SUB:
        case Token::MULT:
        case Token::DIV:
        case Token::INC:
        case Token::DEC:
        case Token::AND:
        case Token::OR:
        case Token::EQUALS:
        case Token::NOT_EQUALS:
        case Token::NOT:
        case Token::MORE_OR_EQUAL:
        case Token::LESS_OR_EQUAL:
        case Token::MORE:
        case Token::LESS:
        case Token::NUMBER:
            return true;
        default:
            return false;
    }
}

