#pragma once

#include "enemy.hpp"
#include "light.hpp"
#include <random>
#include <iostream> 

class Boss : public Enemy {
public:
    Boss() = default;
    void init_boss(const std::string& model_path, 
                   const glm::vec3& spawn_pos,
                   float speed,
                   float maxHp,
                   float dmg,
                   float radius) 
        {
            _model.init(model_path);
            set_position(spawn_pos);
            _move_speed = speed;
            max_hp      = maxHp;
            _hp         = maxHp;
            _damage     = dmg;
            _radius     = radius;
        }

    void update(float delta_time, Player& player) override {
        Enemy::update(delta_time, player);
        _move_speed = _move_speed + 0.003f;
    }

    void die() override {
        Enemy::die();
    }

    void teleport_near_player(const glm::vec3& player_pos) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(4.0f, 5.0f);
        float distance_x = dis(gen);
        float distance_z = dis(gen);
        glm::vec3 new_pos = player_pos + glm::vec3(distance_x, 0.0f, distance_z);
        set_position(new_pos);
        _move_speed = 1.5f;
    }
    float base_xp = 20.0f;
private:
};
