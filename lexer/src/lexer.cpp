#include <lexer.hpp>

std::vector<std::pair<std::string_view, uva::lang::lexer::cursor_type>> uva::lang::lexer::cursor_type_from_string_map = {
    /* Keep in most used order */

    { "//",       cursor_type::cursor_comment          },
    { "{",        cursor_type::cursor_block            },
    { "function", cursor_type::cursor_function         },
    { "return",   cursor_type::cursor_return           },
    { "class",    cursor_type::cursor_class            },
    { "extends",  cursor_type::cursor_baseclass        },
};