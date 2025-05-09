/* danielsinkin97@gmail.com */
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "types.hpp"
#include "utils.hpp"

using VAO = GLuint;
using VBO = GLuint;
using EBO = GLuint;
using ShaderID = GLuint;
using ProgramID = GLuint;
using UniformLocation = GLint;

class ShaderProgram {
public:
    ProgramID id = GL_ZERO;
    std::unordered_map<std::string, UniformLocation> uniforms;

    auto activate() const -> void {
        if (id == 0) panic("Trying to activate uninitialized ShaderProgram!");
        glUseProgram(id);
    }

    auto set_uniform(const std::string &name, float value) const -> void {
        glUniform1f(get_uniform(name), value);
    }

    auto set_uniform(const std::string &name, const glm::vec2 &v) const -> void {
        glUniform2f(get_uniform(name), v.x, v.y);
    }

    auto set_uniform(const std::string &name, const glm::vec3 &v) const -> void {
        glUniform3f(get_uniform(name), v.x, v.y, v.z);
    }

    auto load(const char *vertex_path, const char *fragment_path) -> void {
        const auto vert = compile_shader_from_file(vertex_path, GL_VERTEX_SHADER);
        const auto frag = compile_shader_from_file(fragment_path, GL_FRAGMENT_SHADER);

        id = glCreateProgram();
        glAttachShader(id, vert);
        glAttachShader(id, frag);
        glLinkProgram(id);

        GLint success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(id, 512, nullptr, info_log);
            std::cerr << "Shader Program Linking Failed:\n"
                      << info_log << "\n";
            panic("Shader Program linking failed.");
        }

        glDeleteShader(vert);
        glDeleteShader(frag);
    }

private:
    [[nodiscard]] auto get_uniform(const std::string &name) const -> UniformLocation {
        auto it = uniforms.find(name);
        if (it != uniforms.end()) return it->second;
        panic("Uniform not found: " + name);
    }

    [[nodiscard]] auto compile_shader_from_file(const char *filepath, GLenum type) -> ShaderID {
        std::ifstream in(filepath);
        if (!in) {
            std::cerr << "Couldn't open file: " << filepath << "\n";
            panic("Shader file open failed");
        }

        std::ostringstream ss;
        ss << in.rdbuf();
        const std::string source = ss.str();
        const char *src_ptr = source.c_str();

        ShaderID shader = glCreateShader(type);
        glShaderSource(shader, 1, &src_ptr, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Shader Compilation Failed (" << filepath << "):\n"
                      << log << "\n";
            panic("Shader compile error");
        }

        return shader;
    }
};

struct GeometryBuffers {
    VAO vao = GL_ZERO;
    VBO vbo = GL_ZERO;
    EBO ebo = GL_ZERO;
};

[[nodiscard]] inline auto
create_geometry(const float *vertices, size_t vertex_size, const unsigned int *indices, size_t index_size) -> GeometryBuffers {
    GeometryBuffers gb;
    glGenVertexArrays(1, &gb.vao);
    glBindVertexArray(gb.vao);

    glGenBuffers(1, &gb.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gb.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &gb.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gb.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, indices, GL_STATIC_DRAW);

    glBindVertexArray(GL_ZERO);
    return gb;
}

inline auto set_box_uniforms(const ShaderProgram &sp, const Rect &box) -> void {
    sp.set_uniform("u_Pos", vec2{box.position.x, box.position.y});
    sp.set_uniform("u_Width", box.width);
    sp.set_uniform("u_Height", box.height);
}

inline auto set_color_uniforms(const ShaderProgram &sp, const Color &color) -> void {
    sp.set_uniform("u_Color", vec3{color.r, color.g, color.b});
}