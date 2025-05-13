/* danielsinkin97@gmail.com */
#pragma once

#include "chip8.hpp"
#include "constants.hpp"
#include "log.hpp"
#include "types.hpp"
#include "utils.hpp"

#include "backends/imgui_impl_sdl.h"
#include <SDL.h>

using CHIP8::chip8;

namespace INPUT {
inline auto update_mouse_position() -> void {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    global.input.mouse_pos = Position{
        static_cast<float>(mouse_x) / CONSTANTS::window_width,
        static_cast<float>(mouse_y) / CONSTANTS::window_height};
}

// clang-format off
std::optional<int> map_sdl_key_to_chip8(SDL_Keycode key) {
    switch (key) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
        default: return std::nullopt;
    }
}
// clang-format on

inline auto handle_event(const SDL_Event &event) -> void {
    ImGui_ImplSDL2_ProcessEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        LOG_INFO("Received SDL_QUIT event");
        global.is_running = false;
        break;

    case SDL_KEYDOWN:
    case SDL_KEYUP: {
        bool is_down = (event.type == SDL_KEYDOWN);
        auto chip8_key = map_sdl_key_to_chip8(event.key.keysym.sym);
        if (chip8_key) {
            int key = *chip8_key;

            if (is_down && !chip8.keypad[key]) {
                chip8.just_pressed[key] = true;
            }

            chip8.keypad[key] = is_down;
        }

        if (event.key.keysym.sym == SDLK_ESCAPE && is_down) {
            LOG_INFO("Escape key pressed — exiting");
            global.is_running = false;
        }
        break;
    }

    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_RIGHT) {
            Position mouse_pos_ndc = window_normalized_to_ndc(global.input.mouse_pos, CONSTANTS::aspect_ratio);
            LOG_INFO("Right click NDC: " + to_string(mouse_pos_ndc));
        }
        break;
    }
}

inline auto handle_input() -> void {
    update_mouse_position();
    std::fill(std::begin(chip8.just_pressed), std::end(chip8.just_pressed), false);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handle_event(event);
    }
}
} // namespace INPUT