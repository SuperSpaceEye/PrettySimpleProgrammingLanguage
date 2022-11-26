//
// Created by spaceeye on 28.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_PARSER_H
#define ANOTHERPROGRAMMINGLANGUAGE_PARSER_H

#include <memory>
#include <exception>
#include <iostream>

#include "../Transpiler/Transpiler.h"
#include "Action/Actions.h"
#include "BuiltinFunctions.h"

struct ASTCreationResult {
    std::vector<std::shared_ptr<BaseAction>> function_roots;
    std::vector<int> function_ids;
    std::vector<int> variable_ids;
};

struct ActionTreeResult {

};

class IdRegister {
public:
    std::vector<std::tuple<int, std::string, FunctionDeclaration*>> function_ids;
    std::vector<std::pair<int, std::string>> variable_ids;
    std::vector<int> builtins;
    WordRegister & wreg;

    IdRegister(WordRegister & wreg): wreg(wreg) {
        for (int i = 0; i < wreg.names.size(); i++) {
            auto & item = wreg.names[i];
            for (auto & bitem: builtin_functions_id_names) {
                if (std::get<0>(bitem) == item) {
                    function_ids.emplace_back(i, std::get<0>(bitem), nullptr);
                    builtins.emplace_back(i);
                }
            }
        }
    };

    void register_function(int id, FunctionDeclaration *fn_dec);
    void register_variable(int id);

    bool check_exists(int id);

    bool check_function(int id);
    bool check_variable(int id);

    bool is_builtin_fn(int id);

    std::pair<int, const std::tuple<std::string, VariableType, std::vector<std::pair<VariableType, bool>>>&> get_builtin(int id);
    FunctionDeclaration * get_user_def_fn(int id);

    std::string id_to_string(int id) {
        for (auto reg: function_ids) {if (std::get<0>(reg) == id) { return "fn \"" + std::get<1>(reg) + "\"";}}
        for (auto reg: variable_ids) {if (reg.first == id) { return "var \"" + reg.second + "\"";}}
        return "unknown id";
    }
};

class Parser {
    static void
    recursive_create_ast(const std::vector<Token> &tokens, int logic_indentation, bool &function_declaration,
                         ASTCreationResult &to_return, IdRegister &reg, std::shared_ptr<BaseAction> &root,
                         int begin_num, int end_num, int &i);

    static void check_token_is_valid_argument(Token token, int &i);

    static VariableType convert_token_type(Token token);
    static int get_id(const std::vector<Token> &tokens, int & i);

    static bool is_inc_dec_expression(const std::vector<Token> & tokens, int & i);
    static bool is_assignment_expression(const std::vector<Token> & tokens, int & i);
    static bool is_math_or_logic_expr(const std::vector<Token> & tokens, int i);

    static bool is_math_or_logic_token(Token token);

public:
    static ASTCreationResult create_ast(TranspilerResult & t_result);
    static ActionTreeResult create_action_tree(ASTCreationResult & ast_result);
    static void show_ast(ASTCreationResult &ast_result, IdRegister &id_reg);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_PARSER_H
