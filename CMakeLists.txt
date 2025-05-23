cmake_minimum_required(VERSION 3.16)
project(main LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

include(FetchContent)

# ---------------------------------------
# Download CHIP-8 test ROMs into assets/code/
set(CHIP8_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets/code")
file(MAKE_DIRECTORY "${CHIP8_ASSETS_DIR}")

set(CHIP8_TEST_URLS
    "https://github.com/Timendus/chip8-test-suite/raw/main/bin/1-chip8-logo.ch8"
    "https://github.com/Timendus/chip8-test-suite/raw/main/bin/2-ibm-logo.ch8"
    "https://github.com/Timendus/chip8-test-suite/raw/main/bin/3-corax+.ch8"
    "https://github.com/Timendus/chip8-test-suite/raw/main/bin/4-flags.ch8"
    "https://github.com/Timendus/chip8-test-suite/raw/main/bin/5-quirks.ch8"
    "https://github.com/Timendus/chip8-test-suite/raw/main/bin/6-keypad.ch8"
    "https://github.com/Timendus/chip8-test-suite/raw/main/bin/7-beep.ch8"
    "https://github.com/stianeklund/chip8/raw/refs/heads/master/roms/PONG.ch8"
)

foreach(url IN LISTS CHIP8_TEST_URLS)
    get_filename_component(fname "${url}" NAME)
    set(dest "${CHIP8_ASSETS_DIR}/${fname}")

    message(STATUS "Downloading ${fname}…")
    file(DOWNLOAD
        "${url}"
        "${dest}"
        SHOW_PROGRESS
        STATUS dl_status
        LOG dl_log
    )
    list(GET dl_status 0 dl_code)
    if(NOT dl_code EQUAL 0)
        message(FATAL_ERROR "Failed to download ${url}: ${dl_log}")
    endif()
endforeach()

# ---------------------------------------
# Fetch GLAD (unchanged)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
FetchContent_MakeAvailable(glad)

# ---------------------------------------
# Fetch Dear ImGui (unchanged)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.2
)
FetchContent_MakeAvailable(imgui)

# ---------------------------------------
# Fetch stb
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# ---------------------------------------
# Fetch nlohmann/json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# ---------------------------------------
# Fetch glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# ---------------------------------------
# Fetch SDL2 
FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.26.5
)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sdl2)

# ---------------------------------------
# Fetch SDL2_mixer – audio back-end
FetchContent_Declare(
    sdl2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG        release-2.8.1
)
set(SDL2MIXER_VENDORED        ON  CACHE BOOL "" FORCE)
set(SDL2MIXER_MUSIC_MP3_DRMP3 ON  CACHE BOOL "" FORCE)
set(SDL2MIXER_MIDI            OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_INSTALL         OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sdl2_mixer)

# ---------------------------------------
# Source files & executable
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(main ${SOURCES})

# Copy data directory after build
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/assets
        COMMENT "Copying assets into build directory"
    )
else()
    message(WARNING "No assets directory found at ${CMAKE_SOURCE_DIR}/assets – skipping copy_assets target.")
    add_custom_target(copy_assets ALL
        COMMENT "No assets to copy"
    )
endif()
add_dependencies(main copy_assets)

# === include dirs ===
target_include_directories(main PRIVATE
    ${glad_SOURCE_DIR}/include
    ${sdl2_SOURCE_DIR}/include
)

target_include_directories(main SYSTEM PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${stb_SOURCE_DIR}
)

find_package(OpenGL REQUIRED)

# === warnings: only for our target ===
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(main PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wshadow -Wnon-virtual-dtor
    -Wold-style-cast -Wcast-align -Wconversion -Wsign-conversion
    -Wnull-dereference -Wdouble-promotion -Wduplicated-cond
    -Wduplicated-branches -Wlogical-op -Wuseless-cast
    -Wstrict-overflow=5 -Wformat=2
  )
endif()

# === Link libraries ===
target_link_libraries(main PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_mixer::SDL2_mixer
    glad
    OpenGL::GL
    imgui_impl
    glm::glm
    nlohmann_json::nlohmann_json
)

# === ImGui implementation (switch to SDL backend) ===
add_library(imgui_impl STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_impl PRIVATE
    ${sdl2_SOURCE_DIR}/include
)
target_include_directories(imgui_impl SYSTEM PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)