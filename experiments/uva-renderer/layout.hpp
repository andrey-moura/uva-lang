#pragma once

#include <array>
#include <vector>
#include <map>

#include <uva/color.hpp>
#include <uva/xml.hpp>

struct layout_element_style {
    enum layout_type : uint8_t {
        flexible,
        layout_type_max
    };

    static uva::enumeration layout_type_enumeration;

    enum layout_flex_direction : uint8_t {
        vertical,
        horizontal,
        layout_flex_direction_max
    };

    static uva::enumeration layout_flex_direction_enumeration;

    layout_type           type      = layout_type::flexible;
    layout_flex_direction direction = layout_flex_direction::vertical;
};

struct border {
    int width = 0;
    uva::color color = uva::color(0, 0, 0, 0);
    int radius = 0;
};

enum view_element_cursor : uint8_t {
    view_element_cursor_default,
    view_element_cursor_pointer,
    view_element_cursor_max
};

struct view_element_style {
    uint8_t             flex = 0;
    view_element_cursor cursor = view_element_cursor::view_element_cursor_default;

    int        padding = 0;
    int        gap     = 0;
    uva::color color = uva::color(0, 0, 0, 255);
    uva::color background_color = uva::color(0, 0, 0, 0);
    int border_radius = 0;
};

struct view_element {
    int x;
    int y;

    int w;
    int h;

    view_element_style style;

    virtual void draw();
};

struct layout : view_element {
    layout_element_style layout_style;

    std::vector<std::shared_ptr<view_element>> childreans;

    void calculate_layout(int __x, int __y, int __w, int __h)
    {
        x = __x;
        y = __y;
        w = __w;
        h = __h;

        int current_x = x + style.padding;
        int current_y = y + style.padding;

        int number_of_spacer_elements = 0;
        int available_space = 0;

        if(layout_style.direction == layout_element_style::layout_flex_direction::vertical) {
            available_space = __h - (style.padding * 2) - style.gap * childreans.size();
        } else {
            available_space = __w - (style.padding * 2) - style.gap * childreans.size();
        }

        for(auto& child : childreans) {
            if(child->style.flex > 0) {
                number_of_spacer_elements += child->style.flex;
            } else {
                if(layout_style.direction == layout_element_style::layout_flex_direction::vertical) {
                    available_space -= child->h;
                } else {
                    available_space -= child->w;
                }
            }
        }

        if(number_of_spacer_elements > 0) {
            int available_space_per_spacer = available_space / number_of_spacer_elements;

            for(auto& child : childreans) {
                if(child->style.flex > 0) {
                    int child_space = available_space_per_spacer * child->style.flex;

                    if(layout_style.direction == layout_element_style::layout_flex_direction::vertical) {
                        child->h = child_space;
                    } else {
                        child->w = child_space;
                    }
                }
            }
        }

        for(auto& child : childreans) {
            child->x = current_x;
            child->y = current_y;

            if(layout_style.direction == layout_element_style::layout_flex_direction::vertical) {
                child->w = __w - (style.padding * 2);
                current_y += child->h;
                current_y += style.gap;
            } else {
                child->h = __h - (style.padding * 2);
                current_x += child->w;
                current_x += style.gap;
            }

            layout* le = dynamic_cast<layout*>(child.get());

            if(le) {
                le->calculate_layout(child->x, child->y, child->w, child->h);
            }
        }
    }

    void draw() override;
};

enum text_vertical_alignment : uint8_t {
    text_vertical_alignment_top,
    text_vertical_alignment_center,
    text_vertical_alignment_bottom,
    text_vertical_alignment_max
};

enum text_horizontal_alignment : uint8_t {
    text_horizontal_alignment_left,
    text_horizontal_alignment_center,
    text_horizontal_alignment_right,
    text_horizontal_alignment_max
};

struct text_element_style {
    text_vertical_alignment vertical_alignment;
    text_horizontal_alignment horizontal_alignment;
};

struct text_element : public view_element {
    std::string content;

    text_element_style text_style;

    void draw() override;
};