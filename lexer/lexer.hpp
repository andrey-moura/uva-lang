#include <vector>
#include <memory>
#include <map>
#include <utility>

#include "cursor.hpp"

namespace uva
{
    namespace lang
    {
        namespace lexer
        {
            extern std::vector<std::pair<std::string_view, cursor_type>> cursor_type_from_string_map;
        };
    };
}; // namespace uva