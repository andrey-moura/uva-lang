#pragma once

#include <string>
#include <vector>
#include <parser/parser.hpp>

namespace uva {
    namespace lang {
        class vm;
        class extension {
        public:
            extension(const std::string& name);
        public:
            static void init_all();
            const std::string& get_name() const { return m_name; }
            virtual void init() = 0;
            virtual void load_in_vm(uva::lang::vm* vm) = 0;
        private:
            std::string m_name;
        };
    }
};