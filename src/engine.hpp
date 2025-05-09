#pragma once

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "global.hpp"
#include "imgui.h"
#include "utils.hpp"
#include <SDL.h>
#include <glad/glad.h>

[[nodiscard]] inline auto engine_setup() -> bool {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    global.renderer.window = SDL_CreateWindow(
        Constants::window_title.data(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Constants::window_width, Constants::window_height,
        SDL_WINDOW_OPENGL);
    if (!global.renderer.window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return false;
    }

    global.renderer.gl_context = SDL_GL_CreateContext(global.renderer.window);
    if (!global.renderer.gl_context) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(global.renderer.window);
        SDL_Quit();
        return false;
    }

    SDL_GL_MakeCurrent(global.renderer.window, global.renderer.gl_context);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    global.renderer.imgui_io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(global.renderer.window, global.renderer.gl_context);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    return true;
}

inline auto engine_cleanup() -> void {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(global.renderer.gl_context);
    SDL_DestroyWindow(global.renderer.window);
    SDL_Quit();
}