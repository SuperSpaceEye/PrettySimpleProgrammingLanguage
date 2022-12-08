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

struct ASTCreationResult {
    std::vector<std::shared_ptr<BaseAction>> object_roots;
    IdRegister reg;
};

struct TreeResult {
    std::vector<std::shared_ptr<BaseAction>> object_roots;
    int main_idx;
};

struct ValidateScope {
    std::vector<std::vector<std::tuple<int, VariableType>>> var_scope;
    std::vector<std::vector<std::tuple<int, std::vector<std::pair<VariableType, bool>>, VariableType>>> fn_scope{{}};
    void push_scope() {
        var_scope.emplace_back();
        fn_scope.emplace_back();
    }
    void pop_scope() {
        var_scope.pop_back();
        fn_scope.pop_back();
    }
    void push_id(int id, VariableType type) {
        var_scope.back().emplace_back(id, type);
    }
    void push_fn(int id, const std::vector<std::pair<VariableType, bool>>& arguments_types, VariableType return_type) {
        fn_scope.back().emplace_back(id, arguments_types, return_type);
    }
    bool check_id(int id) {
        for (int i = var_scope.size()-1; i >= 0; i--) {
            for (auto & sid: var_scope[i]) {
                if (std::get<0>(sid) == id) {return true;}
            }
        }
        return false;
    }
    bool check_fn_id(int id) {
        for (int i = fn_scope.size()-1; i >= 0; i--) {
            for (auto & sid: fn_scope[i]) {
                if (std::get<0>(sid) == id) {return true;}
            }
        }
        return false;
    }
    std::tuple<bool, const std::vector<std::pair<VariableType, bool>>&, VariableType> get_fn(int id) {
        if (!check_fn_id(id)) {throw std::logic_error("Function was not declared in this scope.");}
        for (int i = fn_scope.size()-1; i >= 0; i--) {
            for (auto & fn: fn_scope[i]) {
                if (std::get<0>(fn) == id) {
                    return {true, std::get<1>(fn), std::get<2>(fn)};
                }
            }
        }
    }
    VariableType get_var_type(int id) {
//        if (!check_id(id)) {throw std::logic_error("Variable was not declared in this scope.");}
        for (int i = var_scope.size()-1; i >= 0; i--) {
            for (auto & sid: var_scope[i]) {
                if (std::get<0>(sid) == id) {return std::get<1>(sid);}
            }
        }
        throw std::logic_error("Variable was not declared in this scope.");
    }
    bool check_id_in_lscope(int id) {
        for (auto & sid: var_scope.back()) {
            if (std::get<0>(sid) == id) {return true;}
        }
        return false;
    }

    void populate_builtins(IdRegister & reg) {
        for (int id: reg.builtins) {
            auto bnf = reg.get_builtin(id);

            std::vector<std::pair<VariableType, bool>> args;
            for (auto & arg: std::get<2>(bnf.second)) {args.emplace_back(arg.first, arg.second);}

            auto ret_type = std::get<1>(bnf.second);

            push_fn(id, args, ret_type);
        }
    }
};

enum class Bracket {
    Circle,
    Box,
    Fancy,
};

class Parser {
    static void
    recursive_create_ast(const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int &logic_indentation,
                         bool &function_declaration, ASTCreationResult &to_return, IdRegister &reg,
                         std::shared_ptr<BaseAction> &root, int begin_num, int end_num, int &i,
                         std::vector<Bracket> &brackets_stack, int if_statement_nesting, int do_not_recurse);

    static void check_token_is_valid_argument(const std::pair<Token, std::pair<int, int>> &token, int &i);

    static VariableType convert_token_type(const std::pair<Token, std::pair<int, int>> &token);
    static int get_id(const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int & i);

    static bool is_math_or_logic_token(Token token);

    static void
    recursive_validate(ValidateScope &scope, std::shared_ptr<BaseAction> &root, std::vector<int> &ids, int &last_id,
                       std::shared_ptr<BaseAction> &prev_root);

    static ASTCreationResult create_ast(TranspilerResult &t_result, bool debug);
    static void validate_ast(ASTCreationResult &ast_result);
    static void show_ast(ASTCreationResult &ast_result, IdRegister &id_reg);
    static TreeResult extract_nested_functions(ASTCreationResult &ast_result);

    static int get_main_fn_idx(TreeResult &t_res, int main_id);

    static void
    get_arguments(int end_num, const std::vector<std::pair<Token, std::pair<int, int>>> &tokens, int &logic_indentation, bool &function_declaration,
                  ASTCreationResult &to_return, IdRegister &reg, int &i, std::vector<Bracket> &brackets_stack,
                  std::vector<std::shared_ptr<BaseAction>> &arguments, std::string name);

    static void
    make_implicit_cast(std::shared_ptr<BaseAction> &target, BuiltinIDS function_id, VariableType original_type,
                       VariableType target_type);
public:

    static TreeResult create_tree(TranspilerResult &t_result, const Options &options) {
        //TODO reorder so that extract_nested is before validate
        auto ast = Parser::create_ast(t_result, options.debug.show_parser_output);
        Parser::validate_ast(ast);
        auto tree = extract_nested_functions(ast);
        return tree;
    }
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_PARSER_H
