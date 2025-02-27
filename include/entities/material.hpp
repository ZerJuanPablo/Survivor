#pragma once
#include <glm/glm.hpp> // Asumiendo que usas glm para manejar colores y vectores

struct Material {
    void bind() {
        // Textura
        glUniform1f(17, _texture_contribution);

        // Propiedades de iluminación
        glUniform1f(18, _specular);
        glUniform1f(19, _specular_shininess);

        // Colores del material
        glUniform3fv(20, 1, &_ambient[0]);   // Uniform para Ka (ambient)
        glUniform3fv(21, 1, &_diffuse[0]);   // Uniform para Kd (diffuse)
        glUniform3fv(22, 1, &_specularColor[0]); // Uniform para Ks (specular)
    }

    float _texture_contribution = 0;           // Contribución de la textura
    float _specular = 0.4;                       // Factor especular
    float _specular_shininess = 4;            // Brillo especular (Ns)

    glm::vec3 _ambient = glm::vec3(0.1f);      // Color ambiental (Ka)
    glm::vec3 _diffuse = glm::vec3(0.76f, 0.70f, 0.50f);      // Color difuso (Kd)
    glm::vec3 _specularColor = glm::vec3(0.1f, 0.1f, 0.1f); // Color especular (Ks)
};
