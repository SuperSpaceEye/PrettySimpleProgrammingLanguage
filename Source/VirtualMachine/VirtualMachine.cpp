//
// Created by spaceeye on 29.11.22.
//

#include "VirtualMachine.h"

#define NO_REF_OPERATION(operation) \
auto arg1_type = (VariableType)get_4_num(stack, stack.size()-16); \
auto arg1_val  = get_4_num(stack, stack.size()-12);\
auto arg2_type = (VariableType)get_4_num(stack, stack.size()-8); \
auto arg2_val  = get_4_num(stack, stack.size()-4);\
\
uint32_t result;\
arg2_val = cast_to_type(arg2_val, arg2_type, arg1_type);\
                                    \
switch (arg1_type) {\
    case VariableType::INT: {\
        int32_t temp = *(int32_t*)&arg1_val operation *(int32_t*)&arg2_val;\
        result = *(uint32_t*)&temp;\
    }\
    break;\
    case VariableType::UINT:\
        result = arg1_val operation arg2_val;\
        break;\
    case VariableType::FLOAT: {\
        float temp = *(float*)&arg1_val operation *(float*)&arg2_val;\
        result = *(uint32_t*)&temp;\
    }\
        break;\
    }\
    \
    stack.resize(stack.size()+4);\
    *((uint32_t*)&stack[stack.size()-4]) = result; \

#define NO_REF_LOGIC_OPERATION(operation) \
auto arg1_type = (VariableType)get_4_num(stack, stack.size()-16); \
auto arg1_val  = get_4_num(stack, stack.size()-12);\
auto arg2_type = (VariableType)get_4_num(stack, stack.size()-8); \
auto arg2_val  = get_4_num(stack, stack.size()-4);\
\
arg2_val = cast_to_type(arg2_val, arg2_type, arg1_type);\
switch (arg1_type) {\
    case VariableType::INT: {\
        arg1_val = *(int32_t*)&arg1_val operation *(int32_t*)&arg2_val;\
    }\
    break;\
    case VariableType::UINT:\
        arg1_val = arg1_val operation arg2_val;\
        break;\
    case VariableType::FLOAT: {\
        arg1_val = *(float*)&arg1_val operation *(float*)&arg2_val;\
    }\
        break;\
    }\
    \
    stack.resize(stack.size()+4);\
    *((uint32_t*)&stack[stack.size()-4]) = arg1_val;\

#define REF_OPERATION(operation) \
auto arg1_type = (VariableType)get_4_num(stack, stack.size()-16);\
auto arg1_pos = get_4_num(stack, stack.size()-12); \
auto arg2_type = (VariableType)get_4_num(stack, stack.size()-8);\
auto arg2_val  = get_4_num(stack, stack.size()-4);               \
                                 \
auto arg1_val = get_4_num(stack, arg1_pos);\
\
uint32_t result;\
arg2_val = cast_to_type(arg2_val, arg2_type, arg1_type);\
\
switch (arg1_type) {\
    case VariableType::INT: {\
        int32_t temp = *(int32_t*)&arg1_val operation *(int32_t*)&arg2_val;\
        result = *(uint32_t*)&temp;\
    }\
    break;\
    case VariableType::UINT:\
        result = arg1_val operation arg2_val;\
        break;\
    case VariableType::FLOAT: {\
        float temp = *(float*)&arg1_val operation *(float*)&arg2_val;\
        result = *(uint32_t*)&temp;\
    }\
    break;\
}\
\
*((uint32_t*)&stack[arg1_pos]) = result;\


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
                auto temp = get_4_num(code, cur);
                auto pos = (int32_t)*(uint32_t*)&temp;
