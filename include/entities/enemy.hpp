#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
#include <cmath>
#include "entities/model.hpp"
#include "entities/player.hpp"

struct Enemy {

    enum class State {
        ALIVE,
        DEAD
    };

    void init(const std::string& model_path, const glm::vec3& center_offset = glm::vec3(0.0f)) {
        _model.init(model_path);
        _center_offset = center_offset;  // Ajustar manualmente si es necesario
        _radius = 1.0f;
        _state = State::ALIVE;
        _model._transform._scale = glm::vec3(0.5f);
    }

    /*
    ~Enemy() {
        destroy();
    }
    */


    void destroy() {
        _model.destroy();
        printf("Enemy destroyed\n");
    }

    void update(float delta_time, Player &player) {
        glm::vec3 player_pos = player.get_position();
        if (_state == State::DEAD) {
            return;
        }
        update_movement(delta_time, player_pos);
        update_rotation(player_pos);
        check_collision(player);
    }

    void check_collision(Player &player) {

        if (glm::distance(get_position(), player.get_position()) - _radius <= 0.0f ) { // player pos -> player radius
            _state = State::DEAD;
            player.take_damage(_damage);
            destroy();
        }
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

    glm::vec3 get_center() const {
        return _model._transform._position + _center_offset;
    }

    float _move_speed = 0.5f;
    int _hp = 100;
    int max_hp = 100;
    float _damage = 10.0f;
    int _level = 1;
    Model _model;
    glm::vec3 _center_offset = glm::vec3(0.0f);
    float _radius = 1.0f;
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
