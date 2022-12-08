//
// Created by spaceeye on 28.10.22.
//

#include "../Parser.h"

ASTCreationResult Parser::create_ast(TranspilerResult &t_result, bool debug) {
    ASTCreationResult to_return{{}, IdRegister{t_result.wreg}};
    std::vector<std::pair<Token, std::pair<int, int>>> & tokens = t_result.tokens;

    int i = 0;
    while (i < tokens.size() - 1) {
        int logic_indentation = 0;
        bool function_declaration = false;

        auto root = std::make_shared<BaseAction>();
        auto beginning = std::shared_ptr{root};

        std::vector<Bracket> brackets_stack;

        recursive_create_ast(tokens, logic_indentation, function_declaration, to_return, to_return.reg, root, 0,
                             tokens.size(), i, brackets_stack, 0, 0);

        i++;
        to_return.object_roots.emplace_back(beginning->next_action);
    }

    if (debug) {show_ast(to_return, to_return.reg);}
    return to_return;
}

void
Parser::recursive_create_ast(const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int &logic_indentation,
                             bool &function_declaration, ASTCreationResult &to_return, IdRegister &reg,
                             std::shared_ptr<BaseAction> &root, int begin_num, int end_num, int &i,
                             std::vector<Bracket> &brackets_stack, int if_statement_nesting, int do_not_recurse) {
    std::shared_ptr<BaseAction> temp_root;
    for (; i < end_num; i++) {
        auto token = tokens[i];

        switch (token.first) {
            case Token::FUNCTION: {
                function_declaration = true;

                auto fn_decl_pos = tokens[i].second;

                bool is_inline = false;
                if (tokens[++i].first == Token::INLINE) {is_inline = true;} else {--i;}

                VariableType return_type;

                return_type = convert_token_type(tokens[++i]);

                int id = get_id(tokens, ++i);

                if (reg.check_exists(id)) { throw std::logic_error("Id already exists. "
                                                                   "Row " + std::to_string(tokens[i].second.first) +
                                                                   " col " + std::to_string(tokens[i].second.first));}

                //var type, is_reference, var id
                std::vector<std::tuple<VariableType, bool, int>> arguments;

                if (tokens[++i].first != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function declaration. "
                                                                                             "Row " + std::to_string(tokens[i].second.first) +
                                                                                             " col " + std::to_string(tokens[i].second.second)};}
                while (tokens[++i].first != Token::RIGHT_CIRCLE_BRACKET) {
                    check_token_is_valid_argument(tokens[i], i);

                    auto var_type = convert_token_type(tokens[i]);
                    auto is_ref = false; if (tokens[++i].first == Token::REF) {is_ref=true;} else {--i;}
                    auto var_id = get_id(tokens, ++i);

                    reg.register_variable(var_id);

                    arguments.emplace_back(var_type, is_ref, var_id);
                }


                auto temp_root = std::make_shared<FunctionDeclaration>(FunctionDeclaration{
                    BaseAction{ActionType::FunctionDeclaration, nullptr, fn_decl_pos},
                    id, is_inline, return_type, arguments});
                reg.register_function(id, temp_root.get());
                root->next_action = temp_root;
                root = temp_root;
            }
                break;
            case Token::VAR: {
                auto var_pos = token.second;
                if (logic_indentation == 0) {throw std::logic_error{"Can't declare global variable. "
                                                                    "Row " + std::to_string(token.second.first) +
                                                                    " col " + std::to_string(token.second.second)};}
                if (tokens[++i].first == Token::ARRAY) {

                } else {
                    auto var_type = convert_token_type(tokens[i]);

                    bool is_ref = false;
                    if (tokens[++i].first != Token::REF) {is_ref = true;} else {--i;}

                    auto var_id = get_id(tokens, i);
                    reg.register_variable(var_id);

                    temp_root = std::make_shared<VariableDeclaration>(VariableDeclaration{
                        BaseAction{ActionType::VariableDeclaration, nullptr, var_pos},
                        var_type, var_id, is_ref});
                    root->next_action = temp_root;
                    root = temp_root;
                }
            }
                break;
            case Token::IF: {
                auto if_statement_pos = token.second;

                std::vector<std::shared_ptr<BaseAction>> arguments;
                get_arguments(end_num, tokens, logic_indentation, function_declaration, to_return, reg, i,
                              brackets_stack, arguments, "if statement.");

                if (arguments.empty()) {throw std::logic_error("No arguments in an if statement. "
                                                               "Row " + std::to_string(if_statement_pos.first) +
                                                               " col " + std::to_string(if_statement_pos.second));}
                if (arguments.size() != 1) {throw std::logic_error("More than one argument in if statement. "
                                                                   "Row " + std::to_string(if_statement_pos.first) +
                                                                   " col " + std::to_string(if_statement_pos.second));}

                auto true_branch = std::make_shared<BaseAction>(BaseAction{});
                {
                    if (tokens[++i].first != Token::BEGIN_LOGIC_BLOCK) {throw std::logic_error("Invalid if statement declaration. "
                                                                                               "Row " + std::to_string(tokens[i].second.first) +
                                                                                               " col " + std::to_string(tokens[i].second.second));}
                    auto in_root = true_branch;
                    recursive_create_ast(tokens, logic_indentation, function_declaration,
                                         to_return, reg, in_root, 0, end_num, i, brackets_stack, 0,
                                         do_not_recurse+1);
                }
                std::shared_ptr<BaseAction> false_branch = std::make_shared<BaseAction>(BaseAction{});
                if (tokens[i+1].first== Token::ELSE) {
                    i++;
                    if (tokens[++i].first== Token::IF || tokens[i].first== Token::BEGIN_LOGIC_BLOCK) {
                        auto in_root = false_branch;
                        recursive_create_ast(tokens, logic_indentation, function_declaration,
                                             to_return, reg, in_root, 0, end_num, i, brackets_stack,
                                             if_statement_nesting + 1, do_not_recurse+1);
                    } else {throw std::logic_error("Invalid if statement declaration. "
                                                   "Row " + std::to_string(tokens[i].second.first) +
                                                   " col " + std::to_string(tokens[i].second.second));}
                }

                auto if_action = std::make_shared<IfStatement>(IfStatement{
                    BaseAction{ActionType::IfStatement, nullptr, if_statement_pos},
                    arguments[0], true_branch->next_action, false_branch->next_action});

                root->next_action = if_action;
                root = if_action;

                if (if_statement_nesting) { return;}
            }
                break;
            case Token::WHILE: {
                auto while_st_pos = token.second;
                std::vector<std::shared_ptr<BaseAction>> arguments;
                get_arguments(end_num, tokens, logic_indentation, function_declaration, to_return, reg, i,
                              brackets_stack, arguments, "while statement.");

                if (arguments.empty()) {throw std::logic_error("No arguments in a while statement."
                                                               "Row " + std::to_string(while_st_pos.first) +
                                                               " col " + std::to_string(while_st_pos.second));}
                if (arguments.size() != 1) {throw std::logic_error("More than one argument in a while statement."
                                                                   "Row " + std::to_string(while_st_pos.first) +
                                                                   " col " + std::to_string(while_st_pos.second));}

                auto body = std::make_shared<BaseAction>(BaseAction{});

                if (tokens[++i].first!= Token::BEGIN_LOGIC_BLOCK) {throw std::logic_error("Invalid while declaration."
                                                                                          "Row " + std::to_string(tokens[i].second.first) +
                                                                                          " col " + std::to_string(tokens[i].second.second));}
                auto in_root = body;
                recursive_create_ast(tokens, logic_indentation, function_declaration,
                                     to_return, reg, in_root, 0, end_num, i, brackets_stack, 0, do_not_recurse+1);

                auto while_action = std::make_shared<WhileLoop>(WhileLoop{
                    BaseAction{ActionType::WhileLoop, nullptr, while_st_pos},
                    arguments[0],
                    body->next_action
                });

                root->next_action = while_action;
                root = while_action;
            }
                break;
            case Token::FOR:
                break;
            case Token::LEFT_CIRCLE_BRACKET:
                brackets_stack.emplace_back(Bracket::Circle);
                break;
            case Token::RIGHT_CIRCLE_BRACKET:
                if (brackets_stack.empty()) {throw std::logic_error("Unclosed circle bracket.");}
                if (brackets_stack.back() != Bracket::Circle) {throw std::logic_error("Unmatched circle bracket.");}

                brackets_stack.pop_back();

                i--;
                return;
                break;
            case Token::LEFT_BOX_BRACKET:
                brackets_stack.emplace_back(Bracket::Box);
                break;
            case Token::RIGHT_BOX_BRACKET:
                if (brackets_stack.empty()) {throw std::logic_error("Unclosed box bracket.");}
                if (brackets_stack.back() != Bracket::Circle) {throw std::logic_error("Unmatched box bracket.");}

                brackets_stack.pop_back();
                return;
                break;
            case Token::BEGIN_LOGIC_BLOCK:
                brackets_stack.emplace_back(Bracket::Fancy);

                logic_indentation++;
                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::StartLogicBlock});
                root->next_action = temp_root;
                root = temp_root;
                if (!do_not_recurse) {
                    recursive_create_ast(tokens, logic_indentation, function_declaration, to_return, reg, root,
                                         begin_num, end_num, ++i, brackets_stack, 0, do_not_recurse);
                } else {
                    do_not_recurse--;
                }
                break;
            case Token::END_LOGIC_BLOCK:
                if (brackets_stack.size() == 0) {throw std::logic_error("Unclosed scope bracket.");}
                if (brackets_stack.back() != Bracket::Fancy) {throw std::logic_error("Unmatched scope bracket.");}

                brackets_stack.pop_back();

                logic_indentation--;

                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::EndLogicBlock});
                root->next_action = temp_root;
                root = temp_root;
                return;
                break;
            case Token::END_COMMAND:
                if (brackets_stack.back() == Bracket::Circle) {throw std::logic_error("Unclosed circle brackets before end command.");}
                else if (brackets_stack.back() == Bracket::Circle) {throw std::logic_error("Unclosed box brackets before end command.");}

                break;
            case Token::COMMA:
                return;
                break;
            case Token::UNK_WORD: {
                //TODO make logic for x, y, z = array_output_func();

                auto id = (int)tokens[++i].first;
                token = tokens[i];

                if (reg.check_function(id)) {
                    if (reg.is_builtin_fn(id)) {
                        auto bfn = reg.get_builtin(id);

                        std::vector<std::shared_ptr<BaseAction>> arguments;
                        get_arguments(end_num, tokens, logic_indentation, function_declaration, to_return, reg, i,
                                      brackets_stack, arguments, "function call.");

                        if (std::get<0>(bfn.second) != "return") {
                            auto new_root = std::make_shared<FunctionCallAction>(FunctionCallAction{
                                BaseAction{ActionType::FunctionCall, nullptr, token.second},
                                FunctionType::BuiltinFunction, bfn.first, id, std::get<1>(bfn.second), arguments});
                            root->next_action = new_root;
                            root = new_root;
                        } else {
                            if (arguments.size() == 0) {arguments.emplace_back(nullptr);}
                            auto new_root = std::make_shared<ReturnCall>(ReturnCall{
                                    BaseAction{ActionType::ReturnCall, nullptr, token.second},
                                    std::shared_ptr<BaseAction>(arguments[0])});
                            root->next_action = new_root;
                            root = new_root;
                        }
                    } else {
                        auto ufn = reg.get_user_def_fn(id);

                        //var type, is_reference, var id
                        std::vector<std::shared_ptr<BaseAction>> arguments;

                        if (tokens[++i].first!= Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function call. "
                                                                                                    "Row " + std::to_string(tokens[i].second.first) +
                                                                                                    " col " + std::to_string(tokens[i].second.second)};}

                        brackets_stack.emplace_back(Bracket::Circle);
                        if (tokens[i+1].first == Token::RIGHT_CIRCLE_BRACKET) {brackets_stack.pop_back();}
                        while (tokens[++i].first!= Token::RIGHT_CIRCLE_BRACKET) {
                            auto arg_root = std::make_shared<BaseAction>();
                            auto in_root = arg_root;
                            recursive_create_ast(tokens, logic_indentation, function_declaration,
                                                 to_return, reg, in_root, 0, end_num, i, brackets_stack, 0,
                                                 do_not_recurse);
                            arguments.emplace_back(arg_root->next_action);
                        }

                        auto new_root = std::make_shared<FunctionCallAction>(FunctionCallAction{
                                BaseAction{ActionType::FunctionCall, nullptr, token.second},
                                FunctionType::UserFunction, id, id, ufn->return_type, arguments});
                        root->next_action = new_root;
                        root = new_root;
                    }
                    continue;
                }

                if (reg.check_variable(id)) {
                    auto temp_root = std::make_shared<VariableCall>(VariableCall{
                        BaseAction{ActionType::VariableCall, nullptr, token.second}, id});
                    root->next_action = temp_root;
                    root = temp_root;
                    continue;
                } else {
                    throw std::logic_error(std::string{"Undeclared id at"}
                    + " row " + std::to_string(token.second.first)
                    + " col " + std::to_string(token.second.second));
                }

            }
                break;
            case Token::NUMBER: {
                auto type = convert_token_type(tokens[++i]);
                auto value = tokens[++i];
                temp_root = std::make_shared<NumericConst>(NumericConst{
                    BaseAction{ActionType::NumericConst, nullptr, token.second},
                    type, *(uint32_t*)&value});
                root->next_action = temp_root;
                root = temp_root;
            }
                break;

            case Token::STR_BRACKET: {
                std::vector<char> data;
                bool ended = false;
                while (i < tokens.size()) {
                    if (tokens[++i].first == Token::STR_BRACKET) {
                        ended = true;break;
                    }

                    if (!ended) {throw std::logic_error("String bracket isn't closed. "
                                                        "Row " + std::to_string(token.second.first) +
                                                        " col " + std::to_string(token.second.second));}
                    data.emplace_back((char)((int)tokens[i].first-num_tokens));
                }
                i++;

                temp_root = std::make_shared<StringConst>(StringConst{
                    BaseAction{ActionType::StringConst, nullptr, token.second}, data});
                root->next_action = temp_root;
                root = temp_root;
            }
                break;

            case Token::ELSE:
                break;

            case Token::ARRAY:
            case Token::INT:
            case Token::UINT:
            case Token::FLOAT:
            case Token::VOID:
            case Token::COMMENT:
            case Token::INLINE:
                throw std::logic_error(std::string{"Invalid token at "}
                + "row " + std::to_string(token.second.first)
                + " col " + std::to_string(token.second.second));
            default:
                throw std::logic_error{"Shouldn't happen."};
        }
    }

    if (!brackets_stack.empty()) {
               if (brackets_stack.back() == Bracket::Circle) {throw std::logic_error("Unclosed circle brackets.");
        } else if (brackets_stack.back() == Bracket::Box)    {throw std::logic_error("Unclosed box brackets.");
        } else if (brackets_stack.back() == Bracket::Fancy)  {throw std::logic_error("Unclosed scope.");
        }
    }
}

