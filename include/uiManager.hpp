// UIManager.h
#pragma once
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "entities/player.hpp"
#include "entities/upgrade.hpp"
#include "state.hpp"

class UIManager {
public:
    void init(SDL_Window* window, SDL_GLContext context) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 3.0f;
        
        ImGui_ImplSDL3_InitForOpenGL(window, context);
        ImGui_ImplOpenGL3_Init("#version 460 core");
    }

    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
    bool render(Player& player, int window_width, int window_height, bool showing_upgrades, const std::vector<Upgrade> upgrades, float time) {
        start_frame();
        show_fps_window();
        show_health_bar(player, window_width, window_height);
        show_xp_bar(player, window_width, window_height);
        show_timer(time);
        
        if(showing_upgrades) {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            ImGui::SetNextWindowFocus(); 
            ImGui::SetNextWindowPos(ImVec2(window_width/2.0f, window_height/2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Choose an upgrade", nullptr, 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
            
            for(int i = 0; i < upgrades.size(); ++i) {
                auto& upgrade = upgrades[i];
                ImGui::PushID(i);
                
                if(ImGui::Button(upgrade.name.c_str(), ImVec2(200, 50))) {
                    upgrade.apply(player);
                    showing_upgrades = false;
                    player.setShowLevelUpWindow(false);
                }
                
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", upgrade.description.c_str());
                }
                
                ImGui::PopID();
            }
            
            ImGui::End();
        }
        
        end_frame();
        return showing_upgrades;
    }

    void render_main_menu(GameState& gameState, int window_width, int window_height) {
        start_frame();
    
        ImGui::SetNextWindowPos(ImVec2(window_width / 2 - 200, window_height / 3 - 50));
        ImGui::SetNextWindowSize(ImVec2(315, 350));  // Aumenté la altura para el nuevo texto
    
        ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
        // Título del juego
        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("REEF SURVIVOR");
        ImGui::SetWindowFontScale(1.0f);
    
        ImGui::Spacing();
        
        // Texto explicativo
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));  // Color amarillo
        ImGui::SetWindowFontScale(1.3f);
        ImGui::TextWrapped("Survive 10 minutes\nto win!");
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
    
        ImGui::Spacing();
        ImGui::Spacing();
    
        // Botones
        if (ImGui::Button("Start Game", ImVec2(300, 60))) {
            gameState = GameState::RESET;
        }
    
        ImGui::Spacing();
    
        if (ImGui::Button("Exit", ImVec2(300, 60))) {
            gameState = GameState::EXIT;
        }
    
        ImGui::End();
    
        end_frame();
    }

    void render_over_menu(GameState& gameState, int window_width, int window_height) {
        start_frame();
    
        ImGui::SetNextWindowPos(ImVec2(window_width / 2 - 200, window_height / 3 - 50));
        ImGui::SetNextWindowSize(ImVec2(315, 300));

        ImGui::Begin("Game Over Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("Game Over");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::Button("Restart", ImVec2(300, 60))) {
            gameState = GameState::RESET;
        }
        ImGui::Spacing();

        if (ImGui::Button("Exit", ImVec2(300, 60))) {
            gameState = GameState::EXIT;
        }
        
        ImGui::End();

        end_frame();
    }

    void render_win_menu(GameState& gameState, int window_width, int window_height) {
        start_frame();
    
        ImGui::SetNextWindowPos(ImVec2(window_width / 2 - 200, window_height / 3 - 50));
        ImGui::SetNextWindowSize(ImVec2(315, 300));

        ImGui::Begin("Win", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("Congratulations you survived 10min");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::Button("Restart", ImVec2(300, 60))) {
            gameState = GameState::RESET;
        }
        ImGui::Spacing();

        if (ImGui::Button("Exit", ImVec2(300, 60))) {
            gameState = GameState::EXIT;
        }
        
        ImGui::End();

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
        ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_Always);
        ImGui::Begin("FPS", nullptr, 
            ImGuiWindowFlags_NoBackground | 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
        ImGui::End();
    }

    void show_timer(float elapsed_time) {
        ImGui::SetNextWindowPos(ImVec2(30, 50), ImGuiCond_Always);
        ImGui::Begin("Survival Time", nullptr, 
            ImGuiWindowFlags_NoBackground | 
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_AlwaysAutoResize);
    
        int minutes = static_cast<int>(elapsed_time) / 60;
        int seconds = static_cast<int>(elapsed_time) % 60;
    
        ImGui::Text("Time: %02d:%02d", minutes, seconds);
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

        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 bar_size = ImVec2(bar_width, bar_height);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        ImGui::ProgressBar(hp_percent, bar_size, ""); 
        ImGui::PopStyleColor();

        
        ImVec2 text_size = ImGui::CalcTextSize(overlay);
        ImVec2 text_pos = ImVec2(
            cursor_pos.x + (bar_size.x - text_size.x) * 0.5f, 
            cursor_pos.y + (bar_size.y - text_size.y) * 0.5f  
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

        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 bar_size = ImVec2(bar_width, bar_height);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
        ImGui::ProgressBar(xp_percent, bar_size, ""); 
        ImGui::PopStyleColor();

        ImVec2 text_size = ImGui::CalcTextSize(overlay);
        ImVec2 text_pos = ImVec2(
            cursor_pos.x + (bar_size.x - text_size.x) * 0.5f, 
            cursor_pos.y + (bar_size.y - text_size.y) * 0.5f  
        );

        ImGui::GetWindowDrawList()->AddText(text_pos, IM_COL32(255, 255, 255, 255), overlay);

        ImGui::End();
    }

};