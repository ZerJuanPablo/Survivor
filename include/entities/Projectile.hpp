#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
#include <cmath>
#include "entities/model.hpp"
#include "entities/player.hpp"

class Projectile {
public:
    Projectile() = default;

    // Inicializa la bala con posición inicial, dirección, velocidad, etc.
    void init(const glm::vec3& position, const glm::vec3& direction, float speed, float damage, int piercing) {
        _position = position;
        _direction = glm::normalize(direction);
        _speed = speed;
        _active = true;
        _damage = damage;
        _piercing = piercing;

        // Si quieres usar un modelo para la bala (una esfera pequeña, por ejemplo):
        _model.init(Mesh::eSphere);                 // Carga un mesh de esfera
        _model._transform._scale = glm::vec3(0.2f); // Escala pequeña
        _model._transform._position = _position;    // Alinea transform con posición

        // Radio para colisiones:
        _radius = 0.2f;
    }

    // Actualiza la posición de la bala
    void update(float deltaTime) {
        if (_piercing <= 0){
            deactivate();
        }

        // Aquí podrías poner lógica para desactivar la bala si está "muy lejos" o ya no te sirve
        float max_distance = 200.0f; 
        if (glm::length(_position) > max_distance) {
            _active = false;
        }

        if (!_active) return;

        _position += _direction * _speed * deltaTime;
        _model._transform._position = _position;
    }

    // Dibuja la bala
    void draw(bool bind_material = false) {
        _model.draw(bind_material);
    }

    // Getters
    bool is_active() const { return _active; }
    const glm::vec3& get_position() const { return _position; }
    float get_radius() const { return _radius; }

    // Permite desactivarla (si impacta a un enemigo, etc.)
    void deactivate() { _active = false; }

    float _damage;
    int _piercing;
    Model _model;
private:
    glm::vec3 _position = glm::vec3(0.0f);
    glm::vec3 _direction = glm::vec3(0.0f);
    float _speed = 0.0f;
    bool _active = false;
    float _radius = 0.2f;
};
