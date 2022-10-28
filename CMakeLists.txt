cmake_minimum_required(VERSION 3.23)
project(AnotherProgrammingLanguage)

set(CMAKE_CXX_STANDARD 20)

add_executable(AnotherProgrammingLanguage
        main.cpp Source/Compiler/Compiler/Compiler.cpp Source/Compiler/Compiler/Compiler.h Source/Compiler/Transpiler/Transpiler.h Source/Compiler/Compiler/ByteCode.h Source/Compiler/Transpiler/Tokens.h Source/Compiler/Transpiler/Transpiler.cpp Source/Compiler/Transpiler/TokenHelperFunctions.cpp Source/Compiler/Parser/Parser.cpp Source/Compiler/Parser/Parser.h)
