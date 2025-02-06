#pragma once
#include <array>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_audio.h>
#include <glm/glm.hpp>
#include <fmt/base.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "time.hpp"
#include "window.hpp"
#include "input.hpp"
#include "pipeline.hpp"
#include "entities/camera.hpp"
#include "entities/model.hpp"
#include "entities/light.hpp"
#include "entities/player.hpp"
#include "entities/enemy.hpp"
#include "uiManager.hpp"
#include "pipeline2.hpp"
#include <random>

/*
Lo último que hice:
Estaba haciendo Instancing de enemigos/balas, pero no funciona. Al parecer el objeto sí existe, pero no es visible.
Si se espera lo suficiente, el objeto hace contacto con el jugador y hace daño, pero no se ve.
*/

struct Engine {

    void init() {
        Time::init();

        _window.init(width, height, "OpenGL Renderer");
        _camera.set_perspective(width, height, 70);
        glm::vec3 rotation(-glm::radians(90.0f), glm::radians(180.0f), 0.0f);
        _camera._rotation = rotation;


        // create pipeline for textured objects
        _pipeline.init("../assets/shaders/default.vert", "../assets/shaders/default.frag");
        _pipeline2.init("../assets/shaders/instancing.vert", "../assets/shaders/instancing.frag", true);
        _pipeline_shadows.init("../assets/shaders/shadows.vert", "../assets/shaders/shadows.frag");
        _pipeline_shadows.create_framebuffer();

        // create light and its shadow map
        _lights[0].init({0.0, 2.0, 0.0}, {3.0, 3.0, 3.0}, 600);
        //_lights[1].init({+3.0, +1.5, +4.0}, {.992, .984, .827}, 100);
        
        // create players
        _player.init("../assets/models/Goldfish.obj");
        _player._model._transform._scale = glm::vec3(0.5f);

        // create floor and walls
        // 0 - floor
        // 1 - top wall
        // 2 - left wall
        // 3 - right wall
        // 4 - bottom wall
        _models.emplace_back().init(Mesh::eCube);
        _models[0]._transform._scale = glm::vec3(200.0f, 0.1f, 200.0f);
        _models[0]._transform._position = glm::vec3(0.0f, -5.0f, 0.0f);
        // TO DO: WALLS




        // create initial enemies
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist_x(-10.0f, 10.0f);
        std::uniform_real_distribution<float> dist_z(-10.0f, 10.0f);
        
        for (int i = 0; i < 10; i++) {
            _enemies.emplace_back().init("../assets/models/Shark.obj");
            _enemies[i]._model._transform._position = glm::vec3(dist_x(gen), 0.0f, dist_z(gen));  
        }
        // create cube in center of scene

        // to delete enemies:
        // _enemy.erase(_enemy.begin()+5);
        // audio stuff

        // true or false to able or disable
        SDL_SetWindowRelativeMouseMode(_window._window_p, false);

        // init ui manager
        _uiManager.init(_window._window_p, _window._context);
    }
    
    void destroy() {
        // destroy audio stuff
        SDL_DestroyAudioStream (audio_stream);
        SDL_free(audio_file.buffer);

        // free OpenGL resources
        for (auto& light: _lights) light.destroy();
        for (auto& model: _models) model.destroy();
        _player.destroy();
        for (auto& enemy: _enemies) enemy.destroy();
        _pipeline.destroy();
        _window.destroy();
        
        // shut down ImGui
        _uiManager.shutdown();
    }
    
    auto execute_event(SDL_Event* event_p) -> SDL_AppResult {
        
        ImGui_ImplSDL3_ProcessEvent(event_p); // Forward your event to backend
        
        // let input system process event
        Input::register_event(*event_p);
        switch (event_p->type) {
            case SDL_EventType::SDL_EVENT_QUIT: return SDL_AppResult::SDL_APP_SUCCESS;
            default: break;
        }
        return SDL_AppResult::SDL_APP_CONTINUE;   
    }

    void execute_input() {
    float delta_time = Time::get_delta();
    
    // Update camera position and rotation
    glm::mat4 inv_view = glm::inverse(_camera.get_view_matrix());

    _player.update(
        delta_time,
        width, height,
        _camera._projection_mat,
        inv_view
    );

    // _enemy.update(delta_time, _player.get_position());
    _camera._position = _player.get_position() + offset;
    _lights[0]._position = _player.get_position() + glm::vec3(0.0f, 2.0f, 0.0f);
    
    //_player._model.look_at(_models[0]._transform._position);

    // Process input
    Input::flush();
}

    void execute_frame() {
        // update time for accurate Time:get_delta()
        Time::update();

        // update input
        execute_input();
        // Actualizar enemigos
        for (auto& enemy : _enemies) {
            enemy.update(Time::get_delta(), _player);
        }

        // Eliminar enemigos muertos
        std::erase_if(_enemies, [](const Enemy& enemy) {
            return enemy._state == Enemy::State::DEAD;
        });

        if (!_enemies.empty()) {
            _pipeline2.bind();
            std::vector<glm::mat4> enemy_transforms;
            enemy_transforms.reserve(_enemies.size());

            for (const auto& enemy : _enemies) {
                enemy_transforms.push_back(enemy._model._transform.get_matrix());
            }

            _pipeline2.update_instance_data(enemy_transforms);

            for (const auto& mesh : _enemies[0]._model._meshes) {
                _pipeline2.draw(mesh._vertex_array_object, mesh._index_count, _enemies.size());
            }
        }
        

        // draw shadows
        if (_shadows_dirty) {
            // do this for each light
            for (auto& light: _lights) {
                _pipeline_shadows.bind();
                glViewport(0, 0, light._shadow_width, light._shadow_height);
                // render into each cubemap face
                for (int face = 0; face < 6; face++) {
                    // bind the target shadow map and clear it
                    light.bind_write(_pipeline_shadows._framebuffer, face);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    // draw the stuff
                    for (auto& model: _models) model.draw(false);
                    _player.draw(false);
                    for (auto& enemy: _enemies) enemy.draw(false);
                }
            }
            _shadows_dirty = false;
        }

        // draw color
        {
            // bind pipeline
            _pipeline.bind();
            glViewport(0, 0, 1280, 720);
            // clear screen before drawing
            glClearColor(0.08627451f, 0.19607843f, 0.35686275f, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // bind lights and their shadow maps
            for (int i = 0; i < _lights.size(); i++) {
                _lights[i].bind_read(i + 1, i * 3);
            }
            _camera.bind();
            // draw the stuff
            for (auto& model: _models) model.draw(false);
            _player.draw(false);

        }
        
        // present to the screen
        _uiManager.render(_player, width, height);

        SDL_GL_SwapWindow(_window._window_p);
        Input::flush();
    }

    Window _window;
    Camera _camera;
    Pipeline _pipeline;
    Pipeline _pipeline_shadows;
    Pipeline2 _pipeline2;
    std::array<Light, 1> _lights;
    std::vector<Model> _models;
    Player _player;
    Model _floor;
    std::vector<Enemy> _enemies;
    UIManager _uiManager;
    //Enemy _enemy;
    glm::vec3 offset = glm::vec3(-0.5f, 12.0f, 0.0f); // Offset con un desplazamiento en X
    

    // other
    bool _shadows_dirty = true;
    bool _mouse_captured = false;
    int width = 1280;
    int height = 720;

    // audio 
    struct AudioFile {
        void init() {}
        void destroy() {}
        SDL_AudioSpec spec;
        Uint8* buffer;
        Uint32 buffer_size;
    };

    AudioFile audio_file;
    SDL_AudioStream* audio_stream;
};
