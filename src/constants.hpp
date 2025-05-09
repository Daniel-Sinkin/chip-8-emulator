// constants.hpp
#pragma once
#include <array>
#include <chrono>
#include <glm/glm.hpp>

namespace Constants {
inline constexpr std::string_view window_title = "Tower Defense";
inline constexpr int window_width = 1280;
inline constexpr int window_height = 720;
inline constexpr float aspect_ratio = static_cast<float>(window_width) / window_height;

inline constexpr float path_marker_width = 0.025f;
inline constexpr float path_marker_height = 0.025f;

inline constexpr std::chrono::duration<float> projectile_life_time = std::chrono::duration<float>(1.0f);

inline constexpr std::array<float, 12> square_vertices = {
    1.0f, -1.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, -1.0f, 0.0f};

inline constexpr std::array<unsigned int, 6> square_indices = {
    0, 1, 3,
    1, 2, 3};

inline constexpr std::array<float, 9> triangle_vertices = {
    0.5f, 0.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f};

inline constexpr std::array<unsigned int, 3> triangle_indices = {
    0, 1, 2};

inline constexpr std::array<float, 51> circle_vertices = {
    0.000000f, 0.000000f, 0.000000f,
    1.000000f, 0.000000f, 0.000000f,
    0.923880f, 0.382684f, 0.000000f,
    0.707106f, 0.707106f, 0.000000f,
    0.382684f, 0.923880f, 0.000000f,
    0.000000f, 1.000000f, 0.000000f,
    -0.382684f, 0.923880f, 0.000000f,
    -0.707106f, 0.707106f, 0.000000f,
    -0.923880f, 0.382684f, 0.000000f,
    -1.000000f, 0.000000f, 0.000000f,
    -0.923880f, -0.382684f, 0.000000f,
    -0.707106f, -0.707106f, 0.000000f,
    -0.382684f, -0.923880f, 0.000000f,
    -0.000000f, -1.000000f, 0.000000f,
    0.382684f, -0.923880f, 0.000000f,
    0.707106f, -0.707106f, 0.000000f,
    0.923880f, -0.382684f, 0.000000f};

inline constexpr std::array<unsigned int, 48> circle_indices = {
    0, 1, 2, 0, 2, 3, 0, 3, 4,
    0, 4, 5, 0, 5, 6, 0, 6, 7,
    0, 7, 8, 0, 8, 9, 0, 9, 10,
    0, 10, 11, 0, 11, 12, 0, 12, 13,
    0, 13, 14, 0, 14, 15, 0, 15, 16,
    0, 16, 1};

namespace Color {
inline constexpr glm::vec3 white = {1.0f, 1.0f, 1.0f};
inline constexpr glm::vec3 black = {0.0f, 0.0f, 0.0f};
inline constexpr glm::vec3 red = {1.0f, 0.0f, 0.0f};
inline constexpr glm::vec3 green = {0.0f, 1.0f, 0.0f};
inline constexpr glm::vec3 blue = {0.0f, 0.0f, 1.0f};
} // namespace Color

inline constexpr int max_tower_level = 5;

inline constexpr char const *fp_shader_dir = "assets/shaders/";
inline constexpr char const *fp_vertex_shader = "assets/shaders/vertex.glsl";
inline constexpr char const *fp_fragment_shader = "assets/shaders/fragment.glsl";
} // namespace Constants