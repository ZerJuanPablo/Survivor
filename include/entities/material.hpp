#pragma once
#include <glm/glm.hpp>

struct Material {
    void bind() {
        glUniform1f(18, _texture_contribution);

        glUniform1f(19, _specular);
        glUniform1f(20, _specular_shininess);

        glUniform3fv(21, 1, &_ambient[0]);   // Ka (ambient)
        glUniform3fv(22, 1, &_diffuse[0]);   // Kd (diffuse)
        glUniform3fv(23, 1, &_specularColor[0]); // Ks (specular)
    }

    float _texture_contribution = 0;           
    float _specular = 0.4;                     
    float _specular_shininess = 4;           

    glm::vec3 _ambient = glm::vec3(0.1f);      
    glm::vec3 _diffuse = glm::vec3(0.76f, 0.70f, 0.50f);      
    glm::vec3 _specularColor = glm::vec3(0.1f, 0.1f, 0.1f); 
};
