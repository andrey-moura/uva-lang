#include <andy/lang/extension.hpp>

#include <iostream>

#include <uva.hpp>

#include <andy/lang/interpreter.hpp>

#ifdef __linux__
#   include <dlfcn.h>
#elif defined(_WIN32)
#   include <Windows.h>
#endif

#include <uva/file.hpp>

//std::vector<andy::lang::extension*> extensions;

andy::lang::extension::extension(const std::string &name)
    : m_name(name)
{
    //extensions.push_back(this);
}

void andy::lang::extension::import(andy::lang::interpreter* interpreter, std::string_view module)
{
    std::filesystem::path executable_path = uva::file::executable_path();
    std::filesystem::path file_path = executable_path;

    std::string library_name = std::string(module) + "-shared";

#if defined(__linux__)
    library_name = "libandylang-" + library_name;
#elif defined(_WIN32)
    library_name = "andylang-" + library_name;
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

    andy::lang::extension* (*create_extension)() = (andy::lang::extension*(*)())dlsym(handle, "create_extension");

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

    andy::lang::extension* (*create_extension)() = (andy::lang::extension*(*)())GetProcAddress(handle, "create_extension");

    if(!create_extension) {
        throw std::runtime_error("Failed to get create_extension");
    }
#else
    throw std::runtime_error("unsupported platform");
#endif

    andy::lang::extension* extension = create_extension();

    interpreter->load_extension(extension);
}