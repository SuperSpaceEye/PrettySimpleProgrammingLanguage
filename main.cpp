//
// Created by spaceeye on 25.10.22.
//

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "Source/Compiler/Compiler/Compiler.h"

std::vector<std::string> open_files(const std::vector<std::string>& file_paths);
static std::vector<std::string> read_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    //TODO
    auto arguments = read_arguments(argc, argv);

    auto data = open_files(std::vector<std::string>({"/home/spaceeye/CLionProjects/AnotherProgrammingLanguage/test.pspl"}));

    if (data.empty()) {
        std::cout << "No files detected. Aborting.";
        return 0;
    }

    auto code = Compiler::compile(data, true);

    return 0;
}

std::vector<std::string> open_files(const std::vector<std::string>& file_paths) {
    std::vector<std::string> to_return;

    for (auto & path_name: file_paths) {
        std::filesystem::path path{path_name};
        if (is_directory(path) || !exists(path)) {
            std::cout << path << " is not included because it is either a directory or doesn't exist.";
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