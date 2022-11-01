//
// Created by spaceeye on 28.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_PARSER_H
#define ANOTHERPROGRAMMINGLANGUAGE_PARSER_H

#include <memory>
#include <exception>

#include "../Transpiler/Transpiler.h"
#include "Action/Actions.h"

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

    IdRegister(WordRegister & wreg): wreg(wreg) {};

    void register_function(int id);
    void register_variable(int id);

    void check_function(int id);
    void check_variable(int id);
};

class Parser {
    static void
    recursive_create_ast(const std::vector<Token> &tokens, int logic_indentation, bool &function_declaration,
                         ASTCreationResult &to_return, IdRegister &reg, std::shared_ptr<BaseAction> &root,
                         int begin_num, int end_num);

    static void check_token_is_valid_argument(Token token, int &i);

    static VariableType convert_token_type(Token token);
    static int get_id(const std::vector<Token> &tokens, int & i);

public:
    static ASTCreationResult create_ast(TranspilerResult & t_result);
    static ActionTreeResult create_action_tree(ASTCreationResult & ast_result);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_PARSER_H
