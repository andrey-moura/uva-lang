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
            const std::string& get_name() const { return m_name; }
            virtual void init() = 0;
            virtual void load_in_interpreter(uva::lang::interpreter* interpreter) = 0;
        private:
            std::string m_name;
        };
    }
};