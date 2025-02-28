#version 460 core
#define LIGHT_COUNT 2

// Input desde el vertex shader
layout (location = 0) in vec3 in_pos;  // Posición en espacio mundo
layout (location = 1) in vec3 in_norm; // Normal en espacio mundo
layout (location = 2) in vec4 in_col;  // Color del vértice
layout (location = 3) in vec2 in_uv;   // Coordenadas UV

// Salida final
layout (location = 0) out vec4 out_color;

// Uniforms (texturas y materiales)
layout (binding = 0) uniform sampler2D tex_diffuse;
layout (binding = 1) uniform samplerCube tex_shadows[LIGHT_COUNT];
layout (location = 17) uniform vec3 camera_pos;
layout (location = 18) uniform float texture_contribution;
layout (location = 19) uniform float specular;
layout (location = 20) uniform float specular_shininess;
layout (location = 21) uniform vec3 mat_ambient;
layout (location = 22) uniform vec3 mat_diffuse;
layout (location = 23) uniform vec3 mat_specular;

// Estructura de luz
struct Light {
    vec3 pos;    // Posición de la luz
    vec3 col;    // Color de la luz
    float range; // Alcance de la luz
};
layout (location = 24) uniform Light lights[LIGHT_COUNT];

// Cálculo principal
void main() {
    vec3 norm = normalize(in_norm);         // Normal normalizada
    vec3 view_dir = normalize(camera_pos - in_pos); // Vector hacia la cámara

    // Iluminación ambiental
    vec3 ambient = mat_ambient * 0.05;

    // Iluminación difusa y especular
    vec3 diffuse = vec3(0.0);
    vec3 specular_col = vec3(0.0);

    for (int i = 0; i < LIGHT_COUNT; i++) {
        Light light = lights[i];
        vec3 light_dir = normalize(light.pos - in_pos); // Vector hacia la luz
        float light_dist = length(light.pos - in_pos);

        // Atenuación de la luz
        float attenuation = 1.0 / (1.0 + 0.14 * light_dist + 0.07 * light_dist * light_dist);

        // Componente difusa
        float diff = max(dot(norm, light_dir), 0.0);
        diffuse += diff * mat_diffuse * light.col * attenuation;

        // Componente especular
        vec3 reflect_dir = reflect(-light_dir, norm);
        float spec = pow(max(dot(view_dir, reflect_dir), 0.0), specular_shininess);
        specular_col += spec * mat_specular * light.col * attenuation;
    }

    // Color de la textura
    vec3 texture_color = texture(tex_diffuse, in_uv).rgb;

    // Combinación final (mezcla color del vértice y textura con contribución de iluminación)
    vec3 final_color = (ambient + diffuse + specular_col) * mix(in_col.rgb, texture_color, texture_contribution);
    out_color = vec4(final_color, 1.0);
}
