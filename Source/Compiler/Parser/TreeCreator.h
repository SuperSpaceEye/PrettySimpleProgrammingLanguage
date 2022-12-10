//
// Created by spaceeye on 10.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREECREATOR_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREECREATOR_H


#include "TreeValidator.h"
#include "Results.h"
#include "TreeOptimizer.h"
#include "ValidateScope.h"
#include "IdRegister.h"
#include "CommonFNS.h"
#include "../OptionsContainer.h"
#include "BuiltinFunctions.h"
#include "Action/Actions.h"
#include "../Transpiler/Transpiler.h"
#include <iostream>
#include <exception>
#include <memory>

enum class Bracket {
    Circle,
    Box,
    Fancy,
};

class TreeCreator {
    static void process_unk_word(int end_num, int do_not_recurse, std::pair<Token, std::pair<int, int>> &token,
                                 const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                 int &logic_indentation,
                                 bool &function_declaration, ASTCreationResult &to_return, IdRegister &reg,
                                 std::shared_ptr<BaseAction> &node, int &i, std::vector<Bracket> &brackets_stack);

    static void create_variable_call_node(std::shared_ptr<BaseAction> &node, const std::pair<Token, std::pair<int, int>> &token,
                                          int id);

    static void create_user_defined_fn_call_node(int end_num, int do_not_recurse,
                                                 const std::pair<Token, std::pair<int, int>> &token,
                                                 int id, const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                                 int &logic_indentation, bool &function_declaration, ASTCreationResult &to_return,
                                                 IdRegister &reg, std::shared_ptr<BaseAction> &node, int &i,
                                                 std::vector<Bracket> &brackets_stack);

    static void create_builtin_fn_call_node(const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int end_num,
                                            const std::pair<Token, std::pair<int, int>> &token, int id,
                                            int &logic_indentation, bool &function_declaration,
                                            ASTCreationResult &to_return, IdRegister &reg,
                                            std::shared_ptr<BaseAction> &node, int &i,
                                            std::vector<Bracket> &brackets_stack);

    static void create_str_const_node(const std::pair<Token, std::pair<int, int>> &token,
                                      const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                      std::shared_ptr<BaseAction> &node, int &i,
                                      std::shared_ptr<BaseAction> &temp_root);

    static void create_num_const_node(const std::pair<Token, std::pair<int, int>> &token,
                                      const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                      std::shared_ptr<BaseAction> &node, int &i, std::shared_ptr<BaseAction> &temp_root);

    static void create_while_node(int end_num, int do_not_recurse, const std::pair<Token, std::pair<int, int>> &token,
                                  const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                  int &logic_indentation,
                                  bool &function_declaration, ASTCreationResult &to_return, IdRegister &reg,
                                  std::shared_ptr<BaseAction> &node, int &i, std::vector<Bracket> &brackets_stack);

    static void create_if_statement_node(int end_num, int if_statement_nesting, int do_not_recurse,
                                         const std::pair<Token, std::pair<int, int>> &token,
                                         const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                         int &logic_indentation, bool &function_declaration,
                                         ASTCreationResult &to_return,
                                         IdRegister &reg, std::shared_ptr<BaseAction> &node, int &i,
                                         std::vector<Bracket> &brackets_stack);

    static void create_var_decl_node(const int &logic_indentation, IdRegister &reg,
                                     const std::pair<Token, std::pair<int, int>> &token,
                                     const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                     std::shared_ptr<BaseAction> &node, int &i, std::shared_ptr<BaseAction> &temp_root);

    static void create_fn_decl_node(IdRegister &reg, const std::vector<std::pair<Token, std::pair<int, int>>> &tokens,
                                    bool &function_declaration, std::shared_ptr<BaseAction> &root, int &i);

    static void
    get_arguments(int end_num, const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int &logic_indentation, bool &function_declaration,
                  ASTCreationResult &to_return, IdRegister &reg, int &i, std::vector<Bracket> &brackets_stack,
                  std::vector<std::shared_ptr<BaseAction>> &arguments, std::string name);

    static void recursive_create_ast(const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int &logic_indentation,
                                     bool &function_declaration, ASTCreationResult &to_return, IdRegister &reg,
                                     std::shared_ptr<BaseAction> &node, int begin_num, int end_num, int &i,
                                     std::vector<Bracket> &brackets_stack, int if_statement_nesting, int do_not_recurse);

    static int get_id(const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int & i);

    static VariableType convert_token_type(const std::pair<Token, std::pair<int, int>> &token);

    static void check_token_is_valid_argument(const std::pair<Token, std::pair<int, int>> &token, int &i);

public:
    static ASTCreationResult create_ast(TranspilerResult &t_result, bool debug);
};


#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREECREATOR_H