//                auto pos = get_4_num(code, cur);
                cur+=4;
                auto amount = get_4_num(code, cur);
                cur+=4;
                stack.reserve(stack.size()+amount);

                if (pos >= 0) {
                    for (int i = 0; i < amount; i++) {
                        stack.emplace_back(stack[pos + i + stack_scope.back()]);
                    }
                } else {
                    for (int i = 0; i < amount; i++) {
                        stack.emplace_back(stack[pos + i + temp_levels.back()]);
                    }
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
            case ByteCode::REL_GOTO: {
                cur++;
                auto pos = get_4_num(stack, stack.size()-4);
                stack.resize(stack.size()-4);
                cur = pos;
            }
                break;
            case ByteCode::PUSH_STACK_SCOPE:
                stack_scope.emplace_back(stack.size());
                cur++;
                break;
            case ByteCode::POP_STACK_SCOPE:
                stack_scope.pop_back();
                cur++;
                break;
            case ByteCode::PUSH_CURRENT_STACK_LEVEL:
                stack.resize(stack.size()+4);
                *((uint32_t*)&stack[stack.size()-4]) = stack_scope.back();
                cur++;
                break;
            case ByteCode::GET_ABSOLUTE_POS: {
                //position can be negative if var is below current scope but
                // is in function frame.
                auto temp = get_4_num(stack, stack.size()-8);
                auto rel_var_pos = (int32_t)*(uint32_t*)&temp;
                auto var_level = get_4_num(stack, stack.size()-4);

                auto abs_pos = var_level + rel_var_pos;

                stack.resize(stack.size()-4);

                *((uint32_t*)&stack[stack.size()-4]) = abs_pos;
                cur++;
            }
                break;
            case ByteCode::START_ARGUMENTS:
                temp_levels.emplace_back(stack.size());
                cur++;
                break;
            case ByteCode::END_ARGUMENTS:
                stack_scope.emplace_back(temp_levels.back());
                temp_levels.pop_back();
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
            auto arg1_type = (VariableType)get_4_num(stack, stack.size()-16);
            auto arg1_pos = get_4_num(stack, stack.size()-12);
            auto arg2_type = (VariableType)get_4_num(stack, stack.size()-8);
            auto arg2_val = get_4_num(stack, stack.size()-4);

            uint32_t num = cast_to_type(arg2_val, arg2_type, arg1_type);
            *((uint32_t*)&stack[arg1_pos]) = num;
        }
            break;
        case BuiltinIDS::ASGN_ADD: {REF_OPERATION(+)}break;
        case BuiltinIDS::ASGN_SUB: {REF_OPERATION(-)}break;
        case BuiltinIDS::ASGN_MUL: {REF_OPERATION(*)}break;
        case BuiltinIDS::ASGN_DIV: {REF_OPERATION(/)}break;

        case BuiltinIDS::ADD: {NO_REF_OPERATION(+)}break;
        case BuiltinIDS::SUB: {NO_REF_OPERATION(-)}break;
        case BuiltinIDS::MUL: {NO_REF_OPERATION(*)}break;
        case BuiltinIDS::DIV: {NO_REF_OPERATION(/)}break;

        case BuiltinIDS::AND:         {NO_REF_LOGIC_OPERATION(&&)}break;
        case BuiltinIDS::OR:          {NO_REF_LOGIC_OPERATION(||)}break;
        case BuiltinIDS::EQUALS:      {NO_REF_LOGIC_OPERATION(==)}break;
        case BuiltinIDS::NOT_EQUALS:  {NO_REF_LOGIC_OPERATION(!=)}break;
        case BuiltinIDS::MORE_EQUALS: {NO_REF_LOGIC_OPERATION(>=)}break;
        case BuiltinIDS::LESS_EQUALS: {NO_REF_LOGIC_OPERATION(<=)}break;
        case BuiltinIDS::MORE:        {NO_REF_LOGIC_OPERATION(>)}break;
        case BuiltinIDS::LESS:        {NO_REF_LOGIC_OPERATION(<)}break;

        case BuiltinIDS::NOT: {
            auto arg_val = get_4_num(stack, stack.size()-8);
            auto arg_type = (VariableType)(get_4_num(stack, stack.size()-4));

            switch (arg_type) {
                case VariableType::INT: {
                    arg_val = !*(int32_t*)&arg_val;
                }
                break;
                case VariableType::UINT:
                    arg_val = !arg_val;
                    break;
                case VariableType::FLOAT: {
                    arg_val = !*(float*)&arg_val;
                }
                break;
            }

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = arg_val;
        }break;

        case BuiltinIDS::CONCAT:
            break;
        case BuiltinIDS::TO_STR:
            break;
        case BuiltinIDS::PRINT:
            break;
        case BuiltinIDS::TO_INT: {
            auto arg_val = get_4_num(stack, stack.size()-8);
            auto arg_type = (VariableType)get_4_num(stack, stack.size()-4);

            arg_val = cast_to_type(arg_val, arg_type, VariableType::INT);

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = arg_val;
        }
            break;
        case BuiltinIDS::TO_UINT: {
            auto arg_val = get_4_num(stack, stack.size()-8);
            auto arg_type = (VariableType)get_4_num(stack, stack.size()-4);

            arg_val = cast_to_type(arg_val, arg_type, VariableType::UINT);

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = arg_val;
        }
            break;
        case BuiltinIDS::TO_FLOAT: {
            auto arg_val = get_4_num(stack, stack.size()-8);
            auto arg_type = (VariableType)get_4_num(stack, stack.size()-4);

            arg_val = cast_to_type(arg_val, arg_type, VariableType::FLOAT);

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = arg_val;
        }
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
