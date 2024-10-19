#pragma once

#include <filesystem>
#include <map>
#include <functional>
#include <string>

#include <uva/core.hpp>

#include <lexer/cursor.hpp>
#include "class.hpp"
#include "method.hpp"

namespace uva
{
    namespace lang {
        struct object
        {
            object(std::shared_ptr<uva::lang::structure> c) : cls(c) {};
            ~object();
            std::shared_ptr<uva::lang::structure> cls;
            void* native = nullptr;
            std::shared_ptr<object> base_instance = nullptr;
            object* derived_instance = nullptr;
        };
    }
    class parser
    {
    public:
        parser() = default;
        ~parser() = default;
    public:
        std::shared_ptr<uva::lang::structure> parse(const std::filesystem::path& path);
        //void parse_to()
    };
}; // namespace uva