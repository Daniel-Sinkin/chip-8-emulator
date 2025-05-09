// danielsinkin97@gmail.com
#pragma once

#include <array>
#include <cstdint>

using BYTE = uint8_t;
using PIXEL = uint8_t; // In principle we could use bool but bit arrays are slower, memory tradeoff worth it
using WORD = uint16_t;

struct Chip8 {
    std::array<BYTE, 4 * 1024> mem = {};
    std::array<std::array<PIXEL, 64>, 32> display = {};
    WORD PC = 0;
    WORD index_register = 0; // I
    WORD SP = 0;
    BYTE delay_timer = 0;
    BYTE sound_timer = 0;
    BYTE V0 = 0;
    BYTE V1 = 0;
    BYTE V2 = 0;
    BYTE V3 = 0;
    BYTE V4 = 0;
    BYTE V5 = 0;
    BYTE V6 = 0;
    BYTE V7 = 0;
    BYTE V8 = 0;
    BYTE V9 = 0;
    BYTE VA = 0;
    BYTE VB = 0;
    BYTE VC = 0;
    BYTE VD = 0;
    BYTE VE = 0;
    BYTE VF = 0;
};
inline Chip8 chip8;