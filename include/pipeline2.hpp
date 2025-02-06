#pragma once
#include <fstream>
#include <fmt/base.h>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <vector>
#include <glm/glm.hpp>

struct Pipeline2 {
    GLuint _shader_program;
    GLuint _framebuffer = 0;
    GLuint instanceVBO = 0; // Buffer de instancias
    bool _use_instancing = false; // Indica si se usará instancing

    // Inicializa el pipeline
    void init(const char* vs_path, const char* fs_path, bool use_instancing = false) {
        _use_instancing = use_instancing;
        compile_shaders(vs_path, fs_path);

        if (_use_instancing) {
            create_instance_buffer();
        }
    }

    // Crea un buffer para instancias solo si se usa instancing
    void create_instance_buffer() {
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, 500 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

        for (int i = 0; i < 4; i++) { // Una matriz ocupa 4 atributos vec4
            glEnableVertexAttribArray(4 + i);
            glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(4 + i, 1); // Cada instancia usa una única matriz
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Actualiza los datos de instancias
    void update_instance_data(const std::vector<glm::mat4>& transforms) {
        if (!_use_instancing) return;

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(glm::mat4), transforms.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Vincular shader y framebuffer
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
        glUseProgram(_shader_program);
    }

    // Dibujar con o sin instancing
    void draw(GLuint VAO, int indexCount, int instanceCount = 1) {
        glBindVertexArray(VAO);

        if (_use_instancing) {
            glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, instanceCount);
        } else {
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
    }

    // Liberar recursos
    void destroy() {
        glDeleteProgram(_shader_program);
        if (_use_instancing) {
            glDeleteBuffers(1, &instanceVBO);
        }
    }

private:
    void compile_shaders(const char* vs_path, const char* fs_path) {
        std::ifstream vs_file(vs_path, std::ios::binary);
        vs_file.seekg(0, std::ios::end);
        GLint vs_size = vs_file.tellg();
        vs_file.seekg(0, std::ios::beg);
        std::vector<GLchar> vs_string(vs_size);
        GLchar* vs_data = vs_string.data();
        vs_file.read(vs_data, vs_size);

        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vs_data, &vs_size);
        glCompileShader(vertex_shader);

        GLint success;
        std::vector<GLchar> info_log(512);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex_shader, info_log.size(), nullptr, info_log.data());
            fmt::print("{}", info_log.data());
        }

        std::ifstream fs_file(fs_path, std::ios::binary);
        fs_file.seekg(0, std::ios::end);
        GLint fs_size = fs_file.tellg();
        fs_file.seekg(0, std::ios::beg);
        std::vector<GLchar> fs_string(fs_size);
        GLchar* fs_data = fs_string.data();
        fs_file.read(fs_data, fs_size);

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fs_data, &fs_size);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragment_shader, info_log.size(), nullptr, info_log.data());
            fmt::print("{}", info_log.data());
        }

        _shader_program = glCreateProgram();
        glAttachShader(_shader_program, vertex_shader);
        glAttachShader(_shader_program, fragment_shader);
        glLinkProgram(_shader_program);
        glGetProgramiv(_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(_shader_program, info_log.size(), nullptr, info_log.data());
            fmt::print("{}", info_log.data());
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }
};
