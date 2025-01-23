#version 460 core

// interpolated input from vertex shader
layout (location = 0) in vec4 in_color;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec3 in_pos;

// final pixel/fragment color
layout (location = 0) out vec4 out_color;

// uniform for camera position
layout (location = 12) uniform vec3 camera_pos;

void main() {
    // Create a sun
    vec3 light_pos = vec3(1.0, 2.0, -4.0);
    vec3 light_col = vec3(.922, .984, .827);
    vec3 light_dir = normalize(light_pos - in_pos);

    // Ambient color
    float ambient_strength = 0.1;
    vec3 ambient_color = light_col * ambient_strength;

    // Direct light color (Diffuse)
    float diffuse_strength = max(dot(in_norm, light_dir), 0.0);
    vec3 diffuse_color = light_col * diffuse_strength;

    // Specular color
    vec3 camera_dir = normalize(camera_pos - in_pos);
    vec3 reflected_dir = reflect(-light_dir, in_norm);
    float specular_strength = pow(max(dot(camera_dir, reflected_dir), 0.0), 32.0);
    float specular = 1.0;
    vec3 specular_color = light_col * specular_strength * specular;

    // Calculate the final color
    vec3 color = in_color.rgb;
    color = color * (ambient_color + diffuse_color) + specular_color;

    out_color = vec4(color, 1.0);
}
