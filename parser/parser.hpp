#pragma once

#include <filesystem>
#include <map>
#include <functional>
#include <string>

#include <uva/core.hpp>

#include <lexer/cursor.hpp>
#include <vm/vm.hpp>

#include "class.hpp"
#include "method.hpp"
#include "object.hpp"

namespace uva
{
    class parser
    {
    public:
        parser() = default;
        ~parser() = default;
    public:
        std::shared_ptr<uva::lang::structure> parse(const std::filesystem::path& path, std::shared_ptr<uva::lang::vm> vm_instance);
    };
}; // namespace uva