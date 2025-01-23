#pragma once
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <stb_image.h>

struct Texture {
    void init(const char* path) {
        // load image
        int width, height, nChannels; // output for stbi_load_from_memory
        stbi_uc* image_p = stbi_load(path, &width, &height, &nChannels, 3); // explicitly ask for 3 channels
        // create texture to store image in (texture is gpu buffer)
        glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
        glTextureStorage2D(_texture, 1, GL_RGB8, width, height);
        glTextureSubImage2D(_texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image_p);
        // free image on cpu side
        stbi_image_free(image_p);
        // sampler parameters
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, GL_REPEAT); // s is the u coordinate (width)
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, GL_REPEAT); // t is the v coordinate (height)
        glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // interpolation mode when scaling image down
        glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // interpolation mode when scaling image up
    }
    void destroy() {
        glDeleteTextures(1, &_texture);
    }
    void bind() {
        glBindTextureUnit(0, _texture);
    }

    GLuint _texture;
};