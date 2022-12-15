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
#include "../OptionsContainer.h"
#include "CommonFNS.h"
#include "IdRegister.h"
#include "ValidateScope.h"
#include "TreeOptimizer.h"
#include "Results.h"
#include "TreeValidator.h"
#include "TreeCreator.h"

class ActionTreeCreator {
    static void show_ast(std::vector<std::shared_ptr<BaseAction>> &roots, IdRegister &id_reg);
    static TreeResult extract_nested_functions(ASTCreationResult &ast_result);
    static int get_main_fn_idx(TreeResult &t_res, int main_id);
public:
    static TreeResult create_tree(TranspilerResult &t_result, const Options &options) {
        auto ast = TreeCreator::create_ast(t_result, options.debug.show_parser_output);
//        if (options.debug.show_parser_output) {show_ast(ast.object_roots, ast.reg);}

        TreeValidator::validate_ast(ast, options);
        auto tree = extract_nested_functions(ast);
        if (options.debug.show_parser_output) {show_ast(tree.object_roots, ast.reg);}
        TreeValidator::validate_function_paths(tree);

        TreeOptimizer::optimize_tree(tree, options);
        if (options.debug.show_optimized_tree) {show_ast(tree.object_roots, ast.reg);}

        return tree;
    }
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_PARSER_H
