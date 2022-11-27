//
// Created by spaceeye on 28.10.22.
//

#include "Parser.h"

ASTCreationResult Parser::create_ast(TranspilerResult &t_result, bool debug) {
    ASTCreationResult to_return{};
    IdRegister reg{t_result.wreg};
    std::vector<Token> & tokens = t_result.tokens;

    int i = 0;
    while (i < tokens.size() - 1) {
        int logic_indentation = 0;
        bool function_declaration = false;

        auto root = std::make_shared<BaseAction>();
        auto beginning = std::shared_ptr{root};

        recursive_create_ast(tokens, logic_indentation, function_declaration, to_return, reg, root, 0, tokens.size(), i);

        i++;
        to_return.function_roots.emplace_back(beginning);
    }

    if (debug) {show_ast(to_return, reg);}
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
                if (logic_indentation != 0) { throw std::logic_error{"Functions should be declared at top level."}; }
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
                                                                                          var_type, var_id, true, is_ref});
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
                break;
            case Token::END_LOGIC_BLOCK:
                logic_indentation--;
                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::EndLogicBlock});
                root->next_action = temp_root;
                root = temp_root;
                return;
                break;
            case Token::END_COMMAND:
                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::EndAction});
                root->next_action = temp_root;
                root = temp_root;
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

                        if (tokens[++i] != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function declaration."};}

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

                        if (tokens[++i] != Token::LEFT_CIRCLE_BRACKET) {throw std::logic_error{"Invalid function declaration."};}

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
                    auto temp_root = std::make_shared<VariableCall>(BaseAction{ActionType::VariableCall}, id);
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

bool Parser::is_assignment_expression(const std::vector<Token> &tokens, int &i) {
    switch (tokens[i + 2]) {
        case Token::ASGN:
        case Token::ADD_ASGN:
        case Token::SUB_ASGN:
        case Token::MULT_ASGN:
        case Token::DIV_ASGN:
            return true;
        default:
            return false;
    }
}

bool Parser::is_inc_dec_expression(const std::vector<Token> &tokens, int &i) {
    switch (tokens[i + 2]) {
        case Token::INC:
        case Token::DEC:
            return true;
        default:
            return false;
    }
}

bool Parser::is_math_or_logic_expr(const std::vector<Token> &tokens, int i) {
    int logic_indentation = 0;

    while (true) {
        if (logic_indentation == 0 && tokens[i] == Token::RIGHT_CIRCLE_BRACKET) { break;}
        if (tokens[i] == Token::RIGHT_CIRCLE_BRACKET) {--logic_indentation; ++i;continue;}
        if (tokens[i] == Token::LEFT_CIRCLE_BRACKET) {++logic_indentation; ++i; continue;}

        if (tokens[i] == Token::UNK_WORD) {i+=2; continue;}
        if (is_math_or_logic_token(tokens[i])) {++i; continue;}

        return false;
    }
    return true;
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

void IdRegister::register_function(int id, FunctionDeclaration *fn_dec) {
    for (auto & ids: variable_ids) {
        if (std::get<0>(ids) == id) {throw std::logic_error{"VariableDeclaration with id already exists."};}
    }

    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) {throw std::logic_error{"Function with id already exists."};}
    }

    function_ids.emplace_back(id, wreg.get_word((Token)id), fn_dec);
}

void IdRegister::register_variable(int id) {
    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) {throw std::logic_error{"Function with id already exists."};}
    }

    for (auto & ids: variable_ids) {
        if (ids.first == id) {
//            throw std::logic_error{"VariableDeclaration with id already exists."};
            return;
        }
    }

    variable_ids.emplace_back(id, wreg.get_word((Token)id));
}

bool IdRegister::check_function(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {return false;}
    }

    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) { return true;}
    }

    throw std::logic_error{"No function was declared."};
}

bool IdRegister::check_exists(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {return true;}
    }

    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) { return true;}
    }

    return false;
}

bool IdRegister::check_variable(int id) {
    for (auto & ids: function_ids) {
        if (std::get<0>(ids) == id) {return false;}
    }

    for (auto & ids: variable_ids) {
        if (ids.first == id) { return true;}
    }
    throw std::logic_error{"VariableDeclaration with id wasn't declared."};
}

bool IdRegister::is_builtin_fn(int id) {
    for (auto item: builtins) {
        if (item == id) {
            return true;
        }
    }
    return false;
}

std::pair<int, const std::tuple<std::string, VariableType, std::vector<std::pair<VariableType, bool>>>&>
IdRegister::get_builtin(int id) {
    for (auto & item: function_ids) {
        if (std::get<0>(item) == id) {
            for (int i = 0; i < builtin_functions_id_names.size(); i++) {
                auto & bitem = builtin_functions_id_names[i];
                if (std::get<0>(bitem) == std::get<1>(item)) {
                    return {i, builtin_functions_id_names[i]};
                }
            }
        }
    }
    throw std::logic_error("shouldn't happen");
}

