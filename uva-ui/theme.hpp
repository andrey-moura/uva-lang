#pragma once

#include <string>
#include <map>

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
                virtual std::map<std::string, std::string> window() = 0;
            };
        };
    };
}; // namespace uva