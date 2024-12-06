#include <GL/glew.h>
#include <GL/gl.h>

#include <uva/drawing.hpp>

struct texture_surface_data {
    GLuint texture = 0;
    GLuint framebuffer = 0;
};

#define m_texture os_specific_data_as<texture_surface_data>().texture

uva::drawing::texture_surface::texture_surface(const uva::size& s)
    : surface(s)
{
    texture_surface_data data;

    glGenTextures(1, &data.texture);
    glBindTexture(GL_TEXTURE_2D, data.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s.w, s.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &data.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, data.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data.texture, 0);

    os_specific_data_as<texture_surface_data>() = data;
}

uva::drawing::texture_surface::~texture_surface()
{
    glDeleteTextures(1, &m_texture);
    m_texture = 0;
}