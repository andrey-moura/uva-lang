// main.cpp - Entry Point
#ifndef _WIN32  // Check if not Windows
#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <dlfcn.h>

#include <interpreter/interpreter.hpp>

extern int uva_main(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    return uva_main(argc, argv);
}

#endif