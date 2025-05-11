// danielsinkin97@gmail.com
#pragma once

#include <array>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <stack>
#include <string>

using BYTE = uint8_t;
using PIXEL = uint8_t; // In principle we could use bool but bit arrays are slower, memory tradeoff worth it
using WORD = uint16_t;

auto print_binary_word = [](WORD value) {
    std::bitset<16> bits(value);
    for (int i = 15; i >= 0; --i) {
        std::cout << bits[i];
        if (i % 4 == 0 && i != 0) std::cout << "_"; // Add separator every 4 bits
    }
};

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

struct Instruction {
    WORD val;

    auto op() const -> BYTE { return static_cast<BYTE>((val & 0xF000) >> 12); }
    auto X() const -> BYTE { return static_cast<BYTE>((val & 0x0F00) >> 8); }
    auto Y() const -> BYTE { return static_cast<BYTE>((val & 0x00F0) >> 4); }
    auto NNN() const -> WORD { return val & 0x0FFF; }
    auto NN() const -> BYTE { return static_cast<BYTE>(val & 0x00FF); }
    auto N() const -> BYTE { return static_cast<BYTE>(val & 0x000F); }

    static auto jump(WORD NNN) -> Instruction {
        if (NNN > 0x0FFF) {
            throw std::invalid_argument("NNN out of range: must be <= 0x0FFF");
        }
        return Instruction{static_cast<WORD>(0x1000 + NNN)};
    }
    static auto clear_screen() -> Instruction {
        return Instruction{static_cast<WORD>(0x00EE)};
    }
    static auto set_index_register(WORD NNN) -> Instruction {
        if (NNN > 0x0FFF) {
            throw std::invalid_argument("NNN out of range: must be <= 0x0FFF");
        }
        return Instruction{static_cast<WORD>(0xA000 + NNN)};
    }
    static auto set_register(BYTE X, BYTE NN) -> Instruction {
        if (X > 0x0F) {
            throw std::invalid_argument("Register index X out of range: must be <= 0x0F");
        }
        return Instruction{static_cast<WORD>(0x6000 | (X << 8) | NN)};
    }
    static auto add_to_reg(BYTE X, BYTE NN) -> Instruction {
        if (X > 0x0F) {
            throw std::invalid_argument("Register index X out of range: must be <= 0x0F");
        }
        return Instruction{static_cast<WORD>(0x7000 | (X << 8) | NN)};
    }
    // 0xDXYN
    static auto draw(BYTE X, BYTE Y, BYTE N) -> Instruction {
        if (X > 0x0F) throw std::invalid_argument("Register index X out of range: must be <= 0x0F");
        if (Y > 0x0F) throw std::invalid_argument("Register index Y out of range: must be <= 0x0F");
        if (N > 0x0F) throw std::invalid_argument("N value out of range: must be <= 0x0F");
        return Instruction{static_cast<WORD>(0xD000 | (X << 8) | (Y << 4) | N)};
    }

    void write_to_mem(std::array<BYTE, 4096> &mem, WORD &addr) const {
        if (addr + 1 >= mem.size()) {
            throw std::out_of_range("Instruction write exceeds memory bounds");
        }
        mem[addr++] = static_cast<BYTE>((val >> 8) & 0xFF);
        mem[addr++] = static_cast<BYTE>(val & 0xFF);
    }
};

auto describe_instruction(const Instruction &instr) -> std::optional<std::string> {
    switch (instr.op()) {
    case 0x0:
        if (instr.val == 0x00EE) {
            return "Return from subroutine (RET)";
        }
        break;
    case 0x1:
        return std::format("Jump to address 0x{:03X}", instr.NNN());
    case 0x6:
        return std::format("Set V{:X} = 0x{:02X}", instr.X(), instr.NN());
    case 0x7:
        return std::format("Add 0x{:02X} to V{:X}", instr.NN(), instr.X());
    case 0xA:
        return std::format("Set index register I = 0x{:03X}", instr.NNN());
    case 0xD:
        return std::format("Draw sprite at (V{:X}, V{:X}) with height {}", instr.X(), instr.Y(), instr.N());
    }
    return std::nullopt;
}

auto dump_memory(Chip8 &c) -> void {
    std::ofstream file("memory.bin", std::ios::binary);
    file.write(
        reinterpret_cast<const char *>(c.mem.data()),
        static_cast<std::streamsize>(c.mem.size()));
}

auto clear_display(Chip8 &c) -> void {
    for (auto &row : c.display) {
        std::fill(row.begin(), row.end(), 0);
    }
}

/* Fetches instruction and increments PC by 2. */
auto fetch_instruction(Chip8 &c) -> Instruction {
    BYTE high = c.mem[c.PC++];
    BYTE low = c.mem[c.PC++];
    WORD opcode = (high << 8) | low;
    LOG_INFO("Fetched opcode: {:#06x}", opcode);
    return Instruction{opcode};
}

auto execute_instruction(Chip8 &c, const Instruction &instr) -> void {
    switch (instr.op()) {
    case 0x0: { // {0x000E, 0x00EE, 0x0NNN}
        std::cout << "Machine Code Routine\n";
        if (instr.Y() == 0x0E) { // 0x000E
            std::cout << "Clear the Display\n";
            clear_display(c);
        } else if (instr.Y() == 0xEE) { // 0x00EE
            std::cout << "Return from Subroutine\n";
            // pop from stack (and adjust for the upcoming PC increment)
            c.PC = c.stack[c.stack_pointer] - 1;
            --c.stack_pointer;
        } else { // 0x0NNN
            std::cout << "Call Machine Code Routine at Address ";
            std::cout << " (Will not implement)\n";
            PANIC_UNDEFINED(instr.val);
        }
        break;
    }
    case 0x1: { // 0x1NNN
        WORD NNN = instr.NNN();
        LOG_INFO("Jump to address {:#06x}", NNN);
        if (NNN < 0x200) {
            PANIC(std::format("Tried to jump to protected location {0}", NNN));
        }
        c.PC = instr.NNN();
        break;
    }
    case 0x6: { // 0x6XNN
        LOG_INFO("Set register");
        c.VX[instr.X()] = instr.NN();
        break;
    }
    case 0x7: { // 0x7XNN
        LOG_INFO("Add Value to Register");
        // TODO: What happens on overflow?
        c.VX[instr.X()] += instr.NN();
        break;
    }
    case 0xA: { // 0xANNN
        LOG_INFO("Set I to Address");
        c.I = instr.NNN();
        break;
    }
    case 0xD: { // 0xDXYN
        LOG_INFO("Draw");
        BYTE VX = c.VX[instr.X()] % 64;
        BYTE VY = c.VX[instr.Y()] % 32;
        c.VX[0xF] = 0;

        BYTE N = instr.N();
        for (size_t row_idx = 0; row_idx < N; ++row_idx) {
            if (VY + row_idx >= c.display.size()) break;
            BYTE sprite_byte = c.mem[c.I + row_idx];
            for (size_t col = 0; col < 8; ++col) {
                PIXEL pixel = (sprite_byte >> (7 - col)) & 1;
                PIXEL curr = c.display[VY + row_idx][VX + col];
                if (curr && pixel) {
                    c.VX[0xF] = 1;
                }
                c.display[VY + row_idx][VX + col] ^= pixel;
            }
        }
        break;
    }
    default: {
        PANIC_NOT_IMPLEMENTED(instr.val);
        break;
    }
    }
}

auto fetch_and_execute(Chip8 &c) -> void {
    execute_instruction(c, fetch_instruction(c));
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

auto write_program_to_memory(Chip8 &c, std::vector<WORD> data) -> void {
    WORD addr = 0x200;
    for (const auto &word : data) {
        Instruction{word}.write_to_mem(c.mem, addr);
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
    WORD addr = 0x200;
    Instruction::clear_screen().write_to_mem(c.mem, addr);
    Instruction::jump(0x212).write_to_mem(c.mem, addr);

    addr = 0x212;
    Instruction::set_index_register(0x050).write_to_mem(c.mem, addr);
    Instruction::set_register(0xB, 0x37).write_to_mem(c.mem, addr);
    Instruction::add_to_reg(0xC, 0x12).write_to_mem(c.mem, addr);
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