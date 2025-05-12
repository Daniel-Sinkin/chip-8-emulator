// danielsinkin97@gmail.com
#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>

#include "constants.hpp"
#include "log.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace CHIP8 {
struct Chip8 {
    std::array<BYTE, 4 * 1024> mem = {};
    std::array<std::array<PIXEL, 64>, 32> display = {};
    WORD PC = 0;
    WORD I = 0; // index register
    int stack_pointer = 0;
    std::array<WORD, 32> stack = {};
    BYTE delay_timer = 0;
    BYTE sound_timer = 0;
    std::chrono::steady_clock::time_point last_timer_update;
    std::array<BYTE, 16> VX{};
    int iteration_counter = 0;
};
inline Chip8 chip8;

inline constexpr BYTE field_X(WORD w) { return (w >> 8) & 0xF; }
inline constexpr BYTE field_Y(WORD w) { return (w >> 4) & 0xF; }
inline constexpr BYTE field_N(WORD w) { return w & 0xF; }
inline constexpr BYTE field_NN(WORD w) { return w & 0xFF; }
inline constexpr WORD field_NNN(WORD w) { return w & 0x0FFF; }

inline auto clear_display(Chip8 &c) -> void {
    for (auto &row : c.display) {
        row.fill(0);
    }
}
inline auto draw_sprite(Chip8 &c, WORD w) -> void {
    BYTE x = c.VX[field_X(w)] % 64;
    BYTE y = c.VX[field_Y(w)] % 32;
    c.VX[0xF] = 0;
    for (int row = 0; row < field_N(w); ++row) {
        uint8_t sprite = c.mem[c.I + row];
        for (int bit = 0; bit < 8; ++bit) {
            auto px = (sprite >> (7 - bit)) & 1;
            auto &dst = c.display[y + row][x + bit];
            if (dst && px) c.VX[0xF] = 1;
            c.display[y + row][x + bit] ^= px;
        }
    }
}

using ExecFn = void (*)(Chip8 &, WORD);
using EncodeFn = WORD (*)(WORD X, WORD Y, WORD N, WORD NN, WORD NNN);

enum class Op {
    cls,                  // ✅ Clear display
    ret,                  // ❌ Return from Subroutine
    jmp,                  // ✅ Jumpt to Address
    call_subroutine,      // ❌ Call subroutine at nnn
    skip_eq,              // ❌
    skip_not_eq,          // ❌
    skip_eq_register,     // ❌
    set_register,         // ❌
    add_to_register,      // ❌
    copy_register,        // ❌
    math_or,              // ❌
    math_and,             // ❌
    math_xor,             // ❌
    math_add,             // ❌
    math_sub,             // ❌
    shr,                  // ❌
    subn,                 // ❌
    shl,                  // ❌
    skip_not_eq_register, // ❌
    set_i,                // ❌
    jmp_offset,           // ❌
    get_random,           // ❌
    draw,                 // ✅ Draw sprite at (Vx, Vy)
    skip_pressed,         // ❌
    skip_not_pressed,     // ❌
    load_delay,           // ❌
    wait_key,             // ❌
    set_delay,            // ✅ Set Delay Timer
    set_sound,            // ❌
    add_i,                // ❌
    set_i_sprite,         // ❌
    store_bcd,            // ❌
    dump_registers,       // ❌
    fill_registers,       // ❌
    sys,                  // ✅ Deprecated, not using
};

struct OpInfo {
    Op id;
    WORD mask;
    WORD pattern;
    std::string_view fmt;
    ExecFn exec;
    EncodeFn encode;
};

