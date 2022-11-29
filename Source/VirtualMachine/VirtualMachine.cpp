//
// Created by spaceeye on 29.11.22.
//

#include "VirtualMachine.h"

void VirtualMachine::execute(std::vector<ByteCode> &code, bool debug) {
    while (cur < code.size()-1) {
        auto word = code[cur];
        switch (word) {
            case ByteCode::PUSH: {
                cur++;
                auto amount = get_4_num(code, cur);
                cur+=4;

                stack.reserve(stack.size()+amount);
                for (int i = 0; i < amount; i++) {
                    stack.emplace_back((uint8_t)code[cur+i]);
                }
                cur += amount;
            }
                break;
            case ByteCode::POP: {
                cur++;
                auto amount = get_4_num(code, cur);
                cur+=4;
                stack.resize(stack.size()-amount);
            }
                break;
            case ByteCode::SWAP: {
                cur++;
                auto pos1 = get_4_num(code, cur);
                cur+=4;
                auto pos2 = get_4_num(code, cur);
                cur+=4;
                auto amount = get_4_num(code, cur);
                cur+=4;
                for (int i = 0; i < amount; i++) {
                    std::swap(stack[i+pos1+stack_scope.back()],
                              stack[i+pos2+stack_scope.back()]);
                }
            }
                break;
            case ByteCode::COPY_PUSH: {
                cur++;
                auto pos = get_4_num(code, cur);
                cur+=4;
                auto amount = get_4_num(code, cur);
                cur+=4;
                stack.reserve(stack.size()+amount);
                for (int i = 0; i < amount; i++) {
                    stack.emplace_back(stack[pos+i+stack_scope.back()]);
                }
            }
                break;
            case ByteCode::BUILTIN_CALL:
                cur++;
                process_builtin(code);
                break;
            case ByteCode::GOTO: {
                cur++;
                auto pos = get_4_num(code, cur);
                cur = pos;
            }
                break;
            case ByteCode::COND_GOTO:
                break;
            case ByteCode::PUSH_STACK_SCOPE:
                stack_scope.emplace_back(stack.size());
                cur++;
                break;
            case ByteCode::POP_STACK_SCOPE:
                stack_scope.pop_back();
                cur++;
                break;
        }

        if (debug) {
            std::cout << "TRANSACTION. STACK\n";
            for (auto val: stack) {
                std::cout << (int)val << " ";
            }
            std::cout << "\n";
        }
    }
}

void VirtualMachine::process_builtin(std::vector<ByteCode> &code) {
    auto id = (BuiltinIDS)get_4_num(code, cur);
    cur+=4;

    switch (id) {
        case BuiltinIDS::ASGN: {
            auto arg1_pos = get_4_num(stack, stack.size()-16);
            auto arg1_type = (VariableType)(get_4_num(stack, stack.size()-12));
            auto arg2_val = get_4_num(stack, stack.size()-8);
            auto arg2_type = (VariableType)(get_4_num(stack, stack.size()-4));

            uint32_t num = cast_to_type(arg2_val, arg2_type, arg1_type);
            *((uint32_t*)&stack[arg1_pos]) = num;
        }
            break;
        case BuiltinIDS::ADD: {
            auto arg1_val = get_4_num(stack, stack.size()-16);
            auto arg1_type = (VariableType)get_4_num(stack, stack.size()-12);
            auto arg2_val = get_4_num(stack, stack.size()-8);
            auto arg2_type = (VariableType)get_4_num(stack, stack.size()-4);

            uint32_t result;
            //cast to arg1 type
            arg2_val = cast_to_type(arg2_val, arg2_type, arg1_type);

            switch (arg1_type) {
                case VariableType::INT: {
                    int32_t temp = *(int32_t*)&arg1_val + *(int32_t*)&arg2_val;
                    result = *(uint32_t*)&temp;
                }
                    break;
                case VariableType::UINT:
                    result = arg1_val + arg2_val;
                    break;
                case VariableType::FLOAT: {
                    float temp = *(float*)&arg1_val + *(float*)&arg2_val;
                    result = *(uint32_t*)&temp;
                }
                    break;
            }

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = result;
//            *((uint32_t*)&stack[stack.size()-4]) = (uint32_t)arg1_type;
        }
            break;
        case BuiltinIDS::SUB:
            break;
        case BuiltinIDS::MUL:
            break;
        case BuiltinIDS::DIV:
            break;
        case BuiltinIDS::ASGN_ADD:
            break;
        case BuiltinIDS::SUB_ADD:
            break;
        case BuiltinIDS::MUL_ADD:
            break;
        case BuiltinIDS::DIV_ADD:
            break;
        case BuiltinIDS::AND:
            break;
        case BuiltinIDS::OR:
            break;
        case BuiltinIDS::EQUALS:
            break;
        case BuiltinIDS::NOT_EQUALS:
            break;
        case BuiltinIDS::NOT:
            break;
        case BuiltinIDS::MORE_EQUALS:
            break;
        case BuiltinIDS::LESS_EQUALS:
            break;
        case BuiltinIDS::MORE:
            break;
        case BuiltinIDS::LESS:
            break;
        case BuiltinIDS::CONCAT:
            break;
        case BuiltinIDS::TO_STR:
            break;
        case BuiltinIDS::PRINT:
            break;
        case BuiltinIDS::TO_INT:
            break;
        case BuiltinIDS::TO_UINT:
            break;
        case BuiltinIDS::TO_FLOAT:
            break;
        case BuiltinIDS::RETURN:
            break;
        case BuiltinIDS::INPUT:
            break;
        case BuiltinIDS::PRINT_NUM: {
            uint32_t res = get_4_num(stack, stack.size()-4);
            std::cout << *(int32_t*)&res << "\n";
        }
            break;
    }
}

uint32_t VirtualMachine::cast_to_type(uint32_t var, VariableType original_type, VariableType to_cast_to) {
    // *(type*)&var = treat var of whatever type as {type}
    // (type) = convert to type
    // (to_type)*(org_type*)&var = treat var of whatever type as {org_type} and cast it to {to_type}
    switch (original_type) {
        case VariableType::INT: {
            switch (to_cast_to) {
                case VariableType::INT:break;
                case VariableType::UINT: {
                    uint32_t temp = (uint32_t)*(int32_t*)&var;
                    var = *(uint32_t*)&temp;
                }
                    break;
                case VariableType::FLOAT: {
                    float temp = (float)*(int32_t*)&var;
                    var = *(uint32_t*)&temp;
                }
                    break;
            }
        }
            break;
        case VariableType::UINT: {
            switch (to_cast_to) {
                case VariableType::INT: {
                    int32_t temp = (int32_t)*(uint32_t*)&var;
                    var = *(uint32_t*)&temp;
                }
                    break;
                case VariableType::UINT: break;
                case VariableType::FLOAT: {
                    float temp = (float)*(uint32_t*)&var;
                    var = *(uint32_t*)&temp;
                }
                    break;
            }
        }
            break;
        case VariableType::FLOAT: {
            switch (to_cast_to) {
                case VariableType::INT: {
                    int32_t temp = (int32_t)*(float*)&var;
                    var = *(uint32_t*)&temp;
                }
                    break;
                case VariableType::UINT: {
                    uint32_t temp = (uint32_t)*(float*)&var;
                    var = *(uint32_t*)&temp;
                }
                    break;
                case VariableType::FLOAT: break;
            }
        }
            break;
    }
    return var;
}
