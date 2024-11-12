#pragma once

#include <lexer/lexer.hpp>

#include <string>
#include <vector>
#include <string_view>
#include <functional>
#include <filesystem>
#include <regex>

namespace uva
{
    namespace lang
    {
        class preprocessor
        {
        public:
            preprocessor(std::filesystem::path __executable_path);
            ~preprocessor();
        protected:
            std::filesystem::path m_executable_path;

            std::string m_vm;
        public:
            void process(const std::filesystem::path& __file_name, uva::lang::lexer& __lexer);
        public:
            void process_include(const std::filesystem::path& __file_name, uva::lang::lexer& __lexer);
            void process_vm(const std::filesystem::path& __file_name, uva::lang::lexer& __lexer);

            std::string_view specified_vm() const { return m_vm; }
            bool has_specified_vm() const { return m_vm.size(); }
            int  launch_vm(int argc, char** argv);
        };
    };
};