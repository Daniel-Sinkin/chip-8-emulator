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

using BYTE = uint8_t;
using WORD = uint16_t;

auto print_binary_word = [](WORD value) {
    std::bitset<16> bits(value);
    for (int i = 15; i >= 0; --i) {
        std::cout << bits[i];
        if (i % 4 == 0 && i != 0) std::cout << "_"; // Add separator every 4 bits
    }
};

struct Instruction {
    WORD val;

    auto op() -> BYTE { return static_cast<BYTE>((val & 0xF000) >> 12); }
    auto X() -> BYTE { return static_cast<BYTE>((val & 0x0F00) >> 8); }
    auto Y() -> BYTE { return static_cast<BYTE>((val & 0x00F0) >> 4); }
    auto NNN() -> WORD { return val & 0x0FFF; }
    auto NN() -> BYTE { return static_cast<BYTE>(val & 0x00FF); }
    auto N() -> BYTE { return static_cast<BYTE>(val & 0x000F); }
};

auto main2() -> void {
    Instruction instr = {.val = 0x01FE};
    print_binary_word(instr.val);
    std::cout << "\n";
    std::cout << "\n";
    std::cout << "Instruction(class): ";
    switch (instr.op()) {
    case 0x0:
        std::cout << "Machine Code Routine\n";
        if (instr.Y() == 0x0E) { // 0x000E
            std::cout << "Clear the Display\n";
        } else if (instr.Y() == 0xEE) { // 0x00EE
            std::cout << "Return from Subroutine\n";
            // Prolly should implement simple stack datastructure and pop here
            // -1 because we will increment PC afterwards.
            chip8.PC = chip8.stack[chip8.stack_pointer] - 1;
            --chip8.stack_pointer;
        } else { // 0x0NNN
            std::cout << "Call Machine Code Routine at Address ";
            print_binary_word(instr.NNN());
            std::cout << " (Will not implement)\n";
            PANIC_undefined(instr.val);
        }
        break;
    case 0x1:
        std::cout << "Machine Code Routine\n";
        std::cout << "Jump to Address ";
        print_binary_word(instr.NNN());
        chip8.PC = instr.NNN();
        break;
    case 0x6:
        std::cout << "Set Register\n";
        chip8.VX[instr.X()] = instr.NN();
    case 0x7:
        std::cout << "Add Value to Register\n";
        // TODO: What happens on overflow?
        chip8.VX[instr.X()] += instr.NN();
    case 0xA:
        std::cout << "Set I to Address\n";
        chip8.I = instr.NNN();
    case 0xD:
        std::cout << "Draw\n";
        BYTE VX = chip8.VX[instr.X()];
        BYTE VY = chip8.VX[instr.Y()];
        BYTE N = instr.N();
        WORD I_offset = 0;
        for (size_t row_idx = 0; row_idx < N; ++row_idx) {
            for (size_t col_idx = 0; col_idx < 8; ++col_idx) {
                WORD ypos = VY + row_idx;
                WORD xpos = VX + col_idx;
                if (chip8.display[ypos][xpos] == 1) {
                    if (chip8.mem[chip8.I + I_offset] == 1) {
                        chip8.display[ypos][xpos] = 0;
                        chip8.VX[0xF] = 1;
                    }
                } else {
                    chip8.display[ypos][xpos] ^= chip8.mem[chip8.I + I_offset];
                }
                ++I_offset;
            }
        }
    default:
        PANIC_not_implemented(instr.val);
    }
}
auto main(int argc, char **argv) -> int {
    main2();
    return EXIT_SUCCESS;
    LOG_INFO("Application starting");

    if (!engine_setup()) PANIC("Setup failed!");
    LOG_INFO("Engine setup complete");

    global.is_running = true;
    global.sim.run_start_time = std::chrono::steady_clock::now();
    global.sim.frame_start_time = global.sim.run_start_time;

    LOG_INFO("Entering main loop");
    while (global.is_running) {
        auto now = std::chrono::steady_clock::now();
        global.sim.delta_time = now - global.sim.frame_start_time;
        global.sim.frame_start_time = now;
        global.sim.total_runtime = now - global.sim.run_start_time;

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