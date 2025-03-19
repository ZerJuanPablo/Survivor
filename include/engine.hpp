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
#include "entities/projectile.hpp"
#include "uiManager.hpp"
#include <glm/gtc/random.hpp>
#include "entities/boss.hpp"
#include "entities/upgrade.hpp"
#include "entities/food.hpp"

struct Engine {

    void init() {
        Time::init();
        _spawn_timer = 0.0f;

        _window.init(width, height, "OpenGL Renderer");
        _camera.set_perspective(width, height, 70);
        glm::vec3 rotation(-glm::radians(70.0f), glm::radians(180.0f), 0.0f);
        _camera._rotation = rotation;


        // create pipeline for textured objects
        _pipeline.init("../assets/shaders/default.vert", "../assets/shaders/default.frag");
        _pipeline_shadows.init("../assets/shaders/shadows.vert", "../assets/shaders/shadows.frag");
        _pipeline_shadows.create_framebuffer();

        // create light and its shadow map
        Light player_light;
        player_light.init({0.0, 0.3, 0.0}, {5.0, 5.0, 5.6}, 350);
        _lights.push_back(player_light);
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
        _terrain.emplace_back().init(Mesh::Wall);
        _terrain[1]._transform._scale = glm::vec3(200.0f, 20.0f, 10.0f);
        _terrain[1]._transform._position = glm::vec3(0.0f, -10.0f, 100.0f);
        _terrain.emplace_back().init(Mesh::Wall);
        _terrain[2]._transform._scale = glm::vec3(10.0f, 20.0f, 200.0f);
        _terrain[2]._transform._position = glm::vec3(-100.0f, -10.0f, 0.0f);
        _terrain.emplace_back().init(Mesh::Wall);
        _terrain[3]._transform._scale = glm::vec3(10.0f, 20.0f, 200.0f);
        _terrain[3]._transform._position = glm::vec3(100.0f, -10.0f, 0.0f);
        _terrain.emplace_back().init(Mesh::Wall);
        _terrain[4]._transform._scale = glm::vec3(200.0f, 2.0f, 10.0f);
        _terrain[4]._transform._position = glm::vec3(0.0f, -5.0f, -100.0f);

        // create initial enemies
        setup_enemy_configs();

        // load models to pool
        load_models_to_pool();

        _boss._state = Enemy::State::DEAD;
        _boss_spawned = false;
        _boss_spawn_timer = 0.0f;

        // true or false to able or disable
        SDL_SetWindowRelativeMouseMode(_window._window_p, false);

        // init ui manager
        _uiManager.init(_window._window_p, _window._context);
    }
    
    void destroy() {
        // destroy audio stuff
        SDL_DestroyAudioStream(audio_stream);
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
        
        if (!_showing_upgrades)
        {
            // Update camera position and rotation
            glm::mat4 inv_view = glm::inverse(_camera.get_view_matrix());

            _player.update(
                delta_time,
                width, height,
                _camera._projection_mat,
                inv_view
        );
        }

        // _enemy.update(delta_time, _player.get_position());
        _camera._position = _player.get_position() + offset;
        _lights[0]._position = _player.get_position() + glm::vec3(0.0f, 1.0f, 1.0f);
        
        //_player._model.look_at(_terrain[0]._transform._position);

        // Process input
        Input::flush();
    }

    void load_models_to_pool() {
        _model_pool["shark"] = Model();
        _model_pool["shark"].init("../assets/models/Shark.obj");
        
        _model_pool["koi"] = Model();
        _model_pool["koi"].init("../assets/models/Koi.obj");

        _model_pool["blob"] = Model();
        _model_pool["blob"].init("../assets/models/Blobfish.obj");
    }

    void create_enemy(EnemyType type, const glm::vec3& position){
        const auto& config = _enemy_configs[type];
    
        _enemies.emplace_back();
        Enemy& new_enemy = _enemies.back();
        
        // Copiar modelo del pool
        new_enemy._model = _model_pool[config.model_key];
        
        // Configurar propiedades
        new_enemy.init_from_config(config, difficulty);
        new_enemy._model._transform._scale = glm::vec3(0.5f);
        new_enemy.set_position(position);
        new_enemy._state = Enemy::State::ALIVE;
    }

    void spawn_boss () {
        play_audio("../assets/audio/jump.wav");
        _boss.init_boss("../assets/models/Anglerfish.obj",
            glm::vec3(3.0f,3.0f,0.0f),
            1.2f,
            20.0f,
            30.0f,
            4.0f);

        _boss._state = Enemy::State::ALIVE;
        _boss_spawned = true;
        
        Light _bosslight;
        _bosslight.init({0.0, 1.0, 0.0}, {4.1f, 4.4f, 4.6f}, 500);
        _lights.push_back(_bosslight);
    }

