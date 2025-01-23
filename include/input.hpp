#pragma once
#include <cctype>
#include <set>
#include <cstdint>
#include <SDL3/SDL_events.h>
// conditional includes in case imgui is available
#if __has_include(<imgui.h>)
#   include <imgui.h>
#   define IMGUI_CAPTURE_KBD ImGui::GetIO().WantCaptureKeyboard
#   define IMGUI_CAPTURE_MOUSE ImGui::GetIO().WantCaptureMouse
#else
#   define IMGUI_CAPTURE_KBD 0
#   define IMGUI_CAPTURE_MOUSE 0
#endif

namespace Input {
	// data storage for internal use only
	struct Data {
		auto static get() noexcept -> Data& { 
            static Data instance;
            return instance;
        }
		std::set<SDL_Keycode> keys_pressed, keys_down, keys_released;
		std::set<uint8_t> buttons_pressed, buttons_down, buttons_released;
		float x, y;
		float dx, dy;
		bool mouse_captured;
	};

	struct Keys {
		// check if key was pressed in this frame
		bool static inline pressed(char character) noexcept { return Data::get().keys_pressed.contains(std::tolower(character)); }
		// check if key was pressed in this frame
		bool static inline pressed(SDL_Keycode code) noexcept { return Data::get().keys_pressed.contains(code); }
		// check if key is being held down
		bool static inline down(char character) noexcept { return Data::get().keys_down.contains(std::tolower(character)); }
		// check if key is being held down
		bool static inline down(SDL_Keycode code) noexcept { return Data::get().keys_down.contains(code); }
		// check if key was released in this frame
		bool static inline released(char character) noexcept { return Data::get().keys_released.contains(std::tolower(character)); }
		// check if key was released in this frame
		bool static inline released(SDL_Keycode code) noexcept { return Data::get().keys_released.contains(code); }
    };
	struct Mouse {
		// button ids to make it easier to use
		struct ids { static constexpr uint8_t left = SDL_BUTTON_LEFT, right = SDL_BUTTON_RIGHT, middle = SDL_BUTTON_MIDDLE; };
		// check if mouse button was pressed in this frame
		bool static inline pressed(uint8_t button_id) noexcept { return Data::get().buttons_pressed.contains(button_id); }
		// check if mouse button is being held down
		bool static inline down(uint8_t button_id) noexcept { return Data::get().buttons_down.contains(button_id); }
		// check if mouse button was released in this frame
		bool static inline released(uint8_t button_id) noexcept { return Data::get().buttons_released.contains(button_id); }
		// get the current mouse position in screen coordinates
		auto static inline position() noexcept -> std::pair<float, float> { return std::pair(Data::get().x, Data::get().y); };
		// get the change in mouse position since the last frame
		auto static inline delta() noexcept -> std::pair<float, float> { return std::pair(Data::get().dx, Data::get().dy); };
		// check if mouse is captured by the window
		bool static inline captured() noexcept { return Data::get().mouse_captured; }
	};
	
	// clear single-frame events
    void static flush() noexcept {
		Data::get().keys_pressed.clear();
		Data::get().keys_released.clear();
		Data::get().buttons_pressed.clear();
		Data::get().buttons_released.clear();
		Data::get().dx = 0;
		Data::get().dy = 0;
	}
	// clear all events (including continuous)
	void static flush_all() noexcept {
		flush();
		Data::get().keys_down.clear();
		Data::get().buttons_down.clear();
	}
	// pass an SDL event to the input system
	void static register_event(const SDL_Event& event) noexcept {
		switch (event.type) {
			case SDL_EventType::SDL_EVENT_KEY_UP:
				if (event.key.repeat || IMGUI_CAPTURE_KBD) return;
				Data::get().keys_released.insert(event.key.key);
				Data::get().keys_down.erase(event.key.key);
				break;
			case SDL_EventType::SDL_EVENT_KEY_DOWN:
				if (event.key.repeat || IMGUI_CAPTURE_KBD) return;
				Data::get().keys_pressed.insert(event.key.key);
				Data::get().keys_down.insert(event.key.key);
				break;
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:
				if (IMGUI_CAPTURE_MOUSE) return;
				Data::get().buttons_released.insert(event.button.button);
				Data::get().buttons_down.erase(event.button.button);
				break;
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN: 
				if (IMGUI_CAPTURE_MOUSE) return;
				Data::get().buttons_pressed.insert(event.button.button);
				Data::get().buttons_down.insert(event.button.button);
				break;
			case SDL_EventType::SDL_EVENT_MOUSE_MOTION:
				if (IMGUI_CAPTURE_MOUSE) return;
				Data::get().dx = event.motion.xrel; // Movimiento relativo
				Data::get().dy = event.motion.yrel;
				// Restablecer posición absoluta del mouse
				float mouseX, mouseY;
				SDL_GetMouseState(&mouseX, &mouseY);
				Data::get().x = mouseX;
				Data::get().y = mouseY;
				break;
			default: break;
		}
	}
	// update the current capture state of the mouse
	void static register_capture(bool captured) noexcept {
		Data::get().mouse_captured = captured;
	}
}
#undef IMGUI_CAPTURE_EVAL
typedef Input::Keys Keys;
typedef Input::Mouse Mouse;