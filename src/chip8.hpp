// danielsinkin97@gmail.com
#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>

#include "constants.hpp"
#include "log.hpp"
#include "types.hpp"

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
    std::chrono::steady_clock::time_point timer_update = std::chrono::steady_clock::now();
    std::array<BYTE, 16> VX{};
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
            dst ^= px;
        }
    }
}

using ExecFn = void (*)(Chip8 &, WORD);
using EncodeFn = WORD (*)(WORD X, WORD Y, WORD N, WORD NN, WORD NNN);

enum class Op {
    cls,
    ret,
    sys,
    jmp,
    ld_vx_byte,  // Load immediate into Vx
    add_vx_byte, // Add immediate to Vx
    ld_i_addr,   // Load address into I
    drw          // Draw sprite at (Vx, Vy)
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
inline auto exec_cls        (Chip8 &c, WORD  ) -> void { clear_display(c); }
inline auto exec_ret        (Chip8 &c, WORD  ) -> void { c.PC = c.stack[--c.stack_pointer]; }
inline auto exec_sys        (Chip8 &c, WORD w) -> void { PANIC_NOT_IMPLEMENTED(w); }
inline auto exec_jmp        (Chip8 &c, WORD w) -> void { c.PC = field_NNN(w); }
inline auto exec_ld_vx_byte (Chip8 &c, WORD w) -> void { c.VX[field_X(w)] = field_NN(w); }
inline auto exec_add_vx_byte(Chip8 &c, WORD w) -> void { c.VX[field_X(w)] += field_NN(w); }
inline auto exec_ld_i_addr  (Chip8 &c, WORD w) -> void { c.I = field_NNN(w); }
inline auto exec_drw        (Chip8 &c, WORD w) -> void { draw_sprite(c, w); }

inline auto encode_cls        (WORD,WORD,WORD,WORD,WORD      ) -> WORD { return 0x00E0; }
inline auto encode_ret        (WORD,WORD,WORD,WORD,WORD      ) -> WORD { return 0x00EE; }
inline auto encode_sys        (WORD,WORD,WORD,WORD,WORD NNN  ) -> WORD { return 0x0000 | (NNN & 0x0FFF); }
inline auto encode_jmp        (WORD,WORD,WORD,WORD,WORD NNN  ) -> WORD { return 0x1000 | (NNN & 0x0FFF); }
inline auto encode_ld_vx_byte (WORD X,WORD,WORD,WORD NN,WORD ) -> WORD { return 0x6000 | ((X & 0xF)<<8) | (NN & 0xFF); }
inline auto encode_add_vx_byte(WORD X,WORD,WORD,WORD NN,WORD ) -> WORD { return 0x7000 | ((X & 0xF)<<8) | (NN & 0xFF); }
inline auto encode_ld_i_addr  (WORD,WORD,WORD,WORD,WORD NNN  ) -> WORD { return 0xA000 | (NNN & 0x0FFF); }
inline auto encode_drw        (WORD X,WORD Y,WORD N,WORD,WORD) -> WORD { return 0xD000 | ((X&0xF)<<8) | ((Y&0xF)<<4) | (N&0xF); }

inline constexpr std::array<OpInfo, 8> OPS = {{
    {Op::cls         , 0xFFFF, 0x00E0, "CLS"                          , exec_cls         , encode_cls         },
    {Op::ret         , 0xFFFF, 0x00EE, "RET"                          , exec_ret         , encode_ret         },
    {Op::sys         , 0xF000, 0x0000, "SYS #{NNN:03X}"               , exec_sys         , encode_sys         },
    {Op::jmp         , 0xF000, 0x1000, "JMP  #{NNN:03X}"              , exec_jmp         , encode_jmp         },
    {Op::ld_vx_byte  , 0xF000, 0x6000, "LDV V{X:X},#{NN:02X}"         , exec_ld_vx_byte  , encode_ld_vx_byte  },
    {Op::add_vx_byte , 0xF000, 0x7000, "ADD V{X:X},#{NN:02X}"         , exec_add_vx_byte , encode_add_vx_byte },
    {Op::ld_i_addr   , 0xF000, 0xA000, "LDI I,#{NNN:03X}"             , exec_ld_i_addr   , encode_ld_i_addr   },
    {Op::drw         , 0xF000, 0xD000, "DRW V{X:X},V{Y:X},#{N:X}"     , exec_drw         , encode_drw         },
}};

auto find_op(Op id) -> const OpInfo * {
    for (const auto &op : OPS)
        if (op.id == id)
            return &op;
    return nullptr;
}
struct ProgramWriter {
    Chip8& c;
    WORD   addr;

