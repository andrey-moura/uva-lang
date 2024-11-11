#pragma once

#include <string>
#include <string_view>
#include <map>

#include <uva/var.hpp>

namespace uva
{
    namespace lang
    {
        namespace ui
        {
            class style
            {
            public:
                style();
            public:
                virtual var request(std::string_view what) = 0;
            };
        };
    };
}; // namespace uva