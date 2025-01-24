#version 460 core

// interpolated input from vertex shader
layout (location = 0) in vec3 in_pos;

// texture unit
layout (binding = 0) uniform sampler2D tex_diffuse;
// uniform buffers
layout (location = 16) uniform vec3 camera_pos;
layout (location = 17) uniform float texture_contribution;
layout (location = 18) uniform float specular;
layout (location = 19) uniform float specular_shininess;
layout (location = 20) uniform vec3 mat_ambient = vec3(1, 1, 1); // TODO
layout (location = 21) uniform vec3 mat_diffuse = vec3(1, 1, 1); // TODO
layout (location = 22) uniform vec3 mat_specular = vec3(1, 1, 1); // TODO
layout (location = 23) uniform vec3 light_pos;
layout (location = 24) uniform vec3 light_col;
layout (location = 25) uniform float light_range;

void main() {
    // do not write any color output, only depth
    vec3 fragment_to_light = in_pos - light_pos;
    float light_distance = length(fragment_to_light);
    // scale down to 0-1
    light_distance = light_distance / light_range;
    // distance from light to the pixel/fragment is our "depth"
    gl_FragDepth = light_distance;
}