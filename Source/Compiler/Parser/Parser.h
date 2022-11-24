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
    std::vector<std::pair<int, std::string>> function_ids;
    std::vector<std::pair<int, std::string>> variable_ids;
    WordRegister & wreg;

    IdRegister(WordRegister & wreg): wreg(wreg) {
        for (int i = 0; i < wreg.names.size(); i++) {
            auto & item = wreg.names[i];
            if (std::find(builtin_functions_id_names.begin(),
                          builtin_functions_id_names.end(),
                          item) != std::end(builtin_functions_id_names)) {
                function_ids.emplace_back(i, builtin_functions_id_names[i]);
            }
        }
    };

    void register_function(int id);
    void register_variable(int id);

    bool check_function(int id);
    bool check_variable(int id);

    std::string id_to_string(int id) {
        for (auto reg: function_ids) {if (reg.first == id) { return "function " + reg.second;}}
        for (auto reg: variable_ids) {if (reg.first == id) { return "variable " + reg.second;}}
        return "unknown id";
    }
};

class Parser {
    static void
    recursive_create_ast(const std::vector<Token> &tokens, int logic_indentation, bool &function_declaration,
                         ASTCreationResult &to_return, IdRegister &reg, std::shared_ptr<BaseAction> &root,
                         int begin_num, int end_num);

    static void check_token_is_valid_argument(Token token, int &i);

    static VariableType convert_token_type(Token token);
    static int get_id(const std::vector<Token> &tokens, int & i);

    static std::pair<int, bool> is_declared_function(const std::vector<Token> & tokens, int & i);
    static std::pair<int, bool> is_declared_variable(const std::vector<Token> & tokens, int & i);

    static bool try_process_inc_dec_expr(const std::vector<Token> & tokens, int & i, std::shared_ptr<BaseAction> & root);
    static bool try_process_builtin_expression(const std::vector<Token> & tokens, int & i, std::shared_ptr<BaseAction> & root);
    static bool try_process_math_or_logic_expr(const std::vector<Token> & tokens, int & i, std::shared_ptr<BaseAction> & root);

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
