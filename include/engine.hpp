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

struct Engine {

    void init() {
        Time::init();
        _window.init(1280, 720, "OpenGL Renderer");
        _camera.set_perspective(1280, 720, 70);
        
        // create pipeline for textured objects
        _pipeline.init("../assets/shaders/default.vert", "../assets/shaders/default.frag");
        _pipeline_shadows.init("../assets/shaders/shadows.vert", "../assets/shaders/shadows.frag");
        _pipeline_shadows.create_framebuffer();

        // create light and its shadow map
        _lights[0].init({+1.0, +3.0, -0.5}, {.992, .984, .827}, 100);
        _lights[1].init({+3.0, +1.5, +4.0}, {.992, .984, .827}, 100);
        

        /*_mesh.init();
        _pipeline_color.init("../shaders/color.vert", "../shaders/color.frag");
        _mesh_color.init();
        _transform_color._position.x -= 2.0f;
        _transform_color._position.z -= 5.0f;
        */
        // create renderable models
        _player.init("../assets/models/Shark.obj");

        // create spheres to represent the lights
                for (auto& light: _lights) {
            _models.emplace_back().init(Mesh::eSphere);
            _models.back()._transform._position = light._position;
        }

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
        // Dimensiones de la ventana (ajústalas según tu motor)
        float windowWidth = 1280.0f;
        float windowHeight = 720.0f;
        
        // get mouse position
        auto [mouseX, mouseY] = Mouse::position();

        // Normalizar las coordenadas del mouse
        float normalizedX = (2.0f * mouseX) / windowWidth - 1.0f;
        float normalizedY = 1.0f - (2.0f * mouseY) / windowHeight;
        glm::vec4 clipSpacePos(normalizedX, normalizedY, -1.0f, 1.0f);

        // Convertir al espacio de cámara
        glm::mat4 invProjection = glm::inverse(_camera._projection_mat);
        glm::vec4 cameraSpacePos = invProjection * clipSpacePos;
        cameraSpacePos /= cameraSpacePos.w;

        // Convertir al espacio mundial
        glm::mat4 viewMat(1.0f);
        viewMat = glm::rotate(viewMat, -_camera._rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        viewMat = glm::rotate(viewMat, -_camera._rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        viewMat = glm::translate(viewMat, -_camera._position);
        glm::mat4 invView = glm::inverse(viewMat);

        glm::vec4 worldSpacePos = invView * cameraSpacePos;

        // Calcular la dirección y el ángulo hacia el mouse
        glm::vec3 direction = glm::normalize(glm::vec3(worldSpacePos) - _player._transform._position);
        float angle = std::atan2(direction.y, direction.x);
        if (angle < 0) angle += glm::two_pi<float>(); // Asegurar rango [0, 2π]
        _player._transform._rotation.z = angle;

        // move player
        float speed = 0.08f;
        if (Keys::down(SDLK_W)) _player._transform._position.y += speed;
        if (Keys::down(SDLK_S)) _player._transform._position.y -= speed;
        if (Keys::down(SDLK_A)) _player._transform._position.x -= speed;
        if (Keys::down(SDLK_D)) _player._transform._position.x += speed;
        float rotationSpeed = 0.005f;
        // _camera._rotation.x -= rotationSpeed * Mouse::delta().second;
        // _camera._rotation.y -= rotationSpeed * Mouse::delta().first;
        // Hacer que la cámara siga al cubo
         // Offset para que la cámara esté detrás y arriba
        glm::vec3 offset(0.0f, 0.0f, 10.0f);
        _camera._position = _player._transform._position + offset;
        // let input system process input
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
            glClearColor(0.1, 0.1, 0.1, 0.0);
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
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(_window._window_p);
        Input::flush();
    }

    Window _window;
    Camera _camera;
    Pipeline _pipeline;
    Pipeline _pipeline_shadows;
    std::array<Light, 2> _lights;
    std::vector<Model> _models;
    Model _player;                // Referencia al modelo principal

    // other
    bool _shadows_dirty = true;
    bool _mouse_captured = false;

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
