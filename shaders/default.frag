#version 460 core

// interpolated input from vertex shader
layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_norm;
layout (location = 3) in vec3 in_pos;
// final pixel/fragment color
layout (location = 0) out vec4 out_color;
// texture unit
layout (binding = 0) uniform sampler2D tex_diffuse;
//uniform buffers
layout (location = 12) uniform vec3 camera_pos;

void main() {
    // out_color = in_color;
    // Blinn-Phong Shading
    // 1. Create a sun
    vec3 light_pos = vec3(1.0, 2.0, -4.0);
    vec3 light_col = vec3(.922, .984, .827);
    vec3 light_dir = normalize(light_pos - in_pos); // unit vector from fragment/pixel to light


    // 2. ambient color (low scattered indirect light)
    float ambient_strength = 0.1;
    vec3 ambient_color = light_col * ambient_strength;

    // 3. direct light color
    float diffuse_strength = dot(in_norm, light_dir);
    diffuse_strength = max(diffuse_strength, 0.0); // limit light to only be positive
    vec3 diffuse_color = light_col * diffuse_strength;

    // 4. specular color (reflected directly onto the camera)
    vec3 camera_dir = normalize(in_pos - camera_pos);
    vec3 reflected_dir = reflect(light_dir, in_norm);
    float specular_strength = dot(camera_dir, reflected_dir);
    specular_strength = max(specular_strength, 0.0);
    float specular = 1.0;
    float specular_shininess = 32.0;
    specular_strength = pow(specular_strength, specular_shininess);
    specular_strength = specular_strength * specular;
    vec3 specular_color = light_col * specular_strength;

    // final color
    vec3 color = texture(tex_diffuse, in_uv).rgb;
    color = color * (ambient_color + diffuse_color + specular_color);

    out_color = vec4(color, 1.0);
}