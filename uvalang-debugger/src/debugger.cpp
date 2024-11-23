#include <iostream>
#include <filesystem>
#include <chrono>

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "uvalang-debugger <input-file>" << std::endl;
        return 1;
    }

    // launch uva with the input file

    std::filesystem::path uva_executable_path = argv[0];
}