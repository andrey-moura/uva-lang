#include <GL/gl.h>

#include <uva/drawing.hpp>

struct hardware_surface_data {
    GLuint texture = 0;
};

#define m_texture os_specific_data_as<hardware_surface_data>().texture

uva::drawing::hardware_renderer::hardware_renderer(surface& __surface)
    : basic_renderer(__surface)
{

}

uva::drawing::hardware_renderer::~hardware_renderer()
{
    
}

void uva::drawing::hardware_renderer::fill_rect(const uva::rect& __rect, const uva::color& __color)
{
}

void uva::drawing::hardware_renderer::clear(const uva::color& __color)
{

}