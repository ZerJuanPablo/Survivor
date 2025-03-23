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

    void init(const glm::vec3& position, const glm::vec3& direction, float speed, float damage, int piercing) {
        _position = position;
        _direction = glm::normalize(direction);
        _speed = speed;
        _active = true;
        _damage = damage;
        _piercing = piercing;

        _lifespan = 2.0f;

        _model.init(Mesh::eSphere);                 
        _model._transform._scale = glm::vec3(0.2f); 
        _model._transform._position = _position;    
        _radius = 0.2f;
    }

    
    void update(float deltaTime) {
        if (_piercing <= 0){
            deactivate();
        }

        _lifespan -= deltaTime; 
        if (_lifespan <= 0) {
            _active = false;
        }

        if (!_active) return;

        _position += _direction * _speed * deltaTime;
        _model._transform._position = _position;
    }

    void draw(bool bind_material = false) {
        _model.draw(bind_material);
    }

    bool is_active() const { return _active; }
    const glm::vec3& get_position() const { return _position; }
    float get_radius() const { return _radius; }
    float get_damage() const { return _damage; }

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
    float _lifespan = 0.0f;
};
