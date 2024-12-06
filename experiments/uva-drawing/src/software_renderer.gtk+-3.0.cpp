#include <gtk/gtk.h>

#include "gtk3+-3.0.hpp"

#include <uva/drawing.hpp>

#define m_cairo os_specific_data_as<software_renderer_data>().cairo

uva::drawing::software_renderer::software_renderer(surface& __surface)
    : basic_renderer(__surface)
{
    // Create the software_renderer
    software_renderer_data data;
    data.cairo = cairo_create(__surface.os_specific_data_as<surface_data>().cairo_surface);

    os_specific_data_as<software_renderer_data>() = data;
}

uva::drawing::software_renderer::~software_renderer()
{
    cairo_destroy(m_cairo);
}

uva::colord cairo_normalize_color(const uva::color& __color)
{
    return __color / 255.0;
}

void cairo_set_source_rgba(cairo_t* cr, const uva::color& __color)
{
    uva::colord normalized = cairo_normalize_color(__color);
    cairo_set_source_rgba(cr, normalized.r, normalized.g, normalized.b, normalized.a);
}

void cairo_rectangle(cairo_t* cr, const uva::rect& __rect)
{
    cairo_rectangle(cr, __rect.x, __rect.y, __rect.w, __rect.h);
}

void uva::drawing::software_renderer::fill_rect(const uva::rect& __rect, const uva::color& __color)
{
    cairo_set_source_rgba(m_cairo, __color);
    cairo_rectangle(m_cairo, __rect);
    cairo_fill(m_cairo);
}

void uva::drawing::software_renderer::clear(const uva::color& __color)
{
    cairo_set_source_rgba(m_cairo, __color);
    cairo_paint(m_cairo);
}