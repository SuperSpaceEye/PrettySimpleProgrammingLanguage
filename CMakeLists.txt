cmake_minimum_required(VERSION 3.23)
project(PrettySimpleProgrammingLanguage)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_LINK_DEF_FILE_FLAG "${CMAKE_LINK_DEF_FILE_FLAG} -fsanitize=address")

add_executable(PrettySimpleProgrammingLanguage main.cpp Source/Compiler/Compiler/Compiler.cpp Source/Compiler/Compiler/Compiler.h Source/Compiler/Transpiler/Transpiler.h Source/Compiler/Compiler/ByteCode.h Source/Compiler/Transpiler/Tokens.h Source/Compiler/Transpiler/Transpiler.cpp Source/Compiler/Transpiler/TokenHelperFunctions.cpp Source/Compiler/Parser/ActionTreeCreator.h Source/Compiler/Parser/Implementations/TreeCreator.cpp Source/Compiler/Parser/Action/Actions.h Source/Compiler/Parser/Action/OperationType.h Source/Compiler/Parser/VariableType.h Source/Compiler/Parser/BuiltinFunctions.h Source/Compiler/Parser/Implementations/ParserDisplayFunc.cpp Source/Compiler/Parser/Implementations/Parser_extract_nested_functions.cpp Source/VirtualMachine/VirtualMachine.cpp Source/VirtualMachine/VirtualMachine.h Source/Compiler/OptionsContainer.h Source/Compiler/Parser/TreeValidator.h Source/Compiler/Parser/Implementations/TreeValidator.cpp Source/Compiler/Parser/TreeOptimizer.h Source/Compiler/Parser/Implementations/TreeOptimizer.cpp Source/Compiler/Parser/CommonFNS.h Source/Compiler/Parser/Implementations/IdRegister.cpp)
