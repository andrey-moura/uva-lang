#pragma once

#include <string>
#include <string_view>
#include <functional>

#include <uva-ui/os_specific_data_member.hpp>
#include <uva-ui/theme.hpp>

namespace uva
{
    namespace lang
    {
        namespace ui
        {
            class app : public os_specific_data_member
            {
            public:
                app(std::string_view __name, std::string_view vendor);
                ~app();
            protected:
                uva::lang::ui::theme* m_theme = nullptr;
            public:
                int run();

                void set_theme(uva::lang::ui::theme* theme) {
                    delete m_theme;
                    m_theme = theme;
                }
            public:
                uva::lang::ui::theme* theme() {
                    return m_theme;
                }
            public:
                virtual void on_init() = 0;
            };
        };
    };
};