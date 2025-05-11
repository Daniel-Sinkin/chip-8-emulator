/* danielsinkin97@gmail.com */
#pragma once

#include "chip8.hpp"
#include "global.hpp"
#include "utils.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "imgui.h"
#include <glad/glad.h>

namespace Render {
inline auto display_grid() -> void {
    constexpr int pixel_size = 10;

    ImGui::Begin("Chip8");
    { // Pixel Buffer
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 64; ++x) {
                // use your Color type and convert to ImVec4 at render time
                Color c = chip8.display[y][x] ? global.color.pixel_on : global.color.pixel_off;
                ImVec4 color = ImVec4(c.r, c.g, c.b, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

                std::string id = "##px_" + std::to_string(y) + "_" + std::to_string(x);
                if (ImGui::Button(id.c_str(), ImVec2(pixel_size, pixel_size))) {
                    chip8.display[y][x] ^= 1; // Toggle pixel value
                }

                ImGui::PopStyleColor(3);
                if (x < 63) ImGui::SameLine();
            }
        }
        ImGui::PopStyleVar();
    } // Pixel Buffer

    { // Chip8 Internals
        WORD instr_raw = (chip8.mem[chip8.PC] << 8) | chip8.mem[chip8.PC + 1];
        Instruction instr{instr_raw};

        if (auto desc = describe_instruction(instr)) {
            ImGui::Text("PC: 0x%03X | Instr: 0x%04X | %s",
                chip8.PC, instr_raw, desc->c_str());
        } else {
            ImGui::Text("PC: 0x%03X | Instr: 0x%04X",
                chip8.PC, instr_raw);
        }

        BYTE mem_at_I = chip8.mem[chip8.I];
        ImGui::Text("Index Register (I): 0x%03X (Mem[I] = 0x%02X)",
            chip8.I, mem_at_I);

        ImGui::Text("Stack Pointer: %d", chip8.stack_pointer);
        ImGui::Text("Delay Timer: %d", chip8.delay_timer);
        ImGui::Text("Sound Timer: %d", chip8.sound_timer);

        if (ImGui::BeginTable("VX Registers", 8)) {
            for (int i = 0; i < 16; ++i) {
                ImGui::TableNextColumn();
                ImGui::Text("V%X=0x%02X", i, chip8.VX[i]);
            }
            ImGui::EndTable();
        }
    } // Chip8 Internals
    ImGui::End();
}

inline auto gui_debug() -> void {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(global.renderer.window);
    ImGui::NewFrame();

    ImGui::Begin("Debug");
    ImGui::ColorEdit3("Background", &global.color.background.r);
    ImGui::ColorEdit3("Pixel On", &global.color.pixel_on.r);
    ImGui::ColorEdit3("Pixel Off", &global.color.pixel_off.r);
    ImGui::Text("Frame Counter: %d", global.sim.frame_counter);
    ImGui::Text("Runtime: %s",
        format_duration(global.sim.total_runtime).c_str());
    ImGui::Text("Delta Time (ms): %f", global.sim.delta_time.count());
    ImGui::Text("Mouse Position: (%.3f, %.3f)",
        global.input.mouse_pos.x, global.input.mouse_pos.y);
    ImGui::End();

    display_grid();
    ImGui::Render();
}

inline auto frame() -> void {
    glViewport(0, 0,
        (int)global.renderer.imgui_io.DisplaySize.x,
        (int)global.renderer.imgui_io.DisplaySize.y);
    glClearColor(global.color.background.r,
        global.color.background.g,
        global.color.background.b,
        1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
} // namespace Render