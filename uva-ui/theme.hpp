#pragma once

#include <string>
#include <map>

#include <uva/var.hpp>

namespace uva
{
    namespace lang
    {
        namespace ui
        {
            class theme
            {
            public:
                theme();
            public:
                virtual var request(std::string_view what) = 0;
            };
        };
    };
}; // namespace uva