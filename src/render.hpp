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

namespace RENDER {
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
    { // Keypad
    }
    ImGui::End();
}

inline auto keypad() -> void {
    ImGui::Begin("Keypad");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

    // grab raw SDL keyboard state
    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    auto &style = ImGui::GetStyle();

    // mapping: button label, SDL scancode, CHIP-8 keypad index
    struct Key {
        const char *label;
        SDL_Scancode sc;
        int idx;
    };
    static constexpr Key keymap[16] = {
        {"1", SDL_SCANCODE_1, 0x1},
        {"2", SDL_SCANCODE_2, 0x2},
        {"3", SDL_SCANCODE_3, 0x3},
        {"C", SDL_SCANCODE_4, 0xC},

        {"4", SDL_SCANCODE_Q, 0x4},
        {"5", SDL_SCANCODE_W, 0x5},
        {"6", SDL_SCANCODE_E, 0x6},
        {"D", SDL_SCANCODE_R, 0xD},

        {"7", SDL_SCANCODE_A, 0x7},
        {"8", SDL_SCANCODE_S, 0x8},
        {"9", SDL_SCANCODE_D, 0x9},
        {"E", SDL_SCANCODE_F, 0xE},

        {"A", SDL_SCANCODE_Z, 0xA},
        {"0", SDL_SCANCODE_X, 0x0},
        {"B", SDL_SCANCODE_C, 0xB},
        {"F", SDL_SCANCODE_V, 0xF},
    };

    // (re)initialize all keys to “up” each frame
    for (int i = 0; i < 16; ++i)
        chip8.keypad[i] = 0;

    // render 4×4 grid
    for (int i = 0; i < 16; ++i) {
        const auto &km = keymap[i];
        bool isDown = keys[km.sc];

        // if held, light it up
        if (isDown)
            chip8.keypad[km.idx] = 1;

        // pick colors: default vs “active” tint
        ImVec4 col = isDown ? style.Colors[ImGuiCol_ButtonActive] : style.Colors[ImGuiCol_Button];
        ImVec4 colHov = isDown ? style.Colors[ImGuiCol_ButtonActive] : style.Colors[ImGuiCol_ButtonHovered];
        ImVec4 colAct = style.Colors[ImGuiCol_ButtonActive];

        ImGui::PushStyleColor(ImGuiCol_Button, col);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colHov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colAct);

        // label shows the hex digit, unique ID hides repetition
        std::string lbl = std::string(km.label) + "##key_" + km.label;
        if (ImGui::Button(lbl.c_str(), ImVec2(40, 40))) {
            // also allow mouse-click to press
            chip8.keypad[km.idx] = 1;
        }

        ImGui::PopStyleColor(3);

        // same-line except after every 4th
        if ((i & 3) != 3)
            ImGui::SameLine();
    }

    ImGui::PopStyleVar();
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
    keypad();
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
} // namespace RENDER