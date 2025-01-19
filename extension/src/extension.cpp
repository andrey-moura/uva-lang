#include "extension/extension.hpp"

#include <iostream>

#include <uva.hpp>

#include <interpreter/interpreter.hpp>

#ifdef __linux__
#   include <dlfcn.h>
#endif

#include <uva/file.hpp>

//std::vector<uva::lang::extension*> extensions;

uva::lang::extension::extension(const std::string &name)
    : m_name(name)
{
    //extensions.push_back(this);
}

void uva::lang::extension::import(uva::lang::interpreter* interpreter, std::string_view module)
{
    std::filesystem::path executable_path = uva::file::executable_path();
    std::filesystem::path file_path = executable_path;

    std::string library_name = std::string(module) + "-shared";

#if defined(__linux__)
    library_name = "libuvalang-" + library_name;
#elif defined(_WIN32)
    library_name = "uvalang-" + library_name;
#else
    throw std::runtime_error("unsupported platform");
#endif
    executable_path.replace_filename(library_name);
#if defined(__linux__)
    executable_path.replace_extension(".so");
#elif defined(_WIN32)
    executable_path.replace_extension(".dll");
#else
    throw std::runtime_error("unsupported platform");
#endif

    if(!std::filesystem::exists(executable_path)) {
        throw std::runtime_error("module " + std::string(module) + " not found. Expect it to be at " + executable_path.string());
    }

#ifdef __linux__
    void* handle = dlopen(executable_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);

    if(!handle) {
        throw std::runtime_error(dlerror());
    }

    uva::lang::extension* (*create_extension)() = (uva::lang::extension*(*)())dlsym(handle, "create_extension");

    if(!create_extension) {
        throw std::runtime_error(dlerror());
    }
#elif defined(_WIN32)
    std::string executable_path_str = executable_path.string();
    const char* executable_path_c_str = executable_path_str.c_str();
    HMODULE handle = LoadLibrary(executable_path_c_str);

    if(!handle) {
        throw std::runtime_error("Failed to load library");
    }

    uva::lang::extension* (*create_extension)() = (uva::lang::extension*(*)())GetProcAddress(handle, "create_extension");

    if(!create_extension) {
        throw std::runtime_error("Failed to get create_extension");
    }
#else
    throw std::runtime_error("unsupported platform");
#endif

    uva::lang::extension* extension = create_extension();

    interpreter->load_extension(extension);
}