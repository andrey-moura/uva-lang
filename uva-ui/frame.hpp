#pragma once

#include <cstddef>
#include <cstdint>

#include <string>
#include <string_view>

#include <uva/var.hpp>

#include <uva-ui/os_specific_data_member.hpp>

namespace uva
{
    namespace lang
    {
        namespace ui
        {
            class frame : public os_specific_data_member
            {
            public:
                frame(std::string_view __title);
                ~frame();
            public:
                void show(bool maximized = false);
                void hide();
            public:
                //virtual var render() = 0;
            };
        };
    }
};