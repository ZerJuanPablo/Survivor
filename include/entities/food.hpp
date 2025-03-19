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


struct Food {
    enum class State {
        ALIVE,
        DEAD
    };

    void init(const std::string& model_path, const glm::vec3& center_offset = glm::vec3(0.0f)) {
        _model.init(model_path);
        _center_offset = center_offset;
        _model._transform._scale = glm::vec3(5.0f);
    }

    virtual void eat() {
        _state = State::DEAD;
    }   

    void draw(bool bind_material = false) {
        _model.draw(bind_material);
    }

    void set_rotation(float angle) {
        _model._transform._rotation.y = angle;
    }
    
    void set_position(const glm::vec3& pos) {
        _model._transform._position = pos - _center_offset;  
    }

    glm::vec3 get_position() const {
        return _model._transform._position + _center_offset; 
    }

    void update_rotation(float delta_time) {
        float rotation_speed = 5.0f;
        _model._transform._rotation.y += rotation_speed * delta_time;
    }

    int heal = 15;
    Model _model;
    glm::vec3 _center_offset = glm::vec3(0.0f);
    float _radius = 0.7f;
    State _state = State::ALIVE;

private:


};
