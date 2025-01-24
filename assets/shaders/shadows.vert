#version 460 core

// input
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;
// output
layout (location = 0) out vec3 out_pos;
// uniforms
layout (location = 0) uniform mat4x4 model_transform;
layout (location = 4) uniform mat4x4 normal_transform;
layout (location = 8) uniform mat4x4 camera_transform;
layout (location = 12) uniform mat4x4 camera_perspective;

void main() {
    gl_Position = model_transform * vec4(in_pos, 1.0);
    out_pos = gl_Position.xyz;
    gl_Position = camera_transform * gl_Position;
    gl_Position = camera_perspective * gl_Position;
}