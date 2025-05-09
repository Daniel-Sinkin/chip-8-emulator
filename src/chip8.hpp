// danielsinkin97@gmail.com
#pragma once

using BYTE = uint8_t;
using PIXEL = uint8_t; // In principle we could use bool but bit arrays are slower, memory tradeoff worth it
using WORD = uint16_t;

struct Chip8 {
    std::array<std::array<PIXEL, 64>, 32> display = {};
};
inline Chip8 chip8;