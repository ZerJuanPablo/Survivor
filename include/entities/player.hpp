#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
#include <cmath>
#include "entities/model.hpp"
#include "input.hpp"

struct Player {
public:

    void init(const std::string& model_path, const glm::vec3& center_offset = glm::vec3(0.0f)) {
        _model.init(model_path);
        _center_offset = center_offset;  // Ajustar manualmente si es necesario
    }

    void destroy() {
        _model.destroy();
    }

    void update(float delta_time, float window_width, float window_height, const glm::mat4& projection_mat, const glm::mat4& inv_view_mat) {
        update_movement(delta_time);
        _mouse_world_position = calculate_mouse_world_position(
            window_width, 
            window_height, 
            glm::inverse(projection_mat), inv_view_mat);
        update_rotation(_mouse_world_position);
    }

    void take_damage(float damage) {
        _hp -= damage;
        // AudioManager::getInstance().play("eat");
        if (_hp < 0) _hp = 0;
        if (_hp == 0) {
            // Game over
        }
    }

    void draw(bool bind_material = false) {
        _model.draw(bind_material);
    }

    void gain_xp(float amount) {
        _xp += amount * _xp_multiplier;
        if (_xp >= _xp_needed){
            _level++;
            _xp = _xp % _xp_needed;
            _xp_needed = _xp_needed * 1.15f;
            _showLevelUpWindow = true;
        }
    }

    void set_rotation(float angle) {
        _model._transform._rotation.y = angle;
    }

    float get_rotation() const {
        return _model._transform._rotation.y;
    }
    
    void set_position(const glm::vec3& pos) {
        _model._transform._position = pos - _center_offset;  // Ajustamos la posición real
    }

    glm::vec3 get_mouse_world_position() const {
        return _mouse_world_position;
    }

    glm::vec3 get_position() const {
        return _model._transform._position + _center_offset;  // Devolvemos la posición corregida
    }

    float get_radius() const { return _radius; } // Ajusta según tu modelo real

    bool showLevelUpWindow() const { return _showLevelUpWindow; }
    void setShowLevelUpWindow(bool show) { _showLevelUpWindow = show; }

    float _move_speed = 5.0f;
    int _hp = 100;
    int _max_hp = 100;
    float _attack_speed = 1.0f;
    float _bullet_speed = 12.0f;
    float _damage = 10.0f;
    int _piercing_strength = 1;
    float _luck = 1.0f;
    float _crit_damage = 1.5f;
    float _crit_chance = 0.05f;
    int _bullet_type = 0;
    int _xp = 0;
    float _xp_multiplier = 1.0f;
    int _level = 1;
    int _xp_needed = 100.0f;
    float _radius = 0.8f;
    Model _model;
    glm::vec3 _center_offset = glm::vec3(0.0f); // Desplazamiento del centro
    bool _showLevelUpWindow = false;

private:
    glm::vec3 _mouse_world_position = glm::vec3(0.0f);

    void update_movement(float delta_time) {
        glm::vec3 movement(0.0f);
        
        if (Keys::down(SDLK_W)) movement.z += 1.0f;
        if (Keys::down(SDLK_S)) movement.z -= 1.0f;
        if (Keys::down(SDLK_A)) movement.x += 1.0f;
        if (Keys::down(SDLK_D)) movement.x -= 1.0f;
        
        if (Keys::pressed(SDLK_Q)) _hp -= 2; // Debugging
        if (Keys::pressed(SDLK_E)) _xp += 99; // Debugging

        if (glm::length(movement) > 0) {
            movement = glm::normalize(movement);
            _model._transform._position += movement * _move_speed * delta_time;
        }
    }

    glm::vec3 calculate_mouse_world_position(float window_width, float window_height, const glm::mat4& inv_projection, const glm::mat4& inv_view) {
        auto [mouse_x, mouse_y] = Mouse::position();

        // Convertimos la posición del mouse a coordenadas normalizadas (-1 a 1)
        float normalized_x = (2.0f * mouse_x) / window_width - 1.0f;
        float normalized_y = 1.0f - (2.0f * mouse_y) / window_height;
        
        // Posición en espacio de clip
        glm::vec4 clip_space_pos(normalized_x, normalized_y, -1.0f, 1.0f);

        // Convertir a espacio de cámara
        glm::vec4 camera_space_pos = inv_projection * clip_space_pos;
        camera_space_pos /= camera_space_pos.w; // Proyección perspectiva

        // Convertir a espacio mundial
        glm::vec4 world_space_pos = inv_view * camera_space_pos;

        // **Raycasting hasta el suelo (Y = 0)**
        glm::vec3 ray_origin = glm::vec3(inv_view[3]);  // Posición de la cámara en el mundo
        glm::vec3 ray_direction = glm::normalize(glm::vec3(world_space_pos) - ray_origin);

        // Intersección con el plano Y = 0
        float t = -ray_origin.y / ray_direction.y;
        glm::vec3 world_mouse_pos = ray_origin + ray_direction * t;

        return world_mouse_pos;
    }

    void update_rotation(const glm::vec3& world_mouse_pos) {
        // Verificar si la posición del mouse es válida
        if (!std::isfinite(world_mouse_pos.x) || !std::isfinite(world_mouse_pos.z)) {
            return; // Evita rotaciones inválidas
        }
        // Hacer que el modelo apunte hacia la posición del mouse
        _model.look_at(world_mouse_pos);
    }

    glm::mat4 get_inverse_view_matrix(const glm::vec3& camera_pos, const glm::vec3& camera_rot) {
        glm::mat4 view_mat(1.0f);
        view_mat = glm::rotate(view_mat, -camera_rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
        view_mat = glm::rotate(view_mat, -camera_rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
        view_mat = glm::translate(view_mat, -camera_pos);
        return glm::inverse(view_mat);
    }
};
