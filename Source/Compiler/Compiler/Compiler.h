//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H
#define ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H

#include <vector>
#include <string>

#include "ByteCode.h"
#include "../Transpiler/Transpiler.h"
#include "../Parser/Parser.h"

class Compiler {
public:
    static std::vector<ByteCode> compile(const std::vector<std::string> &str_data, bool debug);
};


#endif //ANOTHERPROGRAMMINGLANGUAGE_COMPILER_H
