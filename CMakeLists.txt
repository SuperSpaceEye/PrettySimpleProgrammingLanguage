cmake_minimum_required(VERSION 3.23)
project(AnotherProgrammingLanguage)

set(CMAKE_CXX_STANDARD 20)

add_executable(AnotherProgrammingLanguage
        main.cpp Source/Compiler/Compiler.cpp Source/Compiler/Compiler.h Source/Compiler/Transpiler/Transpiler.h Source/Compiler/ByteCode.h Source/Compiler/Transpiler/Tokens.h Source/Compiler/Transpiler/Transpiler.cpp)
