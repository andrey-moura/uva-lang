#pragma once

#include <filesystem>

namespace andy
{
    namespace lang
    {
        namespace config
        {
            std::filesystem::path src_dir();
            const std::string_view version = ANDYLANG_VERSION;
            const std::string_view build = ANDY_BUILD_TYPE;
            
            const std::string_view cpp      = ANDY_CPP_VERSION;
            const std::string_view compiler = ANDY_COMPILER;
        };
    };
}