FunctionDeclaration *IdRegister::get_user_def_fn(int id) {
    for (auto & item: function_ids) {
        if (std::get<0>(item) == id) {
            return std::get<2>(item);
        }
    }
    return nullptr;
}


std::string get_string_type(VariableType type) {
    switch (type) {
        case VariableType::VOID:   return "void";
        case VariableType::INT:    return "int";
        case VariableType::UINT:   return "uint";
        case VariableType::FLOAT:  return "float";
        case VariableType::ARRAY:  return "array";
        case VariableType::STRING: return "string";
        case VariableType::B_ANY:  return "any type(checks should be implemented by dev)";
        default: throw std::logic_error("not implemented type");
    }
}

std::string pi(int indentation) {
    std::string buff;
    for (int i = 0; i < indentation; i++) {
        buff += " ";
    }
    return buff;
}

void display_variable(BaseAction *root, int indentation, IdRegister &reg) {
    auto & var_root = *static_cast<VariableDeclaration*>(root);
    std::cout << pi(indentation) << get_string_type(var_root.var_type) << " "
    << reg.id_to_string(var_root.var_id) << " "
    << (var_root.is_declaration ? "decl ": "") << (var_root.reference ? "ref\n": "\n");
}

std::string num_to_str(NumericConst & var) {
    switch (var.type) {
        case VariableType::INT:   return std::to_string((int32_t)var.value);
        case VariableType::UINT:  return std::to_string(var.value);
        case VariableType::FLOAT: return std::to_string((float)var.value);
    }
}

void display_root(BaseAction *root, int indentation, IdRegister &reg) {
    if (root == nullptr) { return;}
    switch (root->act_type) {
        case ActionType::NoType: {
            std::cout << pi(indentation) << "!!!no type\n";
        }
            break;
        case ActionType::EndAction: {

        }
            break;
        case ActionType::VariableDeclaration: {
            display_variable(root, indentation, reg);
        }
            break;
        case ActionType::FunctionCall: {
            auto & fnc = *static_cast<FunctionCallAction*>(root);
            std::cout << pi(indentation) << "Call of " << reg.id_to_string(fnc.name_id)
            << " returns " << get_string_type(fnc.return_type) << " type " <<
            (fnc.fn_type == FunctionType::BuiltinFunction ? "\"builtin\" ": "\"user declared\" ")
            << "\n";
            std::cout << pi(indentation+2) << "Arguments " << fnc.arguments.size() << ":\n";
            for (auto & item: fnc.arguments) {
                display_root(item.get(), indentation+4, reg);
            }
        }
        break;
        case ActionType::VariableCall: {
            auto & var = *static_cast<VariableCall*>(root);
            std::cout << pi(indentation) << "Call of " << reg.id_to_string(var.var_id) << "\n";
        }
            break;
        case ActionType::ForLoop: {

        }
            break;
        case ActionType::WhileLoop: {

        }
            break;
        case ActionType::IfStatement: {

        }
            break;
        case ActionType::StartLogicBlock: {

        }
            break;
        case ActionType::EndLogicBlock: {

        }
            break;
        case ActionType::NumericConst: {
            auto & num_var = *static_cast<NumericConst*>(root);
            std::cout << pi(indentation) << "Call of numeric const " << num_to_str(num_var) << "\n";
        }
            break;
        case ActionType::ReturnCall: {
            auto & ret_call = *static_cast<ReturnCall*>(root);
            std::cout << pi(indentation) << "Return call of\n";
            display_root(ret_call.argument.get(), indentation+2, reg);
        }
    }
    display_root(root->next_action.get(), indentation, reg);
}

void Parser::show_ast(ASTCreationResult &ast_result, IdRegister &id_reg) {
    for (auto root: ast_result.function_roots) {
        std::cout << "\n\n";
        int indentation = 1;
        //TODO !!!!
        root = root.get()->next_action;
        auto & fn_dec = *static_cast<FunctionDeclaration*>(root.get());
        std::cout << "Declaration " << (fn_dec.is_inline ? "inline ": "")
        << id_reg.id_to_string(fn_dec.fn_id) << " returns " << get_string_type(fn_dec.return_type)
        << " arguments " << fn_dec.arguments.size() << ":";
        for (int i = 0; i < fn_dec.arguments.size(); i++) {
            auto & arg = fn_dec.arguments[i];
            std::cout << " " << get_string_type(std::get<0>(arg)) << " ";
            std::cout << (std::get<1>(arg) ? "is_ref " : "");
            std::cout << id_reg.id_to_string(std::get<2>(arg)) << (i < fn_dec.arguments.size()-1 ? " |" : "");
        }
        std::cout << "\n\n";
        display_root(root.get()->next_action.get(), indentation + 2, id_reg);
    }
}
