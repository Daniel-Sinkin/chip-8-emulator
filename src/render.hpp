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

    ImGui::Begin("Display");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            ImVec4 color = chip8.display[y][x] ? ImVec4(1, 1, 1, 1) : ImVec4(0, 0, 0, 1);
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
    ImGui::End();
}

inline auto gui_debug() -> void {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(global.renderer.window);
    ImGui::NewFrame();
    {
        ImGui::Begin("Debug");
        ImGui::ColorEdit3("Background", &global.color.background.r);
        ImGui::Text("Frame Counter: %d", global.sim.frame_counter);
        ImGui::Text("Runtime: %s", format_duration(global.sim.total_runtime).c_str());
        ImGui::Text("Delta Time (ms): %f", global.sim.delta_time.count());
        ImGui::Text("Mouse Position: (%.3f, %.3f)", global.input.mouse_pos.x, global.input.mouse_pos.y);
        ImGui::End();
    }
    display_grid();
    ImGui::Render();
}

inline auto frame() -> void {
    glViewport(0, 0, (int)global.renderer.imgui_io.DisplaySize.x, (int)global.renderer.imgui_io.DisplaySize.y);
    glClearColor(global.color.background.r, global.color.background.g, global.color.background.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
}