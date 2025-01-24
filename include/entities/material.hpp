#pragma once
#include <cstdint>

struct Material {
    void bind() {
        glUniform1f(17, _texture_contribution);
        glUniform1f(18, _specular);
        glUniform1f(19, _specular_shininess);
    }

    float _texture_contribution = 0;
    float _specular = 1;
    float _specular_shininess = 32;
};