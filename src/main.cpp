/* danielsinkin97@gmail.com */

// Core system and OpenGL
#include <SDL.h>
#include <glad/glad.h>

// ImGui backends
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "imgui.h"

// Standard library
#include <chrono>
#include <iostream>

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

auto main(int argc, char **argv) -> int {
    if (!engine_setup()) panic("Setup failed!");
    global.is_running = true;
    global.sim.run_start_time = std::chrono::steady_clock::now();
    global.sim.frame_start_time = global.sim.run_start_time;
    while (global.is_running) {
        auto now = std::chrono::steady_clock::now();
        global.sim.delta_time = now - global.sim.frame_start_time;
        global.sim.frame_start_time = now;
        global.sim.total_runtime = now - global.sim.run_start_time;

        handle_inputs();
        render_gui_debug_panel();
        render_frame();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(global.renderer.window);

        global.sim.frame_counter += 1;
    }

    engine_cleanup();

    return EXIT_SUCCESS;
}