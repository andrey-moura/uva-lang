#pragma once

#include <string>
#include <vector>
#include <parser/parser.hpp>

namespace uva {
    namespace lang {
        class interpreter;
        class extension {
        public:
            extension(const std::string& name);
        public:
            static void init_all();
            static void import(uva::lang::interpreter* interpreter, std::string_view module);
        public:
            const std::string& name() const { return m_name; }
        public:
            /// @brief Load the extension in the interpreter. Good for loading your custom classes. Called BEFORE the source code is executed.
            virtual void load(uva::lang::interpreter* interpreter) = 0;
            /// @brief The init method of the extension. Good for when you need to call code defined in uva. Called AFTER the interpreter is initialized and the source code is executed.
            virtual void init() = 0;
        private:
            std::string m_name;
        };
    }
};