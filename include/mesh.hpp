#pragma once
#include <glm/glm.hpp>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;

struct Mesh {
    // create mesh data (vertices, indices/elements, vertex arrays)
    void init() {
        // create vertices
        struct Vertex {
            glm::vec4 position;
            glm::vec4 color;
            glm::vec2 uv;
            glm::vec3 normal;
        };
        float n = -0.5f; // for readability
        float p = 0.5f; // for readability
        std::vector<Vertex> vertices = {
            {{n, n, p, 1}, {1, 0, 0, 1}, {0.33, 0.75}, {0 ,0 , +1}}, // front
            {{p, n, p, 1}, {1, 0, 0, 1}, {0.66, 0.75}, {0 ,0 , +1}},
            {{n, p, p, 1}, {1, 0, 0, 1}, {0.33, 0.50}, {0 ,0 , +1}},
            {{p, p, p, 1}, {1, 0, 0, 1}, {0.66, 0.50}, {0 ,0 , +1}},
            {{n, n, n, 1}, {1, 0, 0, 1}, {0.33, 0.00}, {0 ,0 , -1}}, // back
            {{p, n, n, 1}, {1, 0, 0, 1}, {0.66, 0.00}, {0 ,0 , -1}},
            {{n, p, n, 1}, {1, 0, 0, 1}, {0.33, 0.25}, {0 ,0 , -1}},
            {{p, p, n, 1}, {1, 0, 0, 1}, {0.66, 0.25}, {0 ,0 , +-1}},
            {{n, n, n, 1}, {0, 1, 0, 1}, {0.00, 0.50}, {-1 ,0 , 0}}, // left
            {{n, n, p, 1}, {0, 1, 0, 1}, {0.00, 0.25}, {-1 ,0 , 0}},
            {{n, p, n, 1}, {0, 1, 0, 1}, {0.33, 0.50}, {-1 ,0 , 0}},
            {{n, p, p, 1}, {0, 1, 0, 1}, {0.33, 0.25}, {-1 ,0 , 0}},
            {{p, n, n, 1}, {0, 1, 0, 1}, {1.00, 0.50}, {+1 ,0 , 0}}, // right
            {{p, n, p, 1}, {0, 1, 0, 1}, {1.00, 0.25}, {+1 ,0 , 0}},
            {{p, p, n, 1}, {0, 1, 0, 1}, {0.66, 0.50}, {+1 ,0 , 0}},
            {{p, p, p, 1}, {0, 1, 0, 1}, {0.66, 0.25}, {+1 ,0 , 0}},
            {{n, p, n, 1}, {0, 0, 1, 1}, {0.33, 0.25}, {0 , +1 , 0}}, // top
            {{n, p, p, 1}, {0, 0, 1, 1}, {0.33, 0.50}, {0 , +1 , 0}},
            {{p, p, n, 1}, {0, 0, 1, 1}, {0.66, 0.25}, {0 , +1 , 0}},
            {{p, p, p, 1}, {0, 0, 1, 1}, {0.66, 0.50}, {0 , +1 , 0}},
            {{n, n, n, 1}, {0, 0, 1, 1}, {0.33, 0.75}, {0 , -1 , 0}}, // bottom
            {{n, n, p, 1}, {0, 0, 1, 1}, {0.33, 1.00}, {0 , -1 , 0}},
            {{p, n, n, 1}, {0, 0, 1, 1}, {0.66, 0.75}, {0 , -1 , 0}},
            {{p, n, p, 1}, {0, 0, 1, 1}, {0.66, 1.00}, {0 , -1 , 0}},
        };
        
        // describe vertex buffer
        GLsizeiptr vertex_byte_count = vertices.size() * sizeof(Vertex);
        glCreateBuffers(1, &_vertex_buffer_object);
        // upload data to GPU buffer
        glNamedBufferStorage(_vertex_buffer_object, vertex_byte_count, vertices.data(), BufferStorageMask::GL_NONE_BIT);

        // create indices
        std::vector<uint32_t> indices = {
            0, 1, 3, 3, 2, 0, // front
            5, 4, 7, 7, 4, 6, // back
            8, 9, 11, 11, 10, 8, // left
            13, 12, 15, 15, 12, 14, // right
            16, 17, 19, 19, 18, 16, // top
            23, 21, 20, 23, 20, 22, // bottom
        };
        _index_count = indices.size();
        // describe index buffer (element buffer)
        GLsizeiptr element_byte_count = vertices.size() * sizeof(Vertex);
        glCreateBuffers(1, &_element_buffer_object);
        // upload data to GPU buffer
        glNamedBufferStorage(_element_buffer_object, element_byte_count, indices.data(), BufferStorageMask::GL_NONE_BIT);

        // create vertex array buffer
        glCreateVertexArrays(1, &_vertex_array_object);
        // assign both vertex and index (element) buffers
        glVertexArrayVertexBuffer(_vertex_array_object, 0, _vertex_buffer_object, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(_vertex_array_object, _element_buffer_object);
        // struct Vertex {
        //     glm::vec4 position; <---
        //     glm::vec4 color;
        //     glm::vec2 uv;
        //     glm::vec3 normal;
        // };
        // total size of 4 floats, starts at byte 0*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 0, 4, GL_FLOAT, GL_FALSE, 0 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 0, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 0);
        // struct Vertex {
        //     glm::vec4 position;
        //     glm::vec4 color; <---
        //     glm::vec2 uv;
        //     glm::vec3 normal;
        // };
        // total size of 4 floats, starts at byte 4*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 1, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 1);
        // struct Vertex {
        //     glm::vec4 position;
        //     glm::vec4 color;
        //     glm::vec2 uv; <---
        //     glm::vec3 normal;
        // };
        // total size of 2 floats, starts at byte 8*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 2, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 2);
         // struct Vertex {
        //     glm::vec4 position;
        //     glm::vec4 color;
        //     glm::vec2 uv; 
        //     glm::vec3 normal; <---
        // };
        // total size of 2 floats, starts at byte 8*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 3, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 3);
    }
    // clean up mesh buffers
    void destroy() {
        glDeleteBuffers(1, &_vertex_buffer_object);
        glDeleteBuffers(1, &_element_buffer_object);
        glDeleteVertexArrays(1, &_vertex_array_object);
    }
    // draw the mesh using previously bound pipeline
    void draw() {
        glBindVertexArray(_vertex_array_object);
        glDrawElements(GL_TRIANGLES, _index_count, GL_UNSIGNED_INT, nullptr);
    }

    GLuint _vertex_buffer_object;
    GLuint _element_buffer_object;
    GLuint _vertex_array_object;
    GLsizei _index_count;
};