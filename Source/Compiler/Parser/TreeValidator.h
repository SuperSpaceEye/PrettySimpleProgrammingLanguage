//
// Created by spaceeye on 10.12.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREEVALIDATOR_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREEVALIDATOR_H


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

class TreeValidator {

    static bool check_for_main(ASTCreationResult & ast_result);
    static void recursive_validate(ValidateScope &scope, std::shared_ptr<BaseAction> &root, std::vector<int> &ids,
                                   int &last_id, std::shared_ptr<BaseAction> &prev_root, const Options &options);
    static void start_logic_block(ValidateScope &scope, std::shared_ptr<BaseAction> &root, std::vector<int> &ids,
                                  int &last_id, std::shared_ptr<BaseAction> &prev_root, int &do_not_push_scope,
                                  const Options &options);
    static void validate_return_call(const std::shared_ptr<BaseAction> &root, ValidateScope &scope,
                                     std::vector<int> &ids, int &last_id,
                                     std::shared_ptr<BaseAction> &prev_root, const Options &options);
    static void validate_if_statement(std::shared_ptr<BaseAction> &root, ValidateScope &scope, std::vector<int> &ids,
                                      int &last_id, std::shared_ptr<BaseAction> &prev_root, const Options &options);
    static void validate_while_loop(const std::shared_ptr<BaseAction> &root, ValidateScope &scope, std::vector<int> &ids,
                                    int &last_id, std::shared_ptr<BaseAction> &prev_root, const Options &options);
    static void validate_fn_decl(ValidateScope &scope, const std::shared_ptr<BaseAction> &root, int &last_id,
                                 int &do_not_push_scope, const Options &options);
    static void validate_fn_call(const std::shared_ptr<BaseAction> &root, ValidateScope &scope, std::vector<int> &ids,
                                 int &last_id, std::shared_ptr<BaseAction> &prev_root, const Options &options);
    static void validate_variable_call(ValidateScope &scope, const std::shared_ptr<BaseAction> &root, const Options &options);
    static void validate_variable_declaration(ValidateScope &scope, const std::shared_ptr<BaseAction> &root, const Options &options);

    static void make_implicit_cast(std::shared_ptr<BaseAction> &target, VariableType original_type,
                                   VariableType target_type);

    static bool recursive_validate_function_path(std::shared_ptr<BaseAction> node);

    static void try_implicit_cast_fn_arg(const Options &options, bool is_builtin, std::shared_ptr<BaseAction> &arg,
                                         VariableType &req_type, VariableType &got_type);

public:
    static void validate_ast(ASTCreationResult &ast_result, const Options &options);
    static void validate_function_paths(TreeResult &tres);
};


#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_TREEVALIDATOR_H
