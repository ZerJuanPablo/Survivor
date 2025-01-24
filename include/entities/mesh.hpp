#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <assimp/mesh.h>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;

struct Mesh {
    enum Primitive { eCube, eSphere };
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;
        glm::vec2 uv;
    };

    // load cube primitive
    void init() {
        // create vertices
        float n = -0.5f; // for readability
        float p = +0.5f; // for readability
        std::vector<Vertex> vertices = {
            {{n, n, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.33, 0.75}}, // front
            {{p, n, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.66, 0.75}},
            {{n, p, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.33, 0.50}},
            {{p, p, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.66, 0.50}},
            {{n, n, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.33, 0.00}}, // back
            {{p, n, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.66, 0.00}},
            {{n, p, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.33, 0.25}},
            {{p, p, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.66, 0.25}},
            {{n, n, n}, {-1, 0, 0}, {0, 1, 0, 1}, {0.00, 0.50}}, // left
            {{n, n, p}, {-1, 0, 0}, {0, 1, 0, 1}, {0.00, 0.25}},
            {{n, p, n}, {-1, 0, 0}, {0, 1, 0, 1}, {0.33, 0.50}},
            {{n, p, p}, {-1, 0, 0}, {0, 1, 0, 1}, {0.33, 0.25}},
            {{p, n, n}, {+1, 0, 0}, {0, 1, 0, 1}, {1.00, 0.50}}, // right
            {{p, n, p}, {+1, 0, 0}, {0, 1, 0, 1}, {1.00, 0.25}},
            {{p, p, n}, {+1, 0, 0}, {0, 1, 0, 1}, {0.66, 0.50}},
            {{p, p, p}, {+1, 0, 0}, {0, 1, 0, 1}, {0.66, 0.25}},
            {{n, p, n}, {0, +1, 0}, {0, 0, 1, 1}, {0.33, 0.25}}, // top
            {{n, p, p}, {0, +1, 0}, {0, 0, 1, 1}, {0.33, 0.50}},
            {{p, p, n}, {0, +1, 0}, {0, 0, 1, 1}, {0.66, 0.25}},
            {{p, p, p}, {0, +1, 0}, {0, 0, 1, 1}, {0.66, 0.50}},
            {{n, n, n}, {0, -1, 0}, {0, 0, 1, 1}, {0.33, 0.75}}, // bottom
            {{n, n, p}, {0, -1, 0}, {0, 0, 1, 1}, {0.33, 1.00}},
            {{p, n, n}, {0, -1, 0}, {0, 0, 1, 1}, {0.66, 0.75}},
            {{p, n, p}, {0, -1, 0}, {0, 0, 1, 1}, {0.66, 1.00}},
        };

        // create indices
        std::vector<uint32_t> indices = {
            0, 1, 3, 3, 2, 0, // front
            5, 4, 7, 7, 4, 6, // back
            8, 9, 11, 11, 10, 8, // left
            13, 12, 15, 15, 12, 14, // right
            16, 17, 19, 19, 18, 16, // top
            23, 21, 20, 23, 20, 22, // bottom
        };
        describe_layout(vertices, indices);
    }
    // load sphere primitive
    void init(uint32_t sector_count, uint32_t stack_count) {
        // https://www.songho.ca/opengl/gl_sphere.html
        float pi = 3.14159265358979323846f;
        float radius = 0.5f;
        // precalc expensive operations
        float length_recip = 1.0f / radius;
        float sector_step = 2.0f * pi / static_cast<float>(sector_count);
        float stack_step = pi / static_cast<float>(stack_count);

        // preallocate some space for vertices
        std::vector<Vertex> vertices;
        vertices.reserve((sector_count + 1) * (stack_count + 1));

        // create vertices
        for (uint32_t i = 0; i <= stack_count; i++) {
            float stack_angle = pi / 2.0f - static_cast<float>(i) * stack_step;
            float xy = radius * std::cos(stack_angle);
            float z = radius * std::sin(stack_angle);

            for (uint32_t k = 0; k <= sector_count; k++) {
                Vertex& vertex = vertices.emplace_back();

                // calculate position
                float sector_angle = static_cast<float>(k) * sector_step;
                vertex.position.x = xy * std::cos(sector_angle);
                vertex.position.y = xy * std::sin(sector_angle);
                vertex.position.z = z;

                // calculate normal
                vertex.normal = vertex.position * length_recip;

                // calculate uv/st coordinates
                // vertex.st.s = static_cast<float>(k) / sector_count;
                // vertex.st.t = static_cast<float>(i) / stack_count;
                vertex.color = glm::vec4(1, 1, 1, 1);
            }
        }

        // create indices
        // k1--k1+1
        // |  / |
        // | /  |
        // k2--k2+1
        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < stack_count; i++) {
            uint32_t k1 = i * (sector_count + 1); // beginning of current stack
            uint32_t k2 = k1 + sector_count + 1;  // beginning of next stack

            for (uint32_t j = 0; j < (uint32_t)sector_count; j++, k1++, k2++) {
                // 2 triangles per sector excluding first and last stacks
                if (i != 0) {
                    indices.insert(indices.end(), {
                        k1, k2, k1 + 1
                    });
                }
                if (i != stack_count - 1) {
                    indices.insert(indices.end(), {
                        k1 + 1, k2, k2 + 1
                    });
                }
            }
        }
        describe_layout(vertices, indices);
    }
    // load mesh from assimp scene
    void init(aiMesh* mesh_p) {
        std::vector<Vertex> vertices;
        vertices.reserve(mesh_p->mNumVertices);
        for (uint32_t i = 0; i < mesh_p->mNumVertices; i++) {
            Vertex vertex;
            // extract positions
            vertex.position.x = mesh_p->mVertices[i].x;
            vertex.position.y = mesh_p->mVertices[i].y;
            vertex.position.z = mesh_p->mVertices[i].z;
            // extract normals
            vertex.normal.x = mesh_p->mNormals[i].x;
            vertex.normal.y = mesh_p->mNormals[i].y;
            vertex.normal.z = mesh_p->mNormals[i].z;
            // extract uv/st coords
            if (mesh_p->HasTextureCoords(0)) {
                vertex.uv.s = mesh_p->mTextureCoords[0][i].x;
                vertex.uv.t = mesh_p->mTextureCoords[0][i].y;
            }
            else vertex.uv = {0, 0};
            // extract vertex colors (if present)
            if (mesh_p->HasVertexColors(0)) {
                vertex.color.r = mesh_p->mColors[0][i].r;
                vertex.color.g = mesh_p->mColors[0][i].g;
                vertex.color.b = mesh_p->mColors[0][i].b;
                vertex.color.a = mesh_p->mColors[0][i].a;
            }
            else vertex.color = {1, 1, 1, 1};
            vertices.push_back(vertex);
        }

        std::vector<uint32_t> indices;
        indices.reserve(mesh_p->mNumFaces * 3);
        for (int i = 0; i < mesh_p->mNumFaces; i++) {
            aiFace face = mesh_p->mFaces[i];
            assert(face.mNumIndices == 3);
            for (int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        _material_index = mesh_p->mMaterialIndex;
        describe_layout(vertices, indices);
    }
    // describe memory layout
    void describe_layout(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        _index_count = indices.size();

        // describe vertex buffer
        GLsizeiptr vertex_byte_count = vertices.size() * sizeof(Vertex);
        glCreateBuffers(1, &_vertex_buffer_object);
        // upload data to GPU buffer
        glNamedBufferStorage(_vertex_buffer_object, vertex_byte_count, vertices.data(), BufferStorageMask::GL_NONE_BIT);

        // describe index buffer (element buffer)
        GLsizeiptr element_byte_count = indices.size() * sizeof(uint32_t);
        glCreateBuffers(1, &_element_buffer_object);
        // upload data to GPU buffer
        glNamedBufferStorage(_element_buffer_object, element_byte_count, indices.data(), BufferStorageMask::GL_NONE_BIT);

        // create vertex array buffer
        glCreateVertexArrays(1, &_vertex_array_object);
        // assign both vertex and index (element) buffers
        glVertexArrayVertexBuffer(_vertex_array_object, 0, _vertex_buffer_object, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(_vertex_array_object, _element_buffer_object);
        // struct Vertex {
        //     glm::vec3 position; <---
        //     glm::vec3 normal;
        //     glm::vec3 color;
        //     glm::vec2 uv;
        // };
        // total size of 3 floats, starts at byte 0*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 0, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 0);
        // struct Vertex {
        //     glm::vec3 position;
        //     glm::vec3 normal; <---
        //     glm::vec3 color;
        //     glm::vec2 uv;
        // };
        // total size of 3 floats, starts at byte 3*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 1, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 1);
        // struct Vertex {
        //     glm::vec3 position;
        //     glm::vec3 normal;
        //     glm::vec3 color; <---
        //     glm::vec2 uv;
        // };
        // total size of 4 floats, starts at byte 6*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 2, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 2, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 2);
        // struct Vertex {
        //     glm::vec3 position;
        //     glm::vec3 normal;
        //     glm::vec3 color;
        //     glm::vec2 uv;
        // };
        // total size of 2 floats, starts at byte 10*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GL_FLOAT));
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
    uint32_t _material_index = 0;
};