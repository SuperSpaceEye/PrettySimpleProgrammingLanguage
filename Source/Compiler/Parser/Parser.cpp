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

    auto root = std::make_shared<BaseAction>();
    auto beginning = std::shared_ptr{root};

    recursive_create_ast(tokens, logic_indentation, function_declaration, to_return, reg, root, 0, tokens.size());

    to_return.function_roots.emplace_back(beginning);

    show_ast(to_return, reg);
    return to_return;
}

void Parser::recursive_create_ast(const std::vector<Token> &tokens, int logic_indentation, bool &function_declaration,
                                  ASTCreationResult &to_return, IdRegister &reg, std::shared_ptr<BaseAction> &root,
                                  int begin_num, int end_num) {
    std::shared_ptr<BaseAction> temp_root;
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

                int id = get_id(tokens, ++i);

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

                temp_root = std::make_shared<FunctionDeclaration>(FunctionDeclaration{BaseAction{ActionType::FunctionDeclaration, root},
                                                                                 id, is_inline, return_type, arguments});
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

                    temp_root = std::make_shared<Variable>(Variable{BaseAction{ActionType::Variable, root},
                                                                        var_type, var_id, true, is_ref});
                    root->next_action = temp_root;
                    root = temp_root;
                }
                break;
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
                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::EndLogicBlock});
                root->next_action = temp_root;
                root = temp_root;
                if (logic_indentation == 0) { return;}
                break;
            case Token::END_COMMAND:
                temp_root = std::make_shared<BaseAction>(BaseAction{ActionType::EndAction});
                root->next_action = temp_root;
                root = temp_root;
                break;
            case Token::COMMA:
                break;
            case Token::UNK_WORD: {
                //TODO make logic for x, y, z = array_output_func();

                if (reg.check_function(int(tokens[++i]))) {

                    continue;
                }

                if (reg.check_variable(int(tokens[i]))) {

                    continue;
                } else {
                    throw std::logic_error("Undeclared token");
                }

//                if (try_process_builtin_expression(tokens, i, root)) { continue;}
//                if (try_process_math_or_logic_expr(tokens, i, root)) { continue;}
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

bool
Parser::try_process_builtin_expression(const std::vector<Token> &tokens, int &i, std::shared_ptr<BaseAction> &root) {
    if (is_inc_dec_expression(tokens, i)) {
        
    }

    return false;
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

bool IdRegister::check_function(int id) {
    for (auto & ids: variable_ids) {
        if (ids.first == id) {return false;}
    }

    for (auto & ids: function_ids) {
        if (ids.first == id) { return true;}
    }

    throw std::logic_error{"No function was declared."};
}

bool IdRegister::check_variable(int id) {
    for (auto & ids: function_ids) {
        if (ids.first == id) {return false;}
    }

    for (auto & ids: variable_ids) {
        if (ids.first == id) { return true;}
    }
    throw std::logic_error{"Variable with id wasn't declared."};
}






std::string get_string_type(VariableType type) {
    switch (type) {
        case VariableType::VOID:   return "void";
        case VariableType::INT:    return "int";
        case VariableType::UINT:   return "uint";
        case VariableType::FLOAT:  return "float";
        case VariableType::ARRAY:  return "array";
        case VariableType::STRING: return "string";
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

void display_variable() {

}

void display_root(BaseAction* root, int indentation) {
    if (root == nullptr) { return;}
    switch (root->act_type) {
        case ActionType::NoType: {
            std::cout << pi(indentation) << "!!!no type\n";
        }
        case ActionType::EndAction: {

        }
        case ActionType::Variable: {

        }
        case ActionType::FunctionCall: {

        }
        case ActionType::FunctionDeclaration: {

        }
        case ActionType::ForLoop: {

        }
        case ActionType::WhileLoop: {

        }
        case ActionType::IfStatement: {

        }
        case ActionType::StartLogicBlock: {

        }
        case ActionType::EndLogicBlock: {

        }
        case ActionType::NumericConst: {

        }
    }
    display_root(root->next_action.get(), indentation);
}

void Parser::show_ast(ASTCreationResult &ast_result, IdRegister &id_reg) {
    for (auto root: ast_result.function_roots) {
        std::cout << "\n\n";
        int indentation = 1;
        //TODO !!!!
        root = root.get()->next_action;
        auto & fn_dec = *static_cast<FunctionDeclaration*>(root.get());
        std::cout << "Declaration " << id_reg.id_to_string(fn_dec.fn_id) << "\n";
        std::cout << "Arguments " << fn_dec.arguments.size() << ":";
        for (auto & arg: fn_dec.arguments) {
            std::cout << "\n  " << get_string_type(std::get<0>(arg)) << " ";
            std::cout << (std::get<1>(arg) ? "is_ref" : "") << " ";
            std::cout << id_reg.id_to_string(std::get<2>(arg)) << "\n";
        }
        display_root(root.get()->next_action.get(), indentation);
    }
}
