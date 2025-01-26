#pragma once

#include <uva/lang/lexer.hpp>

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
            preprocessor();
            ~preprocessor();
        public:
            void process(const std::filesystem::path& __file_name, uva::lang::lexer& __lexer);
        public:
            void process_include(const std::filesystem::path& __file_name, uva::lang::lexer& __lexer);
        };
    };
};