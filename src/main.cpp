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
#include <cstdint>
#include <format>
#include <iostream>
#include <thread>

using std::chrono::steady_clock;
using namespace std::chrono_literals;

// Project headers
#include "audio.hpp"
#include "chip8.hpp"
#include "chip8_types.hpp"
#include "chip8_writer.hpp"
#include "constants.hpp"
#include "engine.hpp"
#include "gl.hpp"
#include "global.hpp"
#include "input.hpp"
#include "log.hpp"
#include "render.hpp"
#include "types.hpp"
#include "utils.hpp"

using CHIP8::chip8;

auto example_dissamble() -> int {
    try {
        auto path1 = CHIP8::disassemble_rom_to_file("assets/IBM Logo.ch8");
        std::cout << "Disassembled IBM Logo -> " << path1 << '\n';

        auto path2 = CHIP8::disassemble_rom_to_file("assets/test_opcode.ch8");
        std::cout << "Disassembled test_opcode -> " << path2 << '\n';
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error during disassembly: " << e.what() << '\n';
        return 1;
    }
}

auto example_ibm_with_sound() -> int {
    CHIP8::load_program_example_ibm(chip8);
    CHIP8::ProgramWriter pw(chip8, 0x228);
    pw.jmp(0x300);
    pw.addr = 0x300;
    pw.ld_vx_byte(0x5, 40);
    pw.set_delay(0x5);
    pw.ld_vx_byte(0x5, 40);
    pw.set_sound(0x5);
    pw.ld_vx_dt(0x5);
    pw.skip_eq(0x5, 0x0);
    pw.jmp(pw.addr - 0x4);
    pw.wait_key(0x6);
    pw.jmp(0x200);
    return 0;
}

auto main() -> int {
    CHIP8::initialise(chip8);
    // CHIP8::load_program_example_corax_test_rom(chip8);
    // CHIP8::load_program_example_test_suite_flags(chip8); // Doesn't Work
    // CHIP8::load_program_example_test_suite_quirks(chip8); // Doesn't Work
    // CHIP8::load_program_example_test_suite_keypad(chip8); // Doesn't Work
    // CHIP8::load_program_example_test_suite_beep(chip8); // Doesn't Work
    example_ibm_with_sound();

    LOG_INFO("Application starting");

    if (!engine_setup()) PANIC("Setup failed!");
    LOG_INFO("Engine setup complete");

    global.is_running = true;
    global.sim.run_start_time = std::chrono::steady_clock::now();
    global.sim.frame_start_time = global.sim.run_start_time;
    constexpr std::chrono::milliseconds instruction_interval{250};

    auto last_instruction_time = std::chrono::steady_clock::now();
    LOG_INFO("Entering main loop");
    while (global.is_running) {
        auto now = std::chrono::steady_clock::now();
        global.sim.delta_time = now - global.sim.frame_start_time;
        global.sim.frame_start_time = now;
        global.sim.total_runtime = now - global.sim.run_start_time;

        CHIP8::step(chip8, 1);

        handle_input();
        Render::gui_debug();
        Render::frame();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(global.renderer.window);

        global.sim.frame_counter += 1;
    }

    LOG_INFO("Main loop exited");
    engine_cleanup();
    LOG_INFO("Engine cleanup complete");
    LOG_INFO("Application exiting successfully");

    return EXIT_SUCCESS;
}