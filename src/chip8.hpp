// danielsinkin97@gmail.com
#pragma once

#include <array>
#include <cstdint>
#include <stack>

using BYTE = uint8_t;
using PIXEL = uint8_t; // In principle we could use bool but bit arrays are slower, memory tradeoff worth it
using WORD = uint16_t;

struct Chip8 {
    std::array<BYTE, 4 * 1024> mem = {};
    std::array<std::array<PIXEL, 64>, 32> display = {};
    WORD PC = 0;
    WORD I = 0; // index register
    int stack_pointer = 0;
    std::array<WORD, 32> stack = {};
    BYTE delay_timer = 0;
    BYTE sound_timer = 0;
    std::array<BYTE, 16> VX{};
};
inline Chip8 chip8;

auto clear_display(Chip8 &c) -> void {
    for (auto &row : c.display) {
        std::fill(row.begin(), row.end(), 0);
    }
}