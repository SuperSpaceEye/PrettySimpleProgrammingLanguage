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
//    int counter = 0;
    while (cur < code.size()) {
        auto word = code[cur];
//        counter++;
//        if (counter == 389) {
//            volatile int a = 10;
//        }
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
                    std::swap(stack[stack.size()-pos1+i],
                              stack[stack.size()-pos2+i]);
                }
            }
                break;
            case ByteCode::COPY_PUSH: {
                cur++;
                auto pos = get_4_num(code, cur);
                cur+=4;
                auto amount = get_4_num(code, cur);
                cur+=4;
                auto size = stack.size();

                stack.resize(stack.size()+amount);
                for (int i = 0; i < amount; i++) {
                    stack[size+i] = stack[size-pos+i];
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
            case ByteCode::COND_GOTO: {
                cur++;
                auto val = get_4_num(stack, stack.size()-4);
                stack.resize(stack.size()-4);

                if (!val) {
                    auto pos = get_4_num(code, cur);
                    cur = pos;
                    continue;
                }
                cur+=4;
            }
                break;
            case ByteCode::REL_GOTO: {
                cur++;
                auto pos = get_4_num(stack, stack.size()-4);
                stack.resize(stack.size()-4);
                cur = pos;
            }
                break;
            case ByteCode::GET_ABSOLUTE_POS: {
                cur++;
                auto rel_var_pos = get_4_num(code, cur);
                cur+=4;

                auto abs_pos = stack.size() - rel_var_pos;

                stack.resize(stack.size()+4);

                *((uint32_t*)&stack[stack.size()-4]) = abs_pos;
            }
                break;
            case ByteCode::NOOP: cur++; break;
            default:
                throw std::logic_error("VM met an unknown word. This is probably a bug in the compiler. If you see this, open an issue on the github or message me, SpaceEye.");
        }

        //DO NOT DELETE. MAKES VM SLOWER FOR SOME REASON. LAST CHECKED 03.12.2022
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
            auto arg_type = (VariableType)get_4_num(stack, stack.size()-8);
            auto arg_val = get_4_num(stack, stack.size()-4);

            arg_val = cast_to_type(arg_val, arg_type, VariableType::INT);

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = arg_val;
        }
            break;
        case BuiltinIDS::TO_UINT: {
            auto arg_type = (VariableType)get_4_num(stack, stack.size()-8);
            auto arg_val = get_4_num(stack, stack.size()-4);

            arg_val = cast_to_type(arg_val, arg_type, VariableType::UINT);

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = arg_val;
        }
            break;
        case BuiltinIDS::TO_FLOAT: {
            auto arg_type = (VariableType)get_4_num(stack, stack.size()-8);
            auto arg_val = get_4_num(stack, stack.size()-4);

            arg_val = cast_to_type(arg_val, arg_type, VariableType::FLOAT);

            stack.resize(stack.size()+4);
            *((uint32_t*)&stack[stack.size()-4]) = arg_val;
        }
            break;
        case BuiltinIDS::INPUT:
            break;
        case BuiltinIDS::PRINT_NUM: {
            auto type = (VariableType)get_4_num(stack, stack.size()-8);
            uint32_t num = get_4_num(stack, stack.size()-4);

            switch (type) {
                case VariableType::INT:
                    std::cout << *(int32_t*)&num << "\n";
                    break;
                case VariableType::UINT:
                    std::cout << *(uint32_t *)&num << "\n";
                    break;
                case VariableType::FLOAT:
                    std::cout << *(float*)&num << "\n";
                    break;
            }
        }
            break;
        default:
            throw std::logic_error("VM has met an unknown builtin fn id. If you see this, open an issue on the github or message me, SpaceEye.");
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
