/* danielsinkin97@gmail.com */

// Core system and OpenGL
#include <SDL.h>
#include <glad/glad.h>

// ImGui backends
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "imgui.h"

// Standard library
#include <array>
#include <bitset>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

// Project headers
#include "constants.hpp"
#include "engine.hpp"
#include "gl.hpp"
#include "global.hpp"
#include "input.hpp"
#include "log.hpp"
#include "render.hpp"
#include "types.hpp"
#include "utils.hpp"

auto main2() -> void {
}

auto main(int argc, char **argv) -> int {
    initialise(chip8);
    load_program_example_ibm(chip8);
    for (size_t i = 0; i < 100000; ++i) {
    }
    dump_memory(chip8);

    LOG_INFO("Application starting");

    if (!engine_setup()) PANIC("Setup failed!");
    LOG_INFO("Engine setup complete");

    global.is_running = true;
    global.sim.run_start_time = std::chrono::steady_clock::now();
    global.sim.frame_start_time = global.sim.run_start_time;

    auto last_instruction_time = std::chrono::steady_clock::now();
    bool first_loop = true;

    LOG_INFO("Entering main loop");
    while (global.is_running) {
        auto now = std::chrono::steady_clock::now();
        global.sim.delta_time = now - global.sim.frame_start_time;
        global.sim.frame_start_time = now;
        global.sim.total_runtime = now - global.sim.run_start_time;

        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_instruction_time).count() >= 1) {
            auto instr = fetch_instruction(chip8);
            execute_instruction(chip8, instr);
            last_instruction_time = now;
        }

        handle_input();
        Render::gui_debug();
        Render::frame();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(global.renderer.window);

        if (first_loop) {
            first_loop = false;
        } else if (global.sim.frame_counter == 1) {
            std::this_thread::sleep_for(std::chrono::seconds(20));
        }
        global.sim.frame_counter += 1;
    }

    LOG_INFO("Main loop exited");
    engine_cleanup();
    LOG_INFO("Engine cleanup complete");
    LOG_INFO("Application exiting successfully");

    return EXIT_SUCCESS;
}