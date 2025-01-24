#version 460 core
#define LIGHT_COUNT 2

// interpolated input from vertex shader
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;

// final pixel/fragment color
layout (location = 0) out vec4 out_color;
// texture unit
layout (binding = 0) uniform sampler2D tex_diffuse;
layout (binding = 1) uniform samplerCube tex_shadows[LIGHT_COUNT];
// uniform buffers
layout (location = 16) uniform vec3 camera_pos;
layout (location = 17) uniform float texture_contribution;
layout (location = 18) uniform float specular;
layout (location = 19) uniform float specular_shininess;
layout (location = 20) uniform vec3 mat_ambient = vec3(1, 1, 1); // TODO
layout (location = 21) uniform vec3 mat_diffuse = vec3(1, 1, 1); // TODO
layout (location = 22) uniform vec3 mat_specular = vec3(1, 1, 1); // TODO

struct Light {
    vec3 pos;    // +0
    vec3 col;    // +1
    float range; // +2
};
layout (location = 23) uniform Light lights[LIGHT_COUNT]; // location = 23 for the first one, 26 for the second

void main() { // blinn-phong shading
    vec3 norm = normalize(in_norm);

    // ambient color (low scattered indirect light)
    float ambient_strength = 0.1;
    vec3 ambient_col = ambient_strength * mat_ambient;
    vec3 diffuse_col = vec3(0, 0, 0);
    vec3 specular_col = vec3(0, 0, 0);

    // calculate diffuse and specular colors with each light
    for (int light_i = 0; light_i < LIGHT_COUNT; light_i++) {
        Light light = lights[light_i];
        vec3 light_dir = normalize(light.pos - in_pos); // unit vector from light to fragment/pixel

        // calculate shadow depth via distance from pixel/fragment to light
        vec3 fragment_to_light = in_pos - light.pos;
        float depth_current = length(fragment_to_light);
        // sample cube texture via vector, not UV coordinates
        // float depth_real = texture(tex_shadow, fragment_to_light).r;
        // depth_real = depth_real * light.range;
        // // use a bias to remedy sampling inaccuracies
        float bias_max = 1.0;
        float bias_min = 0.005;
        float bias = max((1.0 - dot(norm, light_dir) * bias_max), bias_min); 
        // // pixel is in shadow, if light did not see it
        // float light_contribution = 1.0;
        // if (depth_current - bias > depth_real) light_contribution = 0.0;

        // percentage-closer-filter
        float samples = 4;
        float offset = 0.1;
        float shadow = 0.0;
        for(float x = -offset; x < offset; x += offset / (samples * 0.5)) {
            for(float y = -offset; y < offset; y += offset / (samples * 0.5)) {
                for(float z = -offset; z < offset; z += offset / (samples * 0.5)) {
                    float depth_real = texture(tex_shadows[light_i], fragment_to_light + vec3(x, y, z)).r; 
                    depth_real *= light.range;
                    if(depth_current - bias > depth_real) shadow += 1.0;
                }
            }
        }
        // normalize shadow from 0 to 1
        shadow = shadow / (samples * samples * samples);
        // invert shadow to get light contribution
        float light_contribution = 1.0 - shadow;

        // calculate attenuation (point light)
        // see https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation for constant linear/quad factors
        float light_distance = length(light.pos - in_pos);
        float linear = light_distance * 0.14;
        float quadratic = light_distance * light_distance * 0.07;
        float attenuation = 1.0 / (1.0 + linear + quadratic);

        // direct light color
        float diffuse_strength = dot(norm, light_dir);
        diffuse_strength = max(diffuse_strength, 0.0); // filter out "negative" strength
        diffuse_col += light.col * diffuse_strength * attenuation * mat_diffuse * light_contribution;

        // specular color (reflected directly to camera)
        vec3 camera_dir = normalize(in_pos - camera_pos); // unit vector from camera to fragment/pixel
        vec3 reflected_dir = reflect(light_dir, norm);
        float specular_strength = dot(camera_dir, reflected_dir);
        specular_strength = max(specular_strength, 0.0); // filter out "negative" strength
        specular_strength = pow(specular_strength, specular_shininess);
        specular_strength = specular_strength * min(specular, 1.0);
        specular_col += light.col * specular_strength * attenuation * mat_specular * light_contribution;
    }

    // final color
    out_color = mix(in_col, texture(tex_diffuse, in_uv), texture_contribution);
    out_color.rgb = out_color.rgb * (ambient_col + diffuse_col + specular_col); // combine light-affected colors
}