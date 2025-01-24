#pragma once
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <glm/glm.hpp>

struct Light {
    void init(glm::vec3 position, glm::vec3 color, float range) {
        // set member vars
        _position = position;
        _color = color;
        _range = range;
        // create shadow texture as cube map
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_shadow_texture);
        glTextureStorage2D(_shadow_texture, 1, GL_DEPTH_COMPONENT32F, _shadow_width, _shadow_height);
        // set wrapping/magnification behavior
        glTextureParameteri(_shadow_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(_shadow_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(_shadow_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_shadow_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // create shadow camera matrices
        _shadow_projection = glm::perspectiveFov<float>(glm::radians(90.0f), _shadow_width, _shadow_height, 1.0f, _range);
        _shadow_views[0] = glm::lookAt(_position, _position + glm::vec3(+1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // right
        _shadow_views[1] = glm::lookAt(_position, _position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // left
        _shadow_views[2] = glm::lookAt(_position, _position + glm::vec3( 0.0f, +1.0f,  0.0f), glm::vec3(0.0f,  0.0f, +1.0f)); // top
        _shadow_views[3] = glm::lookAt(_position, _position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); // bottom
        _shadow_views[4] = glm::lookAt(_position, _position + glm::vec3( 0.0f,  0.0f, +1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // back
        _shadow_views[5] = glm::lookAt(_position, _position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // front
    }
    void destroy() {
        glDeleteTextures(1, &_shadow_texture);
    }
    void bind(GLuint offset = 0) {
        // bind simple light properties
        glUniform3f(23 + offset, _position.x, _position.y, _position.z);
        glUniform3f(24 + offset, _color.r, _color.g, _color.b);
        glUniform1f(25 + offset, _range);
    }
    void bind_write(GLuint framebuffer, GLuint face_i) {
        bind();
        // set framebuffer texture and clear it
        glNamedFramebufferTextureLayer(framebuffer, GL_DEPTH_ATTACHMENT, _shadow_texture, 0, face_i);
        // bind the light view+projection matrices (act like it is the camera)
        glUniformMatrix4fv( 8, 1, false, glm::value_ptr(_shadow_views[face_i]));
        glUniformMatrix4fv(12, 1, false, glm::value_ptr(_shadow_projection));
    }
    void bind_read(GLuint tex_unit, GLuint offset) {
        bind(offset);
        // bind the entire cube map for reading
        glBindTextureUnit(tex_unit, _shadow_texture);
    }
    glm::vec3 _position = {0, 0, 0};
    glm::vec3 _color = {1, 1, 1};
    float _range = 100;
    // shadow rendering
    GLuint _shadow_width = 512;
    GLuint _shadow_height = 512;
    GLuint _shadow_texture; // cube map (6 textures)
    std::array<glm::mat4x4, 6> _shadow_views; // one view for each texture in cube map
    glm::mat4x4 _shadow_projection;
};