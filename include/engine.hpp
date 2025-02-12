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
#include <glm/gtc/random.hpp>

struct Engine {

    void init() {
        Time::init();
        _spawn_timer = 0.0f;

        _window.init(width, height, "OpenGL Renderer");
        _camera.set_perspective(width, height, 70);
        glm::vec3 rotation(-glm::radians(90.0f), glm::radians(180.0f), 0.0f);
        _camera._rotation = rotation;


        // create pipeline for textured objects
        _pipeline.init("../assets/shaders/default.vert", "../assets/shaders/default.frag");
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
        _terrain.emplace_back().init(Mesh::eCube);
        _terrain[0]._transform._scale = glm::vec3(200.0f, 0.1f, 200.0f);
        _terrain[0]._transform._position = glm::vec3(0.0f, -5.0f, 0.0f);
        // TO DO: WALLS


        // create initial enemies
        setup_enemy_configs();

        // load models to pool
        load_models_to_pool();

        _enemies.emplace_back().init("../assets/models/Shark.obj");
        _enemies[0]._model._transform._position = glm::vec3(3.0f, 0.0f, 3.0f);  
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
        for (auto& terrain: _terrain) terrain.destroy();
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
    
    //_player._model.look_at(_terrain[0]._transform._position);

    // Process input
    Input::flush();
    }

    void load_models_to_pool() {
        _model_pool["shark"] = Model();
        _model_pool["shark"].init("../assets/models/Shark.obj");
        
        _model_pool["koi"] = Model();
        _model_pool["koi"].init("../assets/models/Koi.obj");

        _model_pool["angler"] = Model();
        _model_pool["angler"].init("../assets/models/Anglerfish.obj");
    }

    void create_enemy(EnemyType type, const glm::vec3& position){
        const auto& config = _enemy_configs[type];
    
        _enemies.emplace_back();
        Enemy& new_enemy = _enemies.back();
        
        // Copiar modelo del pool
        new_enemy._model = _model_pool[config.model_key];
        
        // Configurar propiedades
        new_enemy.init_from_config(config);
        new_enemy._model._transform._scale = glm::vec3(0.5f);
        new_enemy.set_position(position);
        new_enemy._state = Enemy::State::ALIVE;
    }

    void setup_enemy_configs() {
        _enemy_configs[EnemyType::SHARK] = {
            "shark",    // model_key
            glm::vec3(0.0f),       // center_offset    
            0.9f,       // movement speed
            10.0f,      // max_hp
            1.0f,        // damage
            0.5f       // radius
        };
        
        _enemy_configs[EnemyType::KOI] = {
            "koi",
            glm::vec3(0.0f),
            1.3f,
            5.0f,
            1.0f,
            0.3f
        };

        _enemy_configs[EnemyType::ANGLER] = {
            "angler",
            glm::vec3(0.0f),
            0.5f,
            20.0f,
            5.0f,
            0.8f
        };
    }

    void update_spawning(float delta_time) {
        _spawn_timer += delta_time;
        
        if(_spawn_timer >= _spawn_time) {
            spawn_enemy_around_player();
            _spawn_timer = 0.0f;
        }
    }

    void spawn_enemy_around_player() {
        const glm::vec3 player_pos = _player.get_position();
        const float angle = glm::linearRand(0.0f, glm::two_pi<float>());
        const float radius = 5.0f;
        
        glm::vec3 spawn_pos = player_pos + glm::vec3{
            glm::cos(angle) * radius,
            0.0f,
            glm::sin(angle) * radius
        };
        
        // Probabilidades de spawn
        float rand = glm::linearRand(0.0f, 1.0f);
        EnemyType type = EnemyType::SHARK;  // Por defecto
        
        if(rand < 0.6f) {       // 60% shark
            type = EnemyType::SHARK;
        } 
        else if(rand < 0.85f) { // 25% koi (60-85)
            type = EnemyType::KOI;
        } 
        else {                  // 15% angler (85-100)
            type = EnemyType::ANGLER;
        }
        
        create_enemy(type, spawn_pos);
    }

    bool check_sphere_collision (const glm::vec3& pos1, float radius1, const glm::vec3& pos2, float radius2) {
        float distance = glm::distance(pos1, pos2);
        return distance < radius1 + radius2;
    }

    void check_collisions() {

        /*

        FOR FUTURE OPTIMIZATION:

        // Pseudocódigo para spatial grid
        void Engine::check_collisions() {
        // 1. Crear grid espacial
        SpatialGrid grid;
        
        // 2. Insertar todas las entidades colisionables
        for (auto& enemy : _enemies) grid.insert(enemy);
        for (auto& bullet : _bullets) grid.insert(bullet);
        for (auto& xp : _xp_orbs) grid.insert(xp);
        
        // 3. Solo checkear colisiones en celdas adyacentes
        grid.check_collisions([this](auto& a, auto& b) {
            // Manejar colisión
        });
    }
        */


        // Player vs Enemies
        const glm::vec3 player_pos = _player.get_position();
        const float player_radius = _player._radius;
        
        for (auto& enemy : _enemies) {
            if (enemy._state == Enemy::State::DEAD) continue;
            
            const glm::vec3 enemy_pos = enemy.get_position();
            const float enemy_radius = enemy._radius;
            
            if (check_sphere_collision(player_pos, player_radius, 
                                      enemy_pos, enemy_radius)) {
                // Daño recíproco
                _player.take_damage(enemy._damage);
                enemy.die();
            }
        }
    }

    void execute_frame() {
        // update time for accurate Time:get_delta()
        Time::update();
        float delta_time = Time::get_delta();

        // update input
        execute_input();

        update_spawning(delta_time);

        // Actualizar enemigos
        for (auto& enemy : _enemies) {
            enemy.update(Time::get_delta(), _player);
        }

        check_collisions();

        // Eliminar enemigos muertos
        std::erase_if(_enemies, [](const Enemy& enemy) {
            return enemy._state == Enemy::State::DEAD;
        });

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
                    for (auto& model: _terrain) model.draw(false);
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
            for (auto& model: _terrain) model.draw(false);
            _player.draw(false);
            for (auto& enemy: _enemies) enemy.draw(false);
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
    std::array<Light, 1> _lights;
    std::vector<Model> _terrain;
    Player _player;
    Model _floor;
    std::vector<Enemy> _enemies;
    UIManager _uiManager;
    //Enemy _enemy;
    glm::vec3 offset = glm::vec3(-0.5f, 12.0f, 0.0f); // Offset con un desplazamiento en X
    
    std::unordered_map<std::string, Model> _model_pool;
    std::unordered_map<EnemyType, EnemyConfig> _enemy_configs;

    // game increase difficulty
    float _spawn_timer;
    const float _spawn_time = 1.0f;

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
