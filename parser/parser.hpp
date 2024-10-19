#pragma once

#include <filesystem>
#include <map>
#include <functional>
#include <string>

#include <uva/core.hpp>

#include "source_cursor.hpp"
#include "class.hpp"
#include "method.hpp"

namespace uva
{
    namespace lang {
        struct Object : public std::enable_shared_from_this<Object>
        {
            Object(std::shared_ptr<uva::lang::Class> c) : cls(c) {};
            ~Object();
            std::shared_ptr<uva::lang::Class> cls;
            void* native = nullptr;
            std::shared_ptr<Object> base_instance = nullptr;
            Object* derived_instance = nullptr;
        };
    }
    class parser
    {
    public:
        parser() = default;
        ~parser() = default;
    public:
        std::shared_ptr<uva::lang::Class> parse(const std::filesystem::path& path);
        //void parse_to()
    };
}; // namespace uva