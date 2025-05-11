/* danielsinkin97@gmail.com */
#pragma once

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "imgui.h"
#include <glad/glad.h>

#include "chip8.hpp"
#include "global.hpp"
#include "utils.hpp"

using CHIP8::chip8;

namespace Render {
inline auto display_grid() -> void {
    constexpr int pixel_size = 10;

    ImGui::Begin("Chip8");
    { // Pixel Buffer
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 64; ++x) {
                Color c = chip8.display[y][x]
                              ? global.color.pixel_on
                              : global.color.pixel_off;
                ImVec4 color{c.r, c.g, c.b, 1.0f};

                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

                std::string id = "##px_" + std::to_string(y) + "_" + std::to_string(x);
                if (ImGui::Button(id.c_str(), ImVec2(pixel_size, pixel_size))) {
                    chip8.display[y][x] ^= 1;
                }

                ImGui::PopStyleColor(3);
                if (x < 63) ImGui::SameLine();
            }
        }
        ImGui::PopStyleVar();
    }

    { // Chip8 Internals
        {
            constexpr int LOOKBACK = 3;
            constexpr int LOOKFORWARD = 4;
            constexpr int BYTES_PER_INSTR = 2;
            constexpr int LINES_SHOWN = LOOKBACK + LOOKFORWARD;

            std::array<char, 512> buf{};
            std::ostringstream oss;

            for (int rel = -LOOKBACK; rel <= LOOKFORWARD; ++rel) {
                int addr = static_cast<int>(chip8.PC) + rel * BYTES_PER_INSTR;
                if (addr < 0 || addr + 1 >= chip8.mem.size()) continue;

                WORD opcode = (chip8.mem[addr] << 8) | chip8.mem[addr + 1];
                std::string dis = CHIP8::disassemble(opcode);

                oss << (rel == 0 ? "-> " : "   ")
                    << CHIP8::format_instruction_line(addr, opcode)
                    << '\n';
            }

            std::string s = oss.str();
            std::copy_n(s.c_str(), std::min(s.size() + 1, buf.size()), buf.data());

            ImVec2 size = ImVec2(
                -FLT_MIN,
                ImGui::GetTextLineHeightWithSpacing() * LINES_SHOWN);

            // Text box with just ReadOnly — avoid triggering scrollbar by fitting exactly
            ImGui::InputTextMultiline(
                "Disassembly",
                buf.data(), buf.size(),
                size,
                ImGuiInputTextFlags_ReadOnly);
        }

        BYTE mem_at_I = chip8.mem[chip8.I];
        ImGui::Text("Index Register (I): 0x%03X (Mem[I] = 0x%02X)",
            chip8.I, mem_at_I);

        ImGui::Text("Stack Pointer: %d", chip8.stack_pointer);
        ImGui::Text("Delay Timer: %d", chip8.delay_timer);
        ImGui::Text("Sound Timer: %d", chip8.sound_timer);
        ImGui::Text("Iteration Counter: %d", chip8.iteration_counter);

        if (ImGui::BeginTable("VX Registers", 8)) {
            for (int i = 0; i < 16; ++i) {
                ImGui::TableNextColumn();
                ImGui::Text("V%X = 0x%02X", i, chip8.VX[i]);
            }
            ImGui::EndTable();
        }
    }
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
    ImGui::Text("Delta Time (ms): %.3f", global.sim.delta_time.count());
    ImGui::Text("Mouse Position: (%.3f, %.3f)",
        global.input.mouse_pos.x,
        global.input.mouse_pos.y);
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