    void boss_slained() {
        _lights.pop_back();
        play_audio("../assets/audio/big_blob.wav");
        _boss.die();
        _boss_spawned = false;
    }

    float get_rarity_weight(Rarity rarity) {
        switch(rarity) {
            case Rarity::COMMON:    return 70.0f;
            case Rarity::UNCOMMON:  return 25.0f;
            case Rarity::RARE:     return 5.0f;
            default:               return 0.0f;
        }
    }

    Upgrade select_random_upgrade(const std::vector<Upgrade>& upgrades){
        static std::random_device rd;
        static std::mt19937 rng(rd());
        // Sistema de ponderación por rareza
        std::vector<float> weights;
        for(const auto& upgrade : upgrades) {
            weights.push_back(get_rarity_weight(upgrade.rarity));
        }
                
        // Selección aleatoria ponderada
        std::discrete_distribution<> dist(weights.begin(), weights.end());
        return upgrades[dist(rng)];
    }

    void generate_upgrades(){
        _current_upgrades.clear();
        
        static const std::vector<Upgrade> all_upgrades = {
            {"Damage up", "+10% damage", Rarity::COMMON, [](Player& p){ p._damage *= 1.1f; }},
            {"Speed up", "+5% speed", Rarity::COMMON, [](Player& p){ p._move_speed *= 1.05f; }},
            {"Attack speed up", "+10% attack speed up", Rarity::COMMON, [](Player& p){ p._attack_speed *= 1.1f; }},
            {"XP mult", "+0.2 XP mult", Rarity::UNCOMMON, [](Player& p){ p._xp_multiplier += 0.2f; }},
            {"Strong Damage up", "+25% damage", Rarity::UNCOMMON, [](Player& p){ p._damage *= 1.25f; }},
            {"Max HP up", "+50 HP", Rarity::UNCOMMON, [](Player& p){ p._max_hp += 20; }},
            {"Piercing up", "+1 piercing", Rarity::RARE, [](Player& p){ p._piercing_strength += 1; }},
            
        };
        
        // Seleccionar 3 aleatorias con ponderación por rareza
        for(int i = 0; i < 3; ++i) {
            _current_upgrades.push_back(select_random_upgrade(all_upgrades));
        }
    }

    void setup_enemy_configs() {
        _enemy_configs[EnemyType::SHARK] = {
            "shark",    // model_key
            glm::vec3(0.0f),       // center_offset    
            1.1f,       // movement speed
            3.0f,      // max_hp
            10.0f,        // damage
            0.5f       // radius
        };
        
        _enemy_configs[EnemyType::KOI] = {
            "koi",
            glm::vec3(0.0f),
            1.5f,
            1.0f,
            12.0f,
            0.5f
        };

        _enemy_configs[EnemyType::BLOB] = {
            "blob",
            glm::vec3(0.0f),
            0.8f,
            7.0f,
            15.0f,
            0.6f
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
        const float radius = 12.0f;
        
        // Define spawn limits
        const float min_x = -92.0f, max_x = 92.0f;
        const float min_z = -92.0f, max_z = 92.0f;

        
        for (int i = 0; i < difficulty + 1; ++i) {
            const float angle = glm::linearRand(0.0f, glm::two_pi<float>());
            glm::vec3 spawn_pos = player_pos + glm::vec3{
                glm::cos(angle) * radius,
                0.0f,
                glm::sin(angle) * radius
            };
            
            // Ensure the spawn position is within bounds
            spawn_pos.x = glm::clamp(spawn_pos.x, min_x, max_x);
            spawn_pos.z = glm::clamp(spawn_pos.z, min_z, max_z);
            
            // Determina el tipo de enemigo con probabilidades
            const float rand = glm::linearRand(0.0f, 1.0f);
            EnemyType type;

            if (rand < 0.6f) {         // 60% shark
                type = EnemyType::SHARK;
            } 
            else if (rand < 0.85f) {    // 25% koi
                type = EnemyType::KOI;
            } 
            else {                      // 15% blob
                type = EnemyType::BLOB;
            }

            // Crea el enemigo
            create_enemy(type, spawn_pos);
        }
    }

    bool check_sphere_collision (const glm::vec3& pos1, float radius1, const glm::vec3& pos2, float radius2) {
        float distance = glm::distance(pos1, pos2);
        return distance < radius1 + radius2;
    }

    void check_collisions() {

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
                play_audio("../assets/audio/contact.wav");
                enemy.die();
            }
        }

