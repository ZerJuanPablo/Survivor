#version 460 core

// input
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in mat4 intance_transform; // Instancing

// output
layout (location = 0) out vec3 out_pos;    // Posición en espacio mundo
layout (location = 1) out vec3 out_norm;   // Normal interpolada
layout (location = 2) out vec4 out_col;    // Color interpolado
layout (location = 3) out vec2 out_uv;     // Coordenadas UV

// uniforms
layout (location = 8) uniform mat4x4 camera_transform;
layout (location = 12) uniform mat4x4 camera_perspective;


void main() {
    vec4 world_pos = intance_transform * vec4(in_pos, 1.0);
    gl_Position = camera_perspective * camera_transform * world_pos;

    // Enviar atributos a fragment shader
    out_pos = world_pos.xyz;                                  // Posición en espacio mundo
    out_norm = normalize(mat3(normal_transform) * in_norm);   // Transformar y normalizar normales
    out_col = in_col;                                         // Color interpolado
    out_uv = in_uv;                                           // Coordenadas UV
}   