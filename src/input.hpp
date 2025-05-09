/* danielsinkin97@gmail.com */
#pragma once

#include "constants.hpp"
#include "global.hpp"
#include "log.hpp"
#include "types.hpp"
#include "utils.hpp"

#include "backends/imgui_impl_sdl.h"
#include <SDL.h>

inline auto update_mouse_position() -> void {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    global.input.mouse_pos = Position{
        static_cast<float>(mouse_x) / Constants::window_width,
        static_cast<float>(mouse_y) / Constants::window_height};
}

inline auto handle_event(const SDL_Event &event) -> void {
    ImGui_ImplSDL2_ProcessEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        LOG_INFO("Received SDL_QUIT event");
        global.is_running = false;
        break;

    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            LOG_INFO("Escape key pressed — exiting");
            global.is_running = false;
            break;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
            LOG_INFO("Left click at: " + to_string(global.input.mouse_pos));
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            Position mouse_pos_ndc = window_normalized_to_ndc(global.input.mouse_pos, Constants::aspect_ratio);
            LOG_INFO("Right click NDC: " + to_string(mouse_pos_ndc));
        }
        break;
    }
}

inline auto handle_input() -> void {
    update_mouse_position();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handle_event(event);
    }
}