#include "layout.hpp"

uva::enumeration layout_element_style::layout_type_enumeration = uva::enumeration("type",
    {
        "flexible",
    }
);

uva::enumeration layout_element_style::layout_flex_direction_enumeration = uva::enumeration("direction",
    {
        "vertical",
        "horizontal",
    }
);