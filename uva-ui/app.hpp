#pragma once

#include <string>
#include <string_view>
#include <functional>

#include <uva-ui/os_specific_data_member.hpp>
#include <uva-ui/theme.hpp>
#include <uva-ui/style.hpp>

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
                uva::lang::ui::style* m_style = nullptr;
            public:
                int run(int argc, char** argv);

                void set_theme(uva::lang::ui::theme* theme) {
                    delete m_theme;
                    m_theme = theme;
                }

                void set_style(uva::lang::ui::style* style) {
                    delete m_style;
                    m_style = style;
                }
            public:
                uva::lang::ui::theme* theme() {
                    return m_theme;
                }

                uva::lang::ui::style* style() {
                    return m_style;
                }
            public:
                virtual void on_init(int argc, char** argv) = 0;
            };
        };
    };
};