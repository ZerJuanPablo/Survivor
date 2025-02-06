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
        const float bar_width = 1000.0f;
        const float bar_height = 20.0f;
        
        ImVec2 window_pos = ImVec2((window_width - bar_width) * 0.5f, 20);
        ImVec2 window_size = ImVec2(bar_width, bar_height);  

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(window_size);

        ImGui::Begin("HealthBar", nullptr, 
            ImGuiWindowFlags_NoBackground | 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoSavedSettings);
        
        float hp_percent = static_cast<float>(player._hp) / player._max_hp;
        char overlay[32];
        snprintf(overlay, sizeof(overlay), "HP: %d/%d", player._hp, player._max_hp);

        // Obtener posición de la barra
        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 bar_size = ImVec2(bar_width, bar_height);
        
        // Dibujar barra de progreso
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        ImGui::ProgressBar(hp_percent, bar_size, ""); // Sin texto para evitar doble overlay
        ImGui::PopStyleColor();

        // Dibujar texto centrado en la barra
        ImVec2 text_size = ImGui::CalcTextSize(overlay);
        ImVec2 text_pos = ImVec2(
            cursor_pos.x + (bar_size.x - text_size.x) * 0.5f, // Centrar horizontalmente
            cursor_pos.y + (bar_size.y - text_size.y) * 0.5f  // Centrar verticalmente
        );

        ImGui::GetWindowDrawList()->AddText(text_pos, IM_COL32(255, 255, 255, 255), overlay);

        ImGui::End();
    }

    void show_xp_bar(Player& player, int window_width, int window_height) {
        const float bar_width = 1000.0f;
        const float bar_height = 20.0f;
        
        ImVec2 window_pos = ImVec2((window_width - bar_width) * 0.5f, 50);
        ImVec2 window_size = ImVec2(bar_width, bar_height);

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(window_size);

        ImGui::Begin("XPBar", nullptr, 
            ImGuiWindowFlags_NoBackground | 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoSavedSettings);
        
        float xp_percent = static_cast<float>(player._xp) / player._xp_needed;
        char overlay[32];
        snprintf(overlay, sizeof(overlay), "XP: %d/%d", player._xp, player._xp_needed);

        // Obtener posición de la barra
        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 bar_size = ImVec2(bar_width, bar_height);
        
        // Dibujar barra de progreso
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
        ImGui::ProgressBar(xp_percent, bar_size, ""); // Sin texto para evitar doble overlay
        ImGui::PopStyleColor();

        // Dibujar texto centrado en la barra
        ImVec2 text_size = ImGui::CalcTextSize(overlay);
        ImVec2 text_pos = ImVec2(
            cursor_pos.x + (bar_size.x - text_size.x) * 0.5f, // Centrar horizontalmente
            cursor_pos.y + (bar_size.y - text_size.y) * 0.5f  // Centrar verticalmente
        );

        ImGui::GetWindowDrawList()->AddText(text_pos, IM_COL32(255, 255, 255, 255), overlay);

        ImGui::End();
    }

};