        if (_boss._state == Enemy::State::ALIVE && _boss_spawned) {
            if (check_sphere_collision(player_pos, player_radius,
                                       _boss.get_position(), _boss._radius))
            {
                _player.take_damage(_boss._damage);
                // teleport boss nearby
                _boss.teleport_near_player(_player.get_position());
                play_audio("../assets/audio/contact.wav");
            }
        }

        // Bullet vs Enemies
        for (auto& projectile : _projectiles) {
            if (!projectile.is_active()) continue; // skip inactive bullets
            
            // boss collision
            if (_boss._state == Enemy::State::ALIVE) {
                if (check_sphere_collision(projectile.get_position(), 
                    projectile.get_radius(),
                    _boss.get_position(),
                    _boss._radius)) 
                    {
                    _boss.take_damage(projectile.get_damage(), _player);                
                    play_audio("../assets/audio/hit.wav");
                    projectile._piercing -= 1;
                }
            } 

            for (auto& enemy : _enemies) {
                if (enemy._state == Enemy::State::DEAD) continue;

                // Check collision
                if (check_sphere_collision(projectile.get_position(), 
                                           projectile.get_radius(),
                                           enemy.get_position(),
                                           enemy._radius)) 
                {
                    // Apply damage to enemy
                    enemy.take_damage(projectile.get_damage(), _player);
                    play_audio("../assets/audio/hit.wav");
                    if (enemy._state == Enemy::State::DEAD)
                    {
                        float rand = glm::linearRand(0.0f,1.0f);
                        if (rand < 0.05f)
                        {
                            _foods.emplace_back().init("../assets/models/Worm.obj");
                            Food& new_food = _foods.back();
                            new_food._model._transform._scale = glm::vec3(1.0f);                      
                            new_food.set_position(enemy.get_position());
                            new_food._state = Food::State::ALIVE;
                        }
                    }
                    projectile._piercing -= 1;
                }  
            }
        }

