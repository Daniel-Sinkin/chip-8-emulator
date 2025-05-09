/* danielsinkin97@gmail.com */
#pragma once

#include "global.hpp"
#include "utils.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "imgui.h"
#include <glad/glad.h>

inline auto render_gui_debug_panel() -> void {
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
    ImGui::Render();
}

inline auto render_frame() -> void {
    glViewport(0, 0, (int)global.renderer.imgui_io.DisplaySize.x, (int)global.renderer.imgui_io.DisplaySize.y);
    glClearColor(global.color.background.r, global.color.background.g, global.color.background.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}