#pragma once
#include <SDL3/SDL_init.h>
#include <glm/glm.hpp>
#include <fmt/base.h>
#include "window.hpp"
#include "input.hpp"
#include "mesh.hpp"
#include "color_mesh.hpp"
#include "pipeline.hpp"
#include "transform.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"



struct Engine {
    void init() {
        _window.init(1280, 720, "OpenGL Renderer");
        _pipeline.init("../shaders/default.vert", "../shaders/default.frag");
        _transform._position.z -= 5.0f;
        _mesh.init();
        _camera.set_perspective(1280, 720, 70);
        _texture.init("../textures/grass.png");


        _pipeline_color.init("../shaders/color.vert", "../shaders/color.frag");
        _mesh_color.init();
        _transform_color._position.x -= 2.0f;
        _transform_color._position.z -= 5.0f;

        // true or false to able or disable
        SDL_SetWindowRelativeMouseMode(_window._window_p, false);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer bindings
        ImGui_ImplSDL3_InitForOpenGL(_window._window_p, _window._context);
        ImGui_ImplOpenGL3_Init("#version 460 core");
    }
    void destroy() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        _pipeline.destroy();
        _mesh.destroy();
        _texture.destroy();

        _mesh_color.destroy();
        _pipeline_color.destroy();
    
        _window.destroy();
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

    void execute_frame() {

            // Dimensiones de la ventana (ajústalas según tu motor)
        float windowWidth = 1280.0f;
        float windowHeight = 720.0f;

        // Obtener la posición del mouse
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
        glm::vec3 direction = glm::normalize(glm::vec3(worldSpacePos) - _transform_color._position);
        float angle = std::atan2(direction.y, direction.x);
        if (angle < 0) angle += glm::two_pi<float>(); // Asegurar rango [0, 2π]

        // Aplicar la rotación al cubo
        _transform_color._rotation.z = angle;


        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow(); // Show demo window! :)
        ImGui::Begin("FPS window");
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Begin("Debug Info");
        ImGui::Text("Mouse Position (Screen): (%.2f, %.2f)", mouseX, mouseY);
        ImGui::Text("Normalized Mouse Position (Clip Space): (%.2f, %.2f, %.2f, %.2f)", clipSpacePos.x, clipSpacePos.y, clipSpacePos.z, clipSpacePos.w);
        ImGui::Text("Mouse Position (Camera Space): (%.2f, %.2f, %.2f, %.2f)", cameraSpacePos.x, cameraSpacePos.y, cameraSpacePos.z, cameraSpacePos.w);
        ImGui::Text("Mouse Position (World Space): (%.2f, %.2f, %.2f)", worldSpacePos.x, worldSpacePos.y, worldSpacePos.z);
        ImGui::Text("Cube Position: (%.2f, %.2f, %.2f)", _transform_color._position.x, _transform_color._position.y, _transform_color._position.z);
        ImGui::Text("Direction Vector: (%.2f, %.2f, %.2f)", direction.x, direction.y, direction.z);
        ImGui::Text("Calculated Angle (Radians): %.2f", angle);
        ImGui::End();

        // clear screen before drawing
        glClearColor(0.1, 0.1, 0.1, 0.0); // theoretically only needs to be set once
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // bind pipeline and draw geometry
        _pipeline.bind();
        _transform._rotation += 0.001f;
        _transform.bind();
        // move camera
        float speed = 0.08f;
        if (Keys::down(SDLK_W)) _transform_color._position.y += speed;
        if (Keys::down(SDLK_S)) _transform_color._position.y -= speed;
        if (Keys::down(SDLK_A)) _transform_color._position.x -= speed;
        if (Keys::down(SDLK_D)) _transform_color._position.x += speed;
        float rotationSpeed = 0.005f;
        // _camera._rotation.x -= rotationSpeed * Mouse::delta().second;
        // _camera._rotation.y -= rotationSpeed * Mouse::delta().first;
        // Hacer que la cámara siga al cubo
         // Offset para que la cámara esté detrás y arriba
        glm::vec3 offset(0.0f, 0.0f, 10.0f);
        _camera._position = _transform_color._position + offset;

        // bind and draw mesh
        _texture.bind();
        _mesh.draw();
        _camera.bind();

        _pipeline_color.bind();
        _transform_color.bind();
        _camera.bind();
        _mesh_color.draw();


        // present to the screen

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(_window._window_p);
        Input::flush();
    }

    Window _window;
    Pipeline _pipeline;
    Camera _camera;
    Transform _transform;
    Texture _texture;
    Mesh _mesh;

    //second cube
    Pipeline _pipeline_color;
    Color_mesh _mesh_color;
    Transform _transform_color;
};