        for (auto& food : _foods) {
            if (food._state == Food::State::DEAD) continue;

            if (check_sphere_collision(player_pos, player_radius, food.get_position(), food._radius))
            {
                _player._hp += food.heal;
                play_audio("../assets/audio/eat.wav");
                food._state = Food::State::DEAD;
            }
        } 
    }

    void play_audio(const char *path){
        SDL_InitSubSystem(SDL_INIT_AUDIO);
        // load .wav file from disk
        SDL_LoadWAV(path, &audio_file.spec, &audio_file.buffer, &audio_file.buffer_size);
        // create an audio stream for default audio device
        audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr, nullptr, nullptr);
        if (audio_stream == nullptr) fmt::println("{}", SDL_GetError());
        // get the format of the device (sample rate and such)
        SDL_AudioSpec device_format;
        SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &device_format, nullptr);
        // set up the audio stream to convert from our .wav file sample rate to the device's sample rate
        if(!SDL_SetAudioStreamFormat(audio_stream, &audio_file.spec, &device_format)) fmt::println("{}", SDL_GetError());
        // load .wav into the audio stream and play
        if(!SDL_PutAudioStreamData(audio_stream, audio_file.buffer, audio_file.buffer_size)) fmt::println("{}", SDL_GetError());
        if(!SDL_ResumeAudioStreamDevice(audio_stream)) fmt::println("{}", SDL_GetError());
    }

    void update_bullets(float delta_time){
        float attack_cooldown = 1.0f / _player._attack_speed;
        time_since_last_shot += delta_time;

        if (time_since_last_shot >= attack_cooldown)
        {    
            time_since_last_shot = 0;
            _projectiles.emplace_back();
            Projectile& new_projectile = _projectiles.back();
            
            glm::vec3 direction = glm::normalize(_player.get_mouse_world_position() - _player.get_position());

            // Configurar propiedades
            new_projectile.init(_player.get_position(), direction, _player._bullet_speed, _player._damage, _player._piercing_strength);
            play_audio("../assets/audio/shot.wav");
        }

        // move all bullets
        for (auto& projectile : _projectiles)
        {
            projectile.update(delta_time);
        }
        for (auto& food : _foods)
        {
            food.update_rotation(delta_time);
        }
    }

    void update_boss(float delta_time) {
        if (_boss._state == Enemy::State::ALIVE && _boss_spawned) {
            _boss.update(delta_time, _player);
            // Obtener la posición actual del boss
            glm::vec3 boss_position = _boss.get_position();

            // Calcular la dirección hacia adelante basado en la rotación Y del boss
            float angle = _boss._model._transform._rotation.y;  // Rotación en radianes
            glm::vec3 forward = glm::vec3(glm::sin(angle), 0.0f, glm::cos(angle));

            // Calcular nueva posición de la luz
            glm::vec3 light_offset = forward * 3.f; // Mueve la luz 1.5 unidades al frente
            light_offset.y += 1.5f; // Mantiene la luz un poco arriba

            // Asignar la nueva posición de la luz
            _lights[1]._position = boss_position + light_offset;
        }
        else {
            _boss_spawn_timer += delta_time;
            if (_boss_spawn_timer >= _boss_spawn_cooldown) {
                spawn_boss();
                _boss_spawn_timer = 0.0f;
            }
        }
    }

    void execute_frame() {
        if (!_showing_upgrades) {
            // update time for accurate Time:get_delta()
            Time::update();
            float delta_time = Time::get_delta();
            
            // update input
            execute_input();
            
            _game_timer += delta_time;
            update_spawning(delta_time);
            update_boss(delta_time);

            //increase difficulty
            _difficulty_timer += delta_time;
            if (_difficulty_timer >= _difficulty_interval){
                difficulty++;
                _difficulty_timer = 0;
            }


            // Actualizar enemigos
            for (auto& enemy : _enemies) {
                enemy.update(Time::get_delta(), _player);
            }
    
            update_bullets(delta_time);
            check_collisions();
        }

        // Eliminar enemigos muertos
        std::erase_if(_enemies, [](const Enemy& enemy) {
            return enemy._state == Enemy::State::DEAD;
        });

        std::erase_if(_projectiles, [](const Projectile& projectile) {
            return !projectile.is_active();
        });

        std::erase_if(_foods, [](const Food& food) {
            return food._state == Food::State::DEAD;
        });
        // draw shadows
        if (_shadows_dirty) {
            // do this for each light
            for (auto& light: _lights) {
                _pipeline_shadows.bind();
                glUniform1f(0, Time::get_total());
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
                    if (_boss_spawned && _boss._state == Enemy::State::ALIVE) {
                        _boss.draw(false);
                    }
                    for (auto& food: _foods) food.draw(false);
                }
            }
            _shadows_dirty = false;
        }

        // draw color
        {
            // bind pipeline
            _pipeline.bind();
            glUniform1f(0, 0);
            glViewport(0, 0, 1280, 720);
            // clear screen before drawing
            glClearColor(0.08627451f, 0.19607843f, 0.35686275f, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // bind lights and their shadow maps
            for (int i = 0; i < _lights.size(); i++) {
                _lights[i].bind_read(i + 1, i * 3);
            }
            for (auto& model: _terrain) model.draw(false);
            glUniform1f(0, Time::get_total());
            _camera.bind();
            // draw the stuff
            _player.draw(false);
            for (auto& enemy: _enemies) enemy.draw(false);
            for (auto& food: _foods) food.draw(false);
            for (auto& projectile : _projectiles) {
                projectile.draw(false);
            }
            if (_boss_spawned && _boss._state == Enemy::State::ALIVE)
            {
                _boss.draw(false);
            }
        }
        
        bool level_up_triggered = _player.showLevelUpWindow();
        if (!_showing_upgrades && level_up_triggered) {
            generate_upgrades();
            _showing_upgrades = true;
        }
        _showing_upgrades = _uiManager.render(_player, width, height, _showing_upgrades, _current_upgrades, _game_timer);

        SDL_GL_SwapWindow(_window._window_p);
        Input::flush();
    }

    Window _window;
    Camera _camera;
    Pipeline _pipeline;
    Pipeline _pipeline_shadows;
    std::vector<Light> _lights;
    std::vector<Model> _terrain;
    Player _player;
    Boss _boss;
    Model _floor;
    std::vector<Enemy> _enemies;
    std::vector<Projectile> _projectiles;
    std::vector<Food> _foods; 
    UIManager _uiManager;
    //Enemy _enemy;
    glm::vec3 offset = glm::vec3(-0.5f, 19.0f, -7.0f); // Offset con un desplazamiento en X
    
    std::unordered_map<std::string, Model> _model_pool;
    std::unordered_map<EnemyType, EnemyConfig> _enemy_configs;

    // game increase difficulty
    float _spawn_timer;
    const float _spawn_time = 3.0f;
    float time_since_last_shot = 0.0f;
    float _boss_spawn_timer = 0.0f;
    const float _boss_spawn_cooldown = 60.0f;
    bool _boss_spawned = false;
    std::vector<Upgrade> _current_upgrades;
    bool _showing_upgrades = false;
    float _game_timer = 0.0f;
    float _difficulty_timer = 0.0f;    // Tracks time since last difficulty increase
    const float _difficulty_interval = 20.0f; // Time interval (1 min) to increase difficulty
    int difficulty = 1; 

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

