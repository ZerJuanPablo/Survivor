// UIManager.h
#pragma once
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "entities/player.hpp"

class UIManager {
public:
    void init(SDL_Window* window, SDL_GLContext context) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        
        // Configurar estilo
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 3.0f;
        
        // Inicializar backends
        ImGui_ImplSDL3_InitForOpenGL(window, context);
        ImGui_ImplOpenGL3_Init("#version 460 core");
    }

    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void render(Player& player, int window_width, int window_height) {
        start_frame();
        show_fps_window();
        show_health_bar(player, window_width, window_height);
        show_xp_bar(player, window_width, window_height);
        end_frame();
    }

private:
    void start_frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void end_frame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void show_fps_window() {
        ImGui::Begin("FPS", nullptr, 
            ImGuiWindowFlags_NoBackground | 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::End();
    }

    void show_health_bar(Player& player, int window_width, int window_height) {
        const float bar_width = 300.0f;
        const float bar_height = 30.0f;
        
        ImGui::SetNextWindowPos(ImVec2(
            (window_width - bar_width) * 0.5f,
            20
        ), ImGuiCond_Always);
        
        ImGui::Begin("HealthBar", nullptr, 
            ImGuiWindowFlags_NoBackground | 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoSavedSettings);
        
        float hp_percent = static_cast<float>(player._hp) / player._max_hp;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        ImGui::ProgressBar(hp_percent, ImVec2(bar_width, bar_height), "");
        ImGui::PopStyleColor();
        
        // Texto centrado
        ImVec2 text_size = ImGui::CalcTextSize("HP: 9999/9999");
        ImGui::SetCursorPos(ImVec2(
            (bar_width - text_size.x) * 0.5f,
            (bar_height - text_size.y) * 0.5f
        ));
        ImGui::Text("HP: %d/%d", player._hp, player._max_hp);
        
        ImGui::End();
    }

    void show_xp_bar(Player& player, int window_width, int window_height) {
        const float bar_width = 300.0f;
        const float bar_height = 20.0f;
        
        ImGui::SetNextWindowPos(ImVec2(
            (window_width - bar_width) * 0.5f,
            60  // 20 (health bar) + 30 (height) + 10 (spacing)
        ), ImGuiCond_Always);
        
        ImGui::Begin("XPBar", nullptr, 
            ImGuiWindowFlags_NoBackground | 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoSavedSettings);
        
        float xp_percent = static_cast<float>(player._xp) / player._xp_needed;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
        ImGui::ProgressBar(xp_percent, ImVec2(bar_width, bar_height), "");
        ImGui::PopStyleColor();
        
        // Texto centrado
        ImVec2 text_size = ImGui::CalcTextSize("XP: 9999/9999");
        ImGui::SetCursorPos(ImVec2(
            (bar_width - text_size.x) * 0.5f,
            (bar_height - text_size.y) * 0.5f
        ));
        ImGui::Text("XP: %d/%d", player._xp, player._xp_needed);
        
        ImGui::End();
    }
};