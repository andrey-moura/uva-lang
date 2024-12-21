#include "extension/extension.hpp"

#include <iostream>

#include <uva.hpp>

#include <interpreter/interpreter.hpp>

#ifdef __linux__
#   include <dlfcn.h>
#endif

//std::vector<uva::lang::extension*> extensions;

uva::lang::extension::extension(const std::string &name)
    : m_name(name)
{
    //extensions.push_back(this);
}

void uva::lang::extension::import(uva::lang::interpreter* interpreter, std::string_view module)
{
    std::filesystem::path file_path = std::filesystem::current_path();

    std::string library_name = std::string(module) + "-shared";

#ifdef __UVA_DEBUG__
    file_path /= "build";
#endif

#if defined(__linux__)
    library_name = "lib" + library_name;
#endif

    file_path /= library_name;

#if defined(__linux__)
    file_path.replace_extension(".so");
#else
    throw std::runtime_error("unsupported platform");
#endif

    if(!std::filesystem::exists(file_path)) {
        throw std::runtime_error("module " + std::string(module) + " not found. Expect it to be at " + file_path.string());
    }

#ifdef __linux__
    void* handle = dlopen(file_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);

    if(!handle) {
        throw std::runtime_error(dlerror());
    }

    uva::lang::extension* (*create_extension)() = (uva::lang::extension*(*)())dlsym(handle, "create_extension");

    if(!create_extension) {
        throw std::runtime_error(dlerror());
    }

    uva::lang::extension* extension = create_extension();

    interpreter->load_extension(extension);
#else
    throw std::runtime_error("unsupported platform");
#endif
}