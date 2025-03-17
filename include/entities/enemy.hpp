#pragma once
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
#include <cmath>
#include "entities/model.hpp"
#include "entities/player.hpp"

enum class EnemyType {
    SHARK, // basic
    KOI,  // speedy
    BLOB // tank 
};

struct EnemyConfig {
    std::string model_key;
    glm::vec3 center_offset;
    float move_speed;
    float max_hp;
    float damage;
    float radius;
};

struct Enemy {

    enum class State {
        ALIVE,
        DEAD
    };

    void init(const std::string& model_path, const glm::vec3& center_offset = glm::vec3(0.0f)) {
        _model.init(model_path);
        _center_offset = center_offset;  // Ajustar manualmente si es necesario
        _model._transform._scale = glm::vec3(0.5f);
    }

    void init_from_config(const EnemyConfig& config, int difficulty){
        _move_speed = config.move_speed * (1 + (0.1 * (difficulty-1)));
        max_hp = config.max_hp + (5 * (difficulty-1));
        _hp = max_hp;
        _damage = config.damage * (1 + (0.2 * (difficulty-1)));
        _radius = config.radius;
        _center_offset = config.center_offset;
    }
    /*
    ~Enemy() {
        destroy();
    }
    */


    void destroy() {
    }

    virtual void update(float delta_time, Player &player) {
        if (_state == State::DEAD) return;
        
        const glm::vec3 player_pos = player.get_position();
        update_movement(delta_time, player_pos);
        update_rotation(player_pos);

        _move_speed = _move_speed + 0.003f; // Increase speed over time
    }

    void take_damage(float ammount, Player &player) {
        _hp -= ammount;
        if (_hp <= 0) {
            die();
            player.gain_xp(base_xp);
        }
    }

    virtual void die() {
        _state = State::DEAD;
        // sound
        // drop xp

        destroy();
    }   

    void draw(bool bind_material = false) {
        _model.draw(bind_material);
    }

    void set_rotation(float angle) {
        _model._transform._rotation.y = angle;
    }
    
    void set_position(const glm::vec3& pos) {
        _model._transform._position = pos - _center_offset;  // Ajustamos la posición real
    }

    glm::vec3 get_position() const {
        return _model._transform._position + _center_offset;  // Devolvemos la posición corregida
    }

    float _move_speed = 0.5f;
    int _hp = 100;
    int max_hp = 100;
    float _damage = 10.0f;
    int _level = 1;
    Model _model;
    glm::vec3 _center_offset = glm::vec3(0.0f);
    float _radius = 1.0f;
    float base_xp = 3.0f;
    State _state = State::ALIVE;

private:

    void update_movement(float delta_time, glm::vec3 player_pos) {
        glm::vec3 direction = glm::normalize(player_pos - get_position());
        glm::vec3 new_position = get_position() + direction * _move_speed * delta_time;
        set_position(new_position);
    }

    void update_rotation(glm::vec3 player_pos) {
        _model.look_at(player_pos);
    }

};