void Parser::get_arguments(int end_num, const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int &logic_indentation, bool &function_declaration,
                           ASTCreationResult &to_return, IdRegister &reg, int &i, std::vector<Bracket> &brackets_stack,
                           std::vector<std::shared_ptr<BaseAction>> &arguments, std::string name) {
    if (tokens[++i].first != Token::LEFT_CIRCLE_BRACKET) {
        throw std::logic_error{
                std::string{"No closing circle bracket in "} + name + " declaration at "
                + "row " + std::to_string(tokens[i].second.first)
                + " col " + std::to_string(tokens[i].second.second)};
    }

    brackets_stack.emplace_back(Bracket::Circle);

    if (tokens[i + 1].first == Token::RIGHT_CIRCLE_BRACKET) { brackets_stack.pop_back(); }
    while (tokens[++i].first != Token::RIGHT_CIRCLE_BRACKET) {
        auto arg_root = std::make_shared<BaseAction>();
        auto in_root = arg_root;
        recursive_create_ast(tokens, logic_indentation, function_declaration,
                             to_return, reg, in_root, 0, end_num, i, brackets_stack, 0, 0);
        arg_root = arg_root->next_action;

        switch (arg_root->act_type) {
            case ActionType::VariableCall:
            case ActionType::FunctionCall:
            case ActionType::NumericConst:
            case ActionType::StringConst:
                break;
            default:
                throw std::logic_error("Invalid argument action type.");
        }

        if (arg_root->next_action != nullptr) {
            throw std::logic_error("Invalid declaration of an argument.");
        }

        arguments.emplace_back(arg_root);
    }
}

int Parser::get_id(const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int &i) {
    return (int)tokens[++i].first;
}

VariableType Parser::convert_token_type(const std::pair<Token, std::pair<int, int>> &token) {
    switch (token.first) {
        case Token::VOID:  return VariableType::VOID; break;
        case Token::INT:   return VariableType::INT; break;
        case Token::UINT:  return VariableType::UINT; break;
        case Token::FLOAT: return VariableType::FLOAT; break;
        case Token::ARRAY: return VariableType::ARRAY; break;
        case Token::STRING:return VariableType::STRING;
        default:
            throw std::logic_error{
                "Unknown type in variable type at "
                "row " + std::to_string(token.second.first) +
                " col " + std::to_string(token.second.second) + "."};
    }
}

void Parser::check_token_is_valid_argument(const std::pair<Token, std::pair<int, int>> &token, int &i) {
    switch (token.first) {
        case Token::INT:
        case Token::UINT:
        case Token::FLOAT:
        case Token::ARRAY:
            break;
        case Token::COMMA:
            ++i;
            break;
        default:
            throw std::logic_error{"Invalid function argument token at "
                                   " row " + std::to_string(token.second.first) +
                                   " col " + std::to_string(token.second.second) +
                                   "."};
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