// clang-format off
inline auto exec_cls                 (Chip8 &c, WORD  ) -> void { clear_display(c); }
inline auto exec_ret                 (Chip8 &c, WORD  ) -> void { c.PC = c.stack[c.stack_pointer--]; }
inline auto exec_jmp                 (Chip8 &c, WORD w) -> void { c.PC = field_NNN(w); }
inline auto exec_call_subroutine     (Chip8 &c, WORD w) -> void { 
    c.stack[++c.stack_pointer] = c.PC;
    c.PC = field_NNN(w);
}
inline auto exec_skip_eq             (Chip8 &c, WORD w) -> void { if(c.VX[field_X(w)] == field_NN(w)) c.PC += 2; }
inline auto exec_skip_not_eq         (Chip8 &c, WORD w) -> void { if(c.VX[field_X(w)] != field_NN(w)) c.PC += 2; }
inline auto exec_skip_eq_register    (Chip8 &c, WORD w) -> void { if(c.VX[field_X(w)] != c.VX[field_Y(w)]) c.PC += 2; }
inline auto exec_set_register        (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] = field_NN(w); }
inline auto exec_add_to_register     (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] += field_NN(w); } // TODO: What to do about overflow?
inline auto exec_copy_register       (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] = c.VX[field_Y(w)]; }
inline auto exec_math_or             (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] |= c.VX[field_Y(w)]; }
inline auto exec_math_and            (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] &= c.VX[field_Y(w)]; }
inline auto exec_math_xor            (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] ^= c.VX[field_Y(w)]; }
inline auto exec_math_add            (Chip8 &c, WORD w) -> void {
    WORD tmp = c.VX[field_X(w)];
    tmp += c.VX[field_Y(w)];
    c.VX[0xF] = (tmp > 0xFF) ? 1 : 0; // Carry Flag bit
    c.VX[field_X(w)] = static_cast<BYTE>(tmp);
}
inline auto exec_math_sub            (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_shr                 (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_subn                (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_shl                 (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_skip_not_eq_register(Chip8 &c, WORD w) -> void { if(c.VX[field_X(w)] != c.VX[field_Y(w)]) c.PC += 2; }
inline auto exec_set_i               (Chip8 &c, WORD w) -> void {c.I = field_NNN(w);}
inline auto exec_jmp_offset          (Chip8 &c, WORD w) -> void { c.PC = field_NNN(w) + c.VX[0x0]; }
inline auto exec_get_random          (Chip8 &c, WORD w) -> void {
    BYTE rand = get_random_byte();
    c.VX[field_X(w)] = rand & field_NN(w);
}
inline auto exec_draw                (Chip8 &c, WORD w) -> void { draw_sprite(c, w); }
inline auto exec_skip_pressed        (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_skip_not_pressed    (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_load_delay          (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] = c.delay_timer; }
inline auto exec_wait_key            (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_set_delay           (Chip8 &c, WORD w) -> void { c.delay_timer = c.VX[field_X(w)]; }
inline auto exec_set_sound           (Chip8 &c, WORD w) -> void { c.sound_timer = c.VX[field_X(w)]; }
inline auto exec_add_i               (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_set_i_sprite        (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_store_bcd           (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_dump_registers      (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_fill_registers      (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_sys                 (Chip8 &c, WORD w) -> void { PANIC_UNDEFINED(w); }

inline auto encode_cls                 (WORD,WORD,WORD,WORD,WORD         ) -> WORD { return 0x00E0; }
inline auto encode_ret                 (WORD,WORD,WORD,WORD,WORD         ) -> WORD { return 0x00EE; }
inline auto encode_jmp                 (WORD,WORD,WORD,WORD,WORD NNN     ) -> WORD { return 0x1000 | (NNN & 0x0FFF); }
inline auto encode_call_subroutine     (WORD, WORD, WORD, WORD, WORD NNN ) -> WORD { return 0x2000 | (NNN & 0x0FFF); }
inline auto encode_skip_eq             (WORD X, WORD, WORD, WORD NN, WORD) -> WORD { return 0x3000 | ((X & 0xF) << 8) | (NN & 0xFF); }
inline auto encode_skip_not_eq         (WORD X, WORD, WORD, WORD NN, WORD) -> WORD { return 0x4000 | ((X & 0xF) << 8) | (NN & 0xFF); }
inline auto encode_skip_eq_register    (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x5000 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_set_register        (WORD X, WORD, WORD, WORD NN, WORD) -> WORD { return 0x6000 | ((X & 0xF) << 8) | (NN & 0xFF); }
inline auto encode_add_to_register     (WORD X, WORD, WORD, WORD NN, WORD) -> WORD { return 0x7000 | ((X & 0xF) << 8) | (NN & 0xFF); }
inline auto encode_copy_register       (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8000 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_math_or             (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8001 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_math_and            (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8002 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_math_xor            (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8003 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_math_add            (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8004 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_math_sub            (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8005 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_shr                 (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8006 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_subn                (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x8007 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_shl                 (WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x800E | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_skip_not_eq_register(WORD X, WORD Y, WORD, WORD, WORD ) -> WORD { return 0x9000 | ((X & 0xF) << 8) | ((Y & 0xF) << 4); }
inline auto encode_set_i               (WORD, WORD, WORD, WORD, WORD NNN ) -> WORD { return 0xA000 | (NNN & 0x0FFF); }
inline auto encode_jmp_offset          (WORD, WORD, WORD, WORD, WORD NNN ) -> WORD { return 0xB000 | (NNN & 0x0FFF); }
inline auto encode_get_random          (WORD X, WORD, WORD, WORD NN, WORD) -> WORD { return 0xC000 | ((X & 0xF) << 8) | (NN & 0xFF); }
inline auto encode_draw                (WORD X,WORD Y,WORD N,WORD,WORD   ) -> WORD { return 0xD000 | ((X&0xF)<<8) | ((Y&0xF)<<4) | (N&0xF); }
inline auto encode_skip_pressed        (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xE09E | ((X & 0xF) << 8); }
inline auto encode_skip_not_pressed    (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xE0A1 | ((X & 0xF) << 8); }
inline auto encode_load_delay          (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF007 | ((X & 0xF) << 8); }
inline auto encode_set_delay           (WORD X,WORD,WORD,WORD,WORD       ) -> WORD { return 0xF015 | ((X & 0xF) << 8); }
inline auto encode_wait_key            (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF00A | ((X & 0xF) << 8); }
inline auto encode_set_sound           (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF018 | ((X & 0xF) << 8); }
inline auto encode_add_i               (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF01E | ((X & 0xF) << 8); }
inline auto encode_set_i_sprite        (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF029 | ((X & 0xF) << 8); }
inline auto encode_store_bcd           (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF033 | ((X & 0xF) << 8); }
inline auto encode_dump_registers      (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF055 | ((X & 0xF) << 8); }
inline auto encode_fill_registers      (WORD X, WORD, WORD, WORD, WORD   ) -> WORD { return 0xF065 | ((X & 0xF) << 8); }
inline auto encode_sys                 (WORD,WORD,WORD,WORD,WORD NNN     ) -> WORD { return 0x0000 | (NNN & 0x0FFF); }

inline constexpr const std::array<OpInfo, 35> OPS = {{
    {Op::cls               , 0xFFFF, 0x00E0, "CLS"                     , exec_cls                   , encode_cls},
    {Op::ret               , 0xFFFF, 0x00EE, "RET"                     , exec_ret                   , encode_ret},
    {Op::jmp               , 0xF000, 0x1000, "JMP #{NNN:03X}"          , exec_jmp                   , encode_jmp},
    {Op::call_subroutine   , 0xF000, 0x2000, "CAL #{NNN:03X}"         , exec_call_subroutine       , encode_call_subroutine},
    {Op::skip_eq           , 0xF000, 0x3000, "SEQ V{X:X},#{NN:02X}"     , exec_skip_eq               , encode_skip_eq},
    {Op::skip_not_eq       , 0xF000, 0x4000, "SNE V{X:X},#{NN:02X}"    , exec_skip_not_eq           , encode_skip_not_eq},
    {Op::skip_eq_register  , 0xF00F, 0x5000, "SER V{X:X},V{Y:X}"        , exec_skip_eq_register      , encode_skip_eq_register},
    {Op::set_register      , 0xF000, 0x6000, "LDS V{X:X},#{NN:02X}"     , exec_set_register          , encode_set_register},
    {Op::add_to_register   , 0xF000, 0x7000, "ADR V{X:X},#{NN:02X}"    , exec_add_to_register       , encode_add_to_register},
    {Op::copy_register     , 0xF00F, 0x8000, "LDC V{X:X},V{Y:X}"        , exec_copy_register         , encode_copy_register},
    {Op::math_or           , 0xF00F, 0x8001, "ORR V{X:X},V{Y:X}"       , exec_math_or               , encode_math_or},
    {Op::math_and          , 0xF00F, 0x8002, "AND V{X:X},V{Y:X}"       , exec_math_and              , encode_math_and},
    {Op::math_xor          , 0xF00F, 0x8003, "XOR V{X:X},V{Y:X}"       , exec_math_xor              , encode_math_xor},
    {Op::math_add          , 0xF00F, 0x8004, "ADD V{X:X},V{Y:X}"       , exec_math_add              , encode_math_add},
    {Op::math_sub          , 0xF00F, 0x8005, "SUB V{X:X},V{Y:X}"       , exec_math_sub              , encode_math_sub},
    {Op::shr               , 0xF00F, 0x8006, "SHR V{X:X}"              , exec_shr                   , encode_shr},
    {Op::subn              , 0xF00F, 0x8007, "SBN V{X:X},V{Y:X}"      , exec_subn                  , encode_subn},
    {Op::shl               , 0xF00F, 0x800E, "SHL V{X:X}"              , exec_shl                   , encode_shl},
    {Op::skip_not_eq_register       , 0xF00F, 0x9000, "SNR V{X:X},V{Y:X}"       , exec_skip_not_eq_register  , encode_skip_not_eq_register},
    {Op::set_i             , 0xF000, 0xA000, "LDI I,#{NNN:03X}"        , exec_set_i                 , encode_set_i},
    {Op::jmp_offset        , 0xF000, 0xB000, "JMO V0,#{NNN:03X}"       , exec_jmp_offset            , encode_jmp_offset},
    {Op::get_random        , 0xF000, 0xC000, "RND V{X:X},#{NN:02X}"    , exec_get_random            , encode_get_random},
    {Op::draw               , 0xF000, 0xD000, "DRW V{X:X},V{Y:X},#{N:X}", exec_draw                   , encode_draw},
    {Op::skip_pressed      , 0xF0FF, 0xE09E, "SKP V{X:X}"              , exec_skip_pressed          , encode_skip_pressed},
    {Op::skip_not_pressed  , 0xF0FF, 0xE0A1, "SKN V{X:X}"             , exec_skip_not_pressed      , encode_skip_not_pressed},
    {Op::load_delay        , 0xF0FF, 0xF007, "LDD  V{X:X},DT"           , exec_load_delay            , encode_load_delay},
    {Op::wait_key          , 0xF0FF, 0xF00A, "LDK  V{X:X},K"            , exec_wait_key              , encode_wait_key},
    {Op::set_delay         , 0xF0FF, 0xF015, "SDD V{X:X}"              , exec_set_delay             , encode_set_delay},
    {Op::set_sound         , 0xF0FF, 0xF018, "SDT ST,V{X:X}"           , exec_set_sound             , encode_set_sound},
    {Op::add_i             , 0xF0FF, 0xF01E, "ADI I,V{X:X}"            , exec_add_i                 , encode_add_i},
    {Op::set_i_sprite      , 0xF0FF, 0xF029, "LDP  F,V{X:X}"            , exec_set_i_sprite          , encode_set_i_sprite},
    {Op::store_bcd         , 0xF0FF, 0xF033, "BCD  B,V{X:X}"            , exec_store_bcd             , encode_store_bcd},
    {Op::dump_registers    , 0xF0FF, 0xF055, "VXD [I],V{X:X}"           , exec_dump_registers        , encode_dump_registers},
    {Op::fill_registers    , 0xF0FF, 0xF065, "VXL V{X:X},[I]"           , exec_fill_registers        , encode_fill_registers},
    {Op::sys               , 0xF000, 0x0000, "SYS #{NNN:03X}"          , exec_sys                   , encode_sys},
}};
namespace detail {
    template <size_t N>
    constexpr bool are_unique_mnemonics(const std::array<OpInfo, N>& ops) {
        for (size_t i = 0; i < N; ++i) {
            const auto& a = ops[i].fmt;
            if (a.size() < 3) return false;
            auto a_prefix = std::string_view(a.data(), 3);
            for (size_t j = i + 1; j < N; ++j) {
                const auto& b = ops[j].fmt;
                if (b.size() < 3) return false;
                auto b_prefix = std::string_view(b.data(), 3);
                if (a_prefix == b_prefix) return false;
            }
        }
        return true;
    }
}
static_assert(detail::are_unique_mnemonics(OPS), "Duplicate 3-letter opcodes in OPS");


auto find_op(Op id) -> const OpInfo * {
    for (const auto &op : OPS)
        if (op.id == id)
            return &op;
    return nullptr;
}

class ProgramWriter {
public:
    WORD addr;

    void set_addr(WORD new_addr) { addr = new_addr; }
    explicit ProgramWriter(Chip8 & chip, WORD start = Constants::rom_program_start)
        : c(chip), addr(start) {}

    // TODO(CLEANUP): Fix Alignment
    void sys          (WORD nnn)                 { write_encoded(Op::sys,                 0,0,0,0, nnn); }
    void cls          ()                         { write_encoded(Op::cls); }
    void ret          ()                         { write_encoded(Op::ret); }                                // 00EE
    void jmp          (WORD nnn)                 { write_encoded(Op::jmp,                 0,0,0,0, nnn); }   // 1NNN
    void call         (WORD nnn)                 { write_encoded(Op::call_subroutine,     0,0,0,0, nnn); }   // 2NNN
    void jmp_offset   (WORD nnn)                 { write_encoded(Op::jmp_offset,          0,0,0,0, nnn); }   // BNNN
    void skip_eq            (BYTE x, BYTE kk)    { write_encoded(Op::skip_eq,             x,0,0, kk); }      // 3xkk
    void skip_not_eq        (BYTE x, BYTE kk)    { write_encoded(Op::skip_not_eq,         x,0,0, kk); }      // 4xkk
    void skip_eq_reg        (BYTE x, BYTE y)     { write_encoded(Op::skip_eq_register,    x,y); }            // 5xy0
    void skip_not_eq_reg    (BYTE x, BYTE y)     { write_encoded(Op::skip_not_eq_register,x,y); }            // 9xy0
    void skip_pressed       (BYTE x)             { write_encoded(Op::skip_pressed,        x); }              // Ex9E
    void skip_not_pressed   (BYTE x)             { write_encoded(Op::skip_not_pressed,    x); }              // ExA1
    void ld_vx_byte   (BYTE x, BYTE kk)          { write_encoded(Op::set_register,        x,0,0, kk); }      // 6xkk
    void add_vx_byte  (BYTE x, BYTE kk)          { write_encoded(Op::add_to_register,     x,0,0, kk); }      // 7xkk
    void ld_vx_vy     (BYTE x, BYTE y)           { write_encoded(Op::copy_register,       x,y); }            // 8xy0
    void or_vx_vy     (BYTE x, BYTE y)           { write_encoded(Op::math_or,             x,y); }            // 8xy1
    void and_vx_vy    (BYTE x, BYTE y)           { write_encoded(Op::math_and,            x,y); }            // 8xy2
    void xor_vx_vy    (BYTE x, BYTE y)           { write_encoded(Op::math_xor,            x,y); }            // 8xy3
    void add_vx_vy    (BYTE x, BYTE y)           { write_encoded(Op::math_add,            x,y); }            // 8xy4
    void sub_vx_vy    (BYTE x, BYTE y)           { write_encoded(Op::math_sub,            x,y); }            // 8xy5
    void shr_vx       (BYTE x, BYTE y = 0)       { write_encoded(Op::shr,                 x,y); }            // 8xy6
    void subn_vx_vy   (BYTE x, BYTE y)           { write_encoded(Op::subn,                x,y); }            // 8xy7
    void shl_vx       (BYTE x, BYTE y = 0)       { write_encoded(Op::shl,                 x,y); }            // 8xyE
    void rnd_vx_byte  (BYTE x, BYTE kk)          { write_encoded(Op::get_random,          x,0,0, kk); }      // Cxkk
    void drw          (BYTE x, BYTE y, BYTE n)   { write_encoded(Op::draw,                 x,y,n); }          // Dxyn
    void ld_i_addr    (WORD nnn)                 { write_encoded(Op::set_i,               0,0,0,0, nnn); }   // ANNN
    void add_i_vx     (BYTE x)                   { write_encoded(Op::add_i,               x); }              // Fx1E
    void ld_f_vx      (BYTE x)                   { write_encoded(Op::set_i_sprite,        x); }              // Fx29
    void bcd_vx       (BYTE x)                   { write_encoded(Op::store_bcd,           x); }              // Fx33
    void dump_vx      (BYTE x)                   { write_encoded(Op::dump_registers,      x); }              // Fx55
    void fill_vx      (BYTE x)                   { write_encoded(Op::fill_registers,      x); }              // Fx65
    void ld_vx_dt     (BYTE x)                   { write_encoded(Op::load_delay,          x); }              // Fx07
    void ld_vx_k      (BYTE x)                   { write_encoded(Op::wait_key,            x); }              // Fx0A
    void set_delay    (BYTE x)                   { write_encoded(Op::set_delay,           x); }              // Fx15
    void set_sound    (BYTE x)                   { write_encoded(Op::set_sound,           x); }              // Fx18

private:
    Chip8 &c;

    // central helper that does the actual write
    void write_encoded(Op id, WORD X = 0, WORD Y = 0,
                       WORD N = 0, WORD NN = 0, WORD NNN = 0)
    {
        const auto* op = find_op(id);
        if (!op) throw std::runtime_error("Unknown opcode ID");
        WORD instr = op->encode(X, Y, N, NN, NNN);
        c.mem[addr++] = BYTE(instr >> 8);
        c.mem[addr++] = BYTE(instr & 0xFF);
    }
};
// clang-format on

inline auto decode(WORD opcode) -> OpInfo const * {
    for (auto const &op : OPS)
        if ((opcode & op.mask) == op.pattern) return &op;
    return nullptr;
}

inline auto human_readable_fmt(WORD opcode) -> std::optional<std::string> {
    if (const auto *info = decode(opcode)) {
        switch (info->id) {
        case Op::sys:
            if (opcode == 0) return std::nullopt;
            return std::format("Execute system call at #{:03X}", field_NNN(opcode));
        case Op::cls:
            return "Clear the display";
        case Op::ret:
            return "Return from sub-routine";
        case Op::jmp:
            return std::format(
                "Jump to address #{:03X}", field_NNN(opcode));
        case Op::call_subroutine:
            return std::format(
                "Call sub-routine at #{:03X}", field_NNN(opcode));
        case Op::jmp_offset:
            return std::format(
                "Jump to V0 + #{:03X}", field_NNN(opcode));
        case Op::skip_eq:
            return std::format(
                "Skip next if V{:X} == #{:02X}",
                field_X(opcode), field_NN(opcode));
        case Op::skip_not_eq:
            return std::format(
                "Skip next if V{:X} != #{:02X}",
                field_X(opcode), field_NN(opcode));
        case Op::skip_eq_register:
            return std::format(
                "Skip next if V{:X} == V{:X}",
                field_X(opcode), field_Y(opcode));
        case Op::skip_not_eq_register:
            return std::format(
                "Skip next if V{:X} != V{:X}",
                field_X(opcode), field_Y(opcode));
        case Op::skip_pressed:
            return std::format(
                "Skip next if key V{:X} pressed",
                field_X(opcode));
        case Op::skip_not_pressed:
            return std::format(
                "Skip next if key V{:X} NOT pressed",
                field_X(opcode));
        case Op::set_register:
            return std::format(
                "V{:X} <- #{:02X}",
                field_X(opcode), field_NN(opcode));
        case Op::add_to_register:
            return std::format(
                "V{:X} += #{:02X}",
                field_X(opcode), field_NN(opcode));
        case Op::copy_register:
            return std::format(
                "V{:X} <- V{:X}",
                field_X(opcode), field_Y(opcode));
        case Op::math_or:
            return std::format(
                "V{:X} |= V{:X}", field_X(opcode), field_Y(opcode));
        case Op::math_and:
            return std::format(
                "V{:X} &= V{:X}", field_X(opcode), field_Y(opcode));
        case Op::math_xor:
            return std::format(
                "V{:X} ^= V{:X}", field_X(opcode), field_Y(opcode));
        case Op::math_add:
            return std::format(
                "V{:X} += V{:X}   (VF = carry)",
                field_X(opcode), field_Y(opcode));
        case Op::math_sub:
            return std::format(
                "V{:X} -= V{:X}   (VF = !borrow)",
                field_X(opcode), field_Y(opcode));
        case Op::shr:
            return std::format(
                "V{:X} >>= 1      (VF = LSB before shift)",
                field_X(opcode));
        case Op::subn:
            return std::format(
                "V{:X} = V{:X}-V{:X} (VF = !borrow)",
                field_X(opcode), field_Y(opcode), field_X(opcode));
        case Op::shl:
            return std::format(
                "V{:X} <<= 1      (VF = MSB before shift)",
                field_X(opcode));
        case Op::set_i:
            return std::format(
                "I <- #{:03X}", field_NNN(opcode));
        case Op::add_i:
            return std::format(
                "I += V{:X}", field_X(opcode));
        case Op::set_i_sprite:
            return std::format(
                "I <- sprite address for digit V{:X}", field_X(opcode));
        case Op::store_bcd:
            return std::format(
                "Store BCD of V{:X} at I, I+1, I+2", field_X(opcode));
        case Op::dump_registers:
            return std::format(
                "Store V0..V{:X} to memory at I", field_X(opcode));
        case Op::fill_registers:
            return std::format(
                "Load V0..V{:X} from memory at I", field_X(opcode));
        case Op::load_delay:
            return std::format(
                "V{:X} <- delay-timer", field_X(opcode));
        case Op::wait_key:
            return std::format(
                "Wait for key-press, store in V{:X}", field_X(opcode));
        case Op::set_delay:
            return std::format(
                "delay-timer <- V{:X}", field_X(opcode));
        case Op::set_sound:
            return std::format(
                "sound-timer <- V{:X}", field_X(opcode));
        case Op::get_random:
            return std::format(
                "V{:X} <- (rand & #{:02X})",
                field_X(opcode), field_NN(opcode));
        case Op::draw:
            return std::format(
                "Draw 8x{:X} sprite at (V{:X},V{:X})   (VF = collision)",
                field_N(opcode), field_X(opcode), field_Y(opcode));
        default:
            return std::nullopt;
        }
    }
    return std::nullopt;
}

inline auto disassemble(WORD w) -> std::string {
    if (!w) return "";
    auto *info = decode(w);
    if (!info) return std::format("DW  0x{:04X}", w);

    std::string s(info->fmt);
    auto replace = [&](std::string_view key, std::string val) {
        if (auto pos = s.find(key); pos != s.npos) s.replace(pos, key.size(), std::move(val));
    };

    replace("{X:X}", std::format("{:X}", field_X(w)));
    replace("{Y:X}", std::format("{:X}", field_Y(w)));
    replace("{N:X}", std::format("{:X}", field_N(w)));
    replace("{NN:02X}", std::format("{:02X}", field_NN(w)));
    replace("{NNN:03X}", std::format("{:03X}", field_NNN(w)));
    return s;
}

inline auto fetch_and_execute(Chip8 &c) -> void {
    c.iteration_counter += 1;
    WORD op = (c.mem[c.PC] << 8) | c.mem[c.PC + 1];
    c.PC += 2;

    if (auto info = decode(op)) {
        info->exec(c, op);
        return;
    }
    PANIC_NOT_IMPLEMENTED(op);
}

inline auto format_instruction_line(WORD pc, WORD instr) -> std::string {
    constexpr int align_to = 20;
    std::string disasm = CHIP8::disassemble(instr);

    std::string padded = disasm;
    if (padded.size() < align_to)
        padded += std::string(align_to - padded.size(), ' ');

    if (auto human = human_readable_fmt(instr); human)
        return std::format("{:04X}: {}; {}", pc, padded, *human);
    else
        return std::format("{:04X}: {}", pc, disasm);
}

inline auto log_current_operation(const Chip8 &c) -> void {
    WORD w = (c.mem[c.PC] << 8) | c.mem[c.PC + 1];
    LOG_INFO("{}", format_instruction_line(c.PC, w));
}

inline auto dump_memory(Chip8 &c) {
    std::ofstream f("memory.bin", std::ios::binary);
    f.write(reinterpret_cast<char const *>(c.mem.data()), c.mem.size());
}

inline auto load_ch8(const std::filesystem::path &filepath) -> std::vector<WORD> {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open ROM file: " + filepath.string());
    }

    std::vector<BYTE> raw_data(std::istreambuf_iterator<char>(file), {});
    if (raw_data.size() % 2 != 0) {
        throw std::runtime_error("ROM size is not even — invalid instruction alignment");
    }

    std::vector<WORD> instructions;
    instructions.reserve(raw_data.size() / 2);

    for (size_t i = 0; i < raw_data.size(); i += 2) {
        // HIGH byte first, matches big-endian file layout
        WORD instr = (static_cast<WORD>(raw_data[i]) << 8) | raw_data[i + 1];
        instructions.push_back(instr);
    }

    return instructions;
}

inline auto write_program_to_memory(Chip8 &c, const std::vector<WORD> data) -> void {
    WORD addr = Constants::rom_program_start;
    for (WORD instr : data) {
        if (addr + 1 >= c.mem.size()) {
            PANIC("Instruction write exceeds memory bound!");
        }
        c.mem[addr++] = static_cast<BYTE>((instr >> 8) & 0xFF);
        c.mem[addr++] = static_cast<BYTE>(instr & 0xFF);
    }
}

inline auto load_program_example_ibm(Chip8 &c) -> void {
    write_program_to_memory(c, load_ch8("assets/IBM Logo.ch8"));
}

inline auto load_program_example_corax_test_rom(Chip8 &c) -> void {
    // https://github.com/corax89/chip8-test-rom
    write_program_to_memory(c, load_ch8("assets/test_opcode.ch8"));
}

auto load_program_example_simple(Chip8 &c) -> void {
    ProgramWriter p(c);

    p.cls();
    p.jmp(0x212);

    p.set_addr(0x212);
    p.ld_i_addr(0x050);
    p.ld_vx_byte(0xB, 0x37);
    p.add_vx_byte(0xC, 0x12);
}

inline auto initialise(Chip8 &c) -> void {
    { // Font data
        size_t loc = 0x050;
        for (auto &font : Constants::fontdata) {
            c.mem[loc] = font;
            ++loc;
        }
    } // Font data
    c.PC = Constants::rom_program_start;
    c.last_timer_update = std::chrono::steady_clock::now();
}

inline auto update_timers(Chip8 &c) -> void {
    using namespace std::chrono;

    auto current_time = steady_clock::now();
    auto time_passed = current_time - c.last_timer_update;

    auto ticks = time_passed / Constants::timer_update_delay;
    if (ticks > 0) {
        auto n = ticks; // int64_t or duration
        c.delay_timer = (c.delay_timer > n) ? c.delay_timer - n : 0;
        c.sound_timer = (c.sound_timer > n) ? c.sound_timer - n : 0;

        c.last_timer_update += Constants::timer_update_delay * n;
    }
}

/* Batches some predefined number of iterations and updates timer once */
inline auto step(Chip8 &c, size_t num_iterations) -> void {
    update_timers(c);
    for (size_t i = 0; i < num_iterations; ++i) {
        fetch_and_execute(c);
    }
}
inline auto step(Chip8 &c) -> void {
    step(c, Constants::n_iter_per_frame);
}

/**
 * Disassemble a binary ROM and write a side-by-side text listing.
 *
 * The function always assumes that the first byte of the file will be loaded
 * at CHIP-8 address 0x200 and increments the program counter accordingly.
 *
 * @param rom_path   Path to the *.ch8* file.
 * @param out_path   (optional) Explicit output location.
 *                   If omitted the function writes a file with the same base
 *                   name and extension “.ch8_code” next to the input ROM.
 * @return           The path of the created listing file.
 */
inline auto disassemble_rom_to_file(
    const std::filesystem::path &rom_path,
    std::optional<std::filesystem::path> out_path = std::nullopt)
    -> std::filesystem::path {
    // Load and validate ROM (re-uses utility already in your code base).
    const std::vector<WORD> instructions = load_ch8(rom_path);

    if (!out_path) {
        out_path = rom_path; // copy
        out_path->replace_extension(
            out_path->extension().string() + "_code"); //  *.ch8_code
    }

    std::ofstream ofs(*out_path);
    if (!ofs) {
        throw std::runtime_error(
            "Failed to create listing file: " + out_path->string());
    }

    WORD pc = Constants::rom_program_start;
    for (const WORD instr : instructions) {
        ofs << format_instruction_line(pc, instr) << '\n';
        pc += 2; // each opcode = 2 bytes
    }

    ofs.flush();
    return *out_path;
}

} // namespace CHIP8