#pragma once

#include <string>
#include <vector>
#include <parser/parser.hpp>

#ifdef _WIN32
    #define UVA_EXTENSION(name) \
    extern "C" __declspec(dllexport) uva::lang::extension* create_extension()\
    {\
        return new name ();\
    }
#endif

namespace uva {
    namespace lang {
        class interpreter;
        class extension {
        public:
            extension(const std::string& name);
        public:
            static void import(uva::lang::interpreter* interpreter, std::string_view module);
        public:
            const std::string& name() const { return m_name; }
        public:
            /// @brief Load the extension in the interpreter. Good for loading your custom classes. Called BEFORE the source code is executed.
            virtual void load(uva::lang::interpreter* interpreter) { }
            virtual void start(uva::lang::interpreter* interpreter) { }
        private:
            std::string m_name;
        };
    }
};