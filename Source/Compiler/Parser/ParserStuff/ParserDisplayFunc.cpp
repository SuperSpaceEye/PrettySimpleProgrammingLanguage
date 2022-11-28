//
// Created by spaceeye on 27.11.22.
//

#include "../Parser.h"

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
    << reg.id_to_string(var_root.var_id) << " " << (var_root.reference ? "ref\n": "\n");
}

std::string num_to_str(NumericConst & var) {
    switch (var.type) {
        case VariableType::INT:   return std::to_string((int32_t)var.value);
        case VariableType::UINT:  return std::to_string(var.value);
        case VariableType::FLOAT: return std::to_string((float)var.value);
    }
}

std::string str_const_to_str(StringConst & str) {
    std::string to_return{};

    for (auto chr: str.data) {
        to_return += chr;
    }

    return to_return;
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
        case ActionType::FunctionDeclaration: {
            std::cout << "\n";
            auto & fn_dec = *static_cast<FunctionDeclaration*>(root);
            std::cout << pi(indentation) << "Declaration " << (fn_dec.is_inline ? "inline ": "")
                      << reg.id_to_string(fn_dec.fn_id) << " returns " << get_string_type(fn_dec.return_type)
                      << " | arguments " << fn_dec.arguments.size() << ":";
            for (int i = 0; i < fn_dec.arguments.size(); i++) {
                auto & arg = fn_dec.arguments[i];
                std::cout << " " << get_string_type(std::get<0>(arg)) << " ";
                std::cout << (std::get<1>(arg) ? "is_ref " : "");
                std::cout << reg.id_to_string(std::get<2>(arg)) << (i < fn_dec.arguments.size()-1 ? " |" : "");
            }
            std::cout << "\n";
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
            << "| arguments " << fnc.arguments.size() << ":\n";
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
            indentation+=2;
        }
            break;
        case ActionType::EndLogicBlock: {
            indentation-=2;
            std::cout << "\n";
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
            break;
        case ActionType::StringConst: {
            auto & str_const = *static_cast<StringConst*>(root);
            std::cout << pi(indentation) << "Call of string const \"" << str_const_to_str(str_const) << "\"\n";
        }
            break;
    }
    display_root(root->next_action.get(), indentation, reg);
}

void Parser::show_ast(ASTCreationResult &ast_result, IdRegister &id_reg) {
    for (auto root: ast_result.object_roots) {
        display_root(root.get(), 0, id_reg);
    }
}