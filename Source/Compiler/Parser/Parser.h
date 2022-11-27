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

struct ASTCreationResult {
    std::vector<std::shared_ptr<BaseAction>> function_roots;
    IdRegister reg;
};

struct Scope {
    std::vector<std::vector<std::tuple<int, VariableType>>> var_scope;
    std::vector<std::vector<std::tuple<int, std::vector<VariableType>, VariableType>>> fn_scope{{}};
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
    void push_fn(int id, const std::vector<VariableType>& arguments_types, VariableType return_type) {
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
    std::tuple<bool, const std::vector<VariableType>&, VariableType> get_fn(int id) {
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
        if (!check_id(id)) {throw std::logic_error("Variable was not declared in this scope.");}
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

            std::vector<VariableType> args;
            for (auto & arg: std::get<2>(bnf.second)) {args.emplace_back(arg.first);}

            auto ret_type = std::get<1>(bnf.second);

            push_fn(id, args, ret_type);
        }
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

    static bool is_math_or_logic_token(Token token);

public:
    static ASTCreationResult create_ast(TranspilerResult &t_result, bool debug);
    static bool validate_ast(ASTCreationResult &ast_result, bool debug);
    static ActionTreeResult create_action_tree(ASTCreationResult & ast_result);
    static void show_ast(ASTCreationResult &ast_result, IdRegister &id_reg);

    static void recursive_validate(Scope &scope, std::shared_ptr<BaseAction> &root);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_PARSER_H
