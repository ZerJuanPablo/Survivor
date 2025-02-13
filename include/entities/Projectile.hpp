// projectile.hpp
#pragma once
#include "entities/model.hpp"
#include <glm/glm.hpp>
#include "player.hpp"

struct Projectile {
    glm::vec3 position;
    glm::vec3 direction;
    float speed = 10.0f;
    float radius = 0.2f;
    float lifetime = 2.0f;
    Model model;

    float damage = 10.0f;
    int piercing_strength = 1;

    void init(const Player& player, const glm::vec3& pos, const glm::vec3& dir) {
        damage = player._damage;
        piercing_strength = player._piercing_strength;
        position = pos;
        direction = glm::normalize(dir);
        model.init(Mesh::eSphere);
        model._transform._scale = glm::vec3(radius);
    }
    
    void update(float delta_time) {
        position += direction * speed * delta_time;
        lifetime -= delta_time;
        model._transform._position = position;
    }
    
    void draw(bool bind_material = false) {
        model.draw(bind_material);
    }
};