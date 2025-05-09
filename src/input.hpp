/* danielsinkin97@gmail.com */
#pragma once

#include "constants.hpp"
#include "global.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <SDL.h>
#include <iostream>

#include "backends/imgui_impl_sdl.h"

// Updates global.input.mouse_pos in normalized window coordinates
inline auto update_mouse_position() -> void {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    global.input.mouse_pos = Position{
        static_cast<float>(mouse_x) / Constants::window_width,
        static_cast<float>(mouse_y) / Constants::window_height};
}

// Processes one SDL event and updates global state accordingly
inline auto handle_event(const SDL_Event &event) -> void {
    ImGui_ImplSDL2_ProcessEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        global.is_running = false;
        break;

    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            global.is_running = false;
            break;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
            std::cout << "Mouse Clicked at: " << global.input.mouse_pos << "\n";
        }
        if (event.button.button == SDL_BUTTON_RIGHT) {
            Position mouse_pos_ndc = window_normalized_to_ndc(global.input.mouse_pos, Constants::aspect_ratio);
            (void)mouse_pos_ndc; // placeholder — do something with it later
        }
        break;
    }
}

// High-level input polling wrapper
inline auto handle_inputs() -> void {
    update_mouse_position();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handle_event(event);
    }
}