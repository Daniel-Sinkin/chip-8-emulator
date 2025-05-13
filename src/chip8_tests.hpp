/* danielsinkin97@gmail.com */
#include "chip8.hpp"
#include "types.hpp"

namespace CHIP8::TESTS {
auto opcode_roundtrip() -> void {
    for (int raw = 0x0000; raw <= 0xFFFF; ++raw) {
        WORD opcode = static_cast<WORD>(raw);
        const auto *info = CHIP8::decode(opcode);
        if (!info) continue;
        BYTE X = CHIP8::field_X(opcode);
        BYTE Y = CHIP8::field_Y(opcode);
        BYTE N = CHIP8::field_N(opcode);
        BYTE NN = CHIP8::field_NN(opcode);
        WORD NNN = CHIP8::field_NNN(opcode);

        assert(info->encode(X, Y, N, NN, NNN) == opcode);

        auto doc = CHIP8::human_readable_fmt(opcode);
        assert(doc.has_value());
    }
}
} // namespace CHIP8::TESTS