    explicit ProgramWriter(Chip8& chip, WORD start = 0x200)
      : c(chip), addr(start) {}

private:
    void write_encoded(Op id, WORD X=0, WORD Y=0, WORD N=0, WORD NN=0, WORD NNN=0) {
        auto* op = find_op(id);
        if (!op) throw std::runtime_error("Unknown opcode ID");
        WORD instr = op->encode(X, Y, N, NN, NNN);
        c.mem[addr++] = BYTE(instr >> 8);
        c.mem[addr++] = BYTE(instr & 0xFF);
    }

public:
    void cls()                      { write_encoded(Op::cls); }
    void ret()                      { write_encoded(Op::ret); }
    void sys(WORD nnn)              { write_encoded(Op::sys, 0,0,0,0, nnn); }
    void jmp(WORD nnn)              { write_encoded(Op::jmp,  0,0,0,0, nnn); }
    void ld_vx_byte(BYTE x, BYTE v) { write_encoded(Op::ld_vx_byte, x,0,0,v); }
    void add_vx_byte(BYTE x, BYTE v){ write_encoded(Op::add_vx_byte, x,0,0,v); }
    void ld_i_addr(WORD nnn)        { write_encoded(Op::ld_i_addr, 0,0,0,0,nnn); }
    void drw(BYTE x, BYTE y, BYTE n){ write_encoded(Op::drw, x,y,n); }

    void set_addr(WORD new_addr)    { addr = new_addr; }
};
// clang-format on

inline auto decode(WORD opcode) -> OpInfo const * {
    for (auto const &op : OPS)
        if ((opcode & op.mask) == op.pattern) return &op;
    return nullptr;
}

inline auto human_readable(WORD opcode) -> std::optional<std::string_view> {
    if (auto *info = decode(opcode)) {
        switch (info->id) {
        case Op::cls:
            return "Clear the display";
        case Op::ret:
            return "Return from sub‑routine";
        case Op::sys:
            return "Execute system call at NNN (legacy/ignored)";
        case Op::jmp:
            return "Jump to address NNN";
        case Op::ld_vx_byte:
            return "Set Vx = NN";
        case Op::add_vx_byte:
            return "Add NN to Vx";
        case Op::ld_i_addr:
            return "Set I = NNN";
        case Op::drw:
            return "Draw sprite (8×N) at (Vx, Vy)";
        default:
            return std::nullopt;
        }
    }
    return std::nullopt;
}

auto disassemble(WORD w) -> std::string {
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
    if (padded.size() < align_to) {
        padded += std::string(align_to - padded.size(), ' ');
    }

    if (auto human = human_readable(instr); human) {
        return std::format("{:04X}: {}; {}", pc, padded, *human);
    } else {
        return std::format("{:04X}: {}", pc, disasm);
    }
}

inline auto log_current_operation(Chip8 &c) -> void {
    WORD w = (c.mem[c.PC] << 8) | c.mem[c.PC + 1];
    LOG_INFO("{}", format_instruction_line(c.PC, w));
}

inline auto dump_memory(Chip8 &c) {
    std::ofstream f("memory.bin", std::ios::binary);
    f.write(reinterpret_cast<char const *>(c.mem.data()), c.mem.size());
}

auto load_ch8(const std::filesystem::path &filepath) -> std::vector<WORD> {
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

auto write_program_to_memory(Chip8 &c, const std::vector<WORD> data) -> void {
    WORD addr = 0x200;
    for (WORD instr : data) {
        if (addr + 1 >= c.mem.size()) {
            PANIC("Instruction write exceeds memory bound!");
        }
        c.mem[addr++] = static_cast<BYTE>((instr >> 8) & 0xFF);
        c.mem[addr++] = static_cast<BYTE>(instr & 0xFF);
    }
}
auto load_program_example_ibm(Chip8 &c) -> void {
    write_program_to_memory(c, load_ch8("assets/IBM Logo.ch8"));
}
auto load_program_example_corax_test_rom(Chip8 &c) -> void {
    // https://github.com/corax89/chip8-test-rom
    write_program_to_memory(c, load_ch8("assets/IBM Logo.ch8"));
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

auto initialise(Chip8 &c) -> void {
    { // Font data
        size_t loc = 0x050;
        for (auto &font : Constants::fontdata) {
            c.mem[loc] = font;
            ++loc;
        }
    } // Font data
    c.PC = 0x200;
}
} // namespace CHIP8