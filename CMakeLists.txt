cmake_minimum_required(VERSION 3.23)
project(PrettySimpleProgrammingLanguage)

set(CMAKE_CXX_STANDARD 20)

add_executable(PrettySimpleProgrammingLanguage
        main.cpp Source/Compiler/Compiler/Compiler.cpp Source/Compiler/Compiler/Compiler.h Source/Compiler/Transpiler/Transpiler.h Source/Compiler/Compiler/ByteCode.h Source/Compiler/Transpiler/Tokens.h Source/Compiler/Transpiler/Transpiler.cpp Source/Compiler/Transpiler/TokenHelperFunctions.cpp Source/Compiler/Parser/ParserStuff/Parser_create_ast.cpp Source/Compiler/Parser/Parser.h Source/Compiler/Parser/Action/Actions.h Source/Compiler/Parser/Action/OperationType.h Source/Compiler/Parser/VariableType.h Source/Compiler/Parser/BuiltinFunctions.h Source/Compiler/Parser/ParserStuff/ParserDisplayFunc.cpp Source/Compiler/Parser/ParserStuff/IdRegFuncs.cpp Source/Compiler/Parser/ParserStuff/Parser_validate_ast.cpp Source/Compiler/Parser/ParserStuff/Parser_extract_nested_functions.cpp)
