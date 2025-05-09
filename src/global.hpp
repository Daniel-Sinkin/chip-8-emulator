#pragma once

#include "constants.hpp"
#include "gl.hpp"
#include "types.hpp"
#include <SDL.h>
#include <chrono>
#include <imgui.h>

struct RendererState {
    SDL_Window *window = nullptr;
    SDL_GLContext gl_context;
    ImGuiIO imgui_io;

    VAO vao_square;
    VAO vao_circle;
    VAO vao_triangle;
    VAO vao_NONE = GL_ZERO;

    ShaderProgram shader_program_single_color;

    int gl_success;
    char gl_error_buffer[512];

    auto panic_gl(const char *reason) -> void {
        std::cerr << reason << "\n"
                  << gl_error_buffer << "\n";
        panic("GL Error");
    }
};

struct SimulationState {
    int frame_counter = 0;
    std::chrono::steady_clock::time_point run_start_time;
    std::chrono::steady_clock::time_point frame_start_time;
    std::chrono::duration<float> delta_time;
    std::chrono::duration<float> total_runtime;
};

struct InputState {
    Position mouse_pos;
};

struct ColorPalette {
    Color background = color_from_u8(15, 15, 21);
};

struct Global {
    bool is_running = false;
    RendererState renderer;
    SimulationState sim;
    InputState input;
    ColorPalette color;
};
inline Global global;