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

struct Engine {

    void init() {
        Time::init();

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
        

        /*_mesh.init();
        _pipeline_color.init("../shaders/color.vert", "../shaders/color.frag");
        _mesh_color.init();
        _transform_color._position.x -= 2.0f;
        _transform_color._position.z -= 5.0f;
        */
        // create renderable models
        _player.init("../assets/models/Goldfish.obj");
        _player._model._transform._scale = glm::vec3(0.5f);

        _enemy.init("../assets/models/Shark.obj", _player.get_position());
        _enemy._model._transform._position = glm::vec3(3.0f, 0.0f, 2.0f);
        // create renderable models
        _models.emplace_back().init(Mesh::eCube);
        

        // audio stuff

        // true or false to able or disable
        SDL_SetWindowRelativeMouseMode(_window._window_p, false);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer bindings
        ImGui_ImplSDL3_InitForOpenGL(_window._window_p, _window._context);
        ImGui_ImplOpenGL3_Init("#version 460 core");
    }
    
    void destroy() {
        // destroy audio stuff
        SDL_DestroyAudioStream (audio_stream);
        SDL_free(audio_file.buffer);

        // free OpenGL resources
        for (auto& light: _lights) light.destroy();
        for (auto& model: _models) model.destroy();
        _player.destroy();
        _enemy.destroy();
        _pipeline.destroy();
        _window.destroy();
        
        // shut down ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
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

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow(); // Show demo window! :)
        ImGui::Begin("FPS window");
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::End();

        // update input
        execute_input();
        _enemy.update(Time::get_delta(), _player.get_position());

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
                    _enemy.draw(false);
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
            _enemy.draw(false);
        }
        
        // present to the screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(_window._window_p);
        Input::flush();
    }

    Window _window;
    Camera _camera;
    Pipeline _pipeline;
    Pipeline _pipeline_shadows;
    std::array<Light, 1> _lights;
    std::vector<Model> _models;
    Player _player;
    Enemy _enemy;
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
