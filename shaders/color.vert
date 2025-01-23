#version 460 core

// input
layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec4 in_col;
layout (location = 2) in vec3 in_norm; // Normal input

// output
layout (location = 0) out vec4 out_col;
layout (location = 1) out vec3 out_norm;
layout (location = 2) out vec3 out_pos;

// uniforms
layout (location = 0) uniform mat4 model_transform;
layout (location = 4) uniform mat4 camera_transform;
layout (location = 8) uniform mat4 camera_perspective;

void main() {
    // Transform vertex position by model, view, and projection matrices
    gl_Position = camera_perspective * camera_transform * model_transform * in_pos;
    
    // Pass through color and normal values
    out_col = in_col;
    out_norm = mat3(model_transform) * in_norm;
    out_pos = vec3(model_transform * in_pos);
}
