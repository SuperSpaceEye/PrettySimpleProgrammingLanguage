//
// Created by spaceeye on 29.11.22.
//

#ifndef PRETTYSIMPLEPROGRAMMINGLANGUAGE_VIRTUALMACHINE_H
#define PRETTYSIMPLEPROGRAMMINGLANGUAGE_VIRTUALMACHINE_H

#include <vector>
#include <iostream>

#include "../Compiler/Compiler/ByteCode.h"
#include "../Compiler/Parser/BuiltinFunctions.h"
#include "../Compiler/Parser/VariableType.h"

class VirtualMachine {
    std::vector<uint8_t> stack{};
    std::vector<int> stack_scope{0};
    std::vector<int> temp_levels = {};

    uint32_t cur = 0;

    template<class T>
    static uint32_t get_4_num(std::vector<T> & code, int i) {
        return *((uint32_t*)&code[i]);
    };

    void process_builtin(std::vector<ByteCode> & code);

    uint32_t cast_to_type(uint32_t var, VariableType original_type, VariableType to_cast_to);
public:
    void execute(std::vector<ByteCode> &code, bool debug);
};


#endif //PRETTYSIMPLEPROGRAMMINGLANGUAGE_VIRTUALMACHINE_H
