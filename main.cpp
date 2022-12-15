//
// Created by spaceeye on 25.10.22.
//

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>

#include "Source/Compiler/Compiler/Compiler.h"
#include "Source/VirtualMachine/VirtualMachine.h"
#include "Source/Compiler/OptionsContainer.h"

std::vector<std::string> open_files(const std::vector<std::string>& file_paths);
static std::vector<std::string> read_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    //TODO
    Options options{};
    auto arguments = read_arguments(argc, argv);

//    options.optimization.trim_unreachable = false;
//    options.optimization.batch_push_commands = false;

    options.debug.show_compiler_output = true;

    auto data = open_files(std::vector<std::string>({"/home/spaceeye/CLionProjects/AnotherProgrammingLanguage/test.pspl"}));

    if (data.empty()) {
        std::cout << "No files detected. Aborting.";
        return 0;
    }

    std::vector<ByteCode> code;
//    try {
        code = Compiler::compile(data, options);
//    } catch (const std::exception & e) {
//        std::cout << "Compiler has failed to compile the code. The exception:\n" << e.what();
//        return 0;
//    }

    VirtualMachine vm{};

    auto time1 = std::chrono::high_resolution_clock::now();
    vm.execute(code, false);
    auto time2 = std::chrono::high_resolution_clock::now();

    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1).count();

    return 0;
}

std::vector<std::string> open_files(const std::vector<std::string>& file_paths) {
    std::vector<std::string> to_return;

    for (auto & path_name: file_paths) {
        std::filesystem::path path{path_name};
        if (is_directory(path) || !exists(path)) {
            std::cout << path << " is not included because it is either a directory or doesn't exist.\n";
            continue;
        }
        std::ifstream t(path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        to_return.emplace_back(buffer.str());
    }

    return to_return;
}

static std::vector<std::string> read_arguments(int argc, char *argv[]) {
    return std::vector<std::string>{};
}