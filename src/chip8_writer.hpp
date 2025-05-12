#pragma once

#include "chip8.hpp"
#include "types.hpp"

namespace CHIP8 {
class ProgramWriter {
public:
    WORD addr;

    /// Set the write cursor to a new memory address.
    void set_addr(WORD new_addr) { addr = new_addr; }

    /// Create a ProgramWriter for `chip`, starting at `start`.
    explicit ProgramWriter(Chip8 &chip, WORD start = Constants::rom_program_start)
        : c(chip), addr(start) {}

    /// (0NNN) Jump to system routine at NNN (ignored by most interpreters).
    void sys(WORD nnn) { write_encoded(Op::sys, 0, 0, 0, 0, nnn); }

    /// (00E0) Clear the display.
    void cls() { write_encoded(Op::cls); }

    /// (00EE) Return from a subroutine.
    void ret() { write_encoded(Op::ret); }

    /// (1NNN) Jump to address NNN.
    void jmp(WORD nnn) { write_encoded(Op::jmp, 0, 0, 0, 0, nnn); }

    /// (2NNN) Call subroutine at NNN.
    void call(WORD nnn) { write_encoded(Op::call_subroutine, 0, 0, 0, 0, nnn); }

    /// (BNNN) Jump to NNN plus V0.
    void jmp_offset(WORD nnn) { write_encoded(Op::jmp_offset, 0, 0, 0, 0, nnn); }

    /// (3XKK) Skip next instr. if VX == KK.
    void skip_eq(BYTE x, BYTE kk) { write_encoded(Op::skip_eq, x, 0, 0, kk); }

    /// (4XKK) Skip next instr. if VX != KK.
    void skip_not_eq(BYTE x, BYTE kk) { write_encoded(Op::skip_not_eq, x, 0, 0, kk); }

    /// (5XY0) Skip next instr. if VX == VY.
    void skip_eq_reg(BYTE x, BYTE y) { write_encoded(Op::skip_eq_register, x, y); }

    /// (9XY0) Skip next instr. if VX != VY.
    void skip_not_eq_reg(BYTE x, BYTE y) { write_encoded(Op::skip_not_eq_register, x, y); }

    /// (EX9E) Skip next instr. if key VX is pressed.
    void skip_pressed(BYTE x) { write_encoded(Op::skip_pressed, x); }

    /// (EXA1) Skip next instr. if key VX is not pressed.
    void skip_not_pressed(BYTE x) { write_encoded(Op::skip_not_pressed, x); }

    /// (6XKK) Set VX = KK.
    void ld_vx_byte(BYTE x, BYTE kk) { write_encoded(Op::set_register, x, 0, 0, kk); }

    /// (7XKK) Add KK to VX (no carry).
    void add_vx_byte(BYTE x, BYTE kk) { write_encoded(Op::add_to_register, x, 0, 0, kk); }

    /// (8XY0) Set VX = VY.
    void ld_vx_vy(BYTE x, BYTE y) { write_encoded(Op::copy_register, x, y); }

    /// (8XY1) Set VX = VX OR VY.
    void or_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_or, x, y); }

    /// (8XY2) Set VX = VX AND VY.
    void and_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_and, x, y); }

    /// (8XY3) Set VX = VX XOR VY.
    void xor_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_xor, x, y); }

    /// (8XY4) Add VY to VX, set VF = carry.
    void add_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_add, x, y); }

    /// (8XY5) Subtract VY from VX, set VF = NOT borrow.
    void sub_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_sub, x, y); }

    /// (8XY6) Shift VX right by 1, store LSB in VF.
    void shr_vx(BYTE x, BYTE y = 0) { write_encoded(Op::shr, x, y); }

    /// (8XY7) Set VX = VY - VX, set VF = NOT borrow.
    void subn_vx_vy(BYTE x, BYTE y) { write_encoded(Op::subn, x, y); }

    /// (8XYE) Shift VX left by 1, store MSB in VF.
    void shl_vx(BYTE x, BYTE y = 0) { write_encoded(Op::shl, x, y); }

    /// (CXKK) Set VX = random byte AND KK.
    void rnd_vx_byte(BYTE x, BYTE kk) { write_encoded(Op::get_random, x, 0, 0, kk); }

    /// (DXYN) Draw sprite at (VX, VY), N bytes tall; VF = collision flag.
    void drw(BYTE x, BYTE y, BYTE n) { write_encoded(Op::draw, x, y, n); }

    /// (ANNN) Set I = NNN.
    void ld_i_addr(WORD nnn) { write_encoded(Op::set_i, 0, 0, 0, 0, nnn); }

    /// (FX1E) Add VX to I.
    void add_i_vx(BYTE x) { write_encoded(Op::add_i, x); }

    /// (FX29) Set I to location of sprite for digit VX.
    void ld_f_vx(BYTE x) { write_encoded(Op::set_i_sprite, x); }

    /// (FX33) Store BCD of VX in memory at I, I+1, I+2.
    void bcd_vx(BYTE x) { write_encoded(Op::store_bcd, x); }

    /// (FX55) Store V0..VX in memory starting at I.
    void dump_vx(BYTE x) { write_encoded(Op::dump_registers, x); }

    /// (FX65) Read V0..VX from memory starting at I.
    void fill_vx(BYTE x) { write_encoded(Op::fill_registers, x); }

    /// (FX07) Set VX = delay timer.
    void ld_vx_dt(BYTE x) { write_encoded(Op::load_delay, x); }

    /// (FX0A) Wait for key press, then store in VX.
    void wait_key(BYTE x) { write_encoded(Op::wait_key, x); }

    /// (FX15) Set delay timer = VX.
    void set_delay(BYTE x) { write_encoded(Op::set_delay, x); }

    /// (FX18) Set sound timer = VX.
    void set_sound(BYTE x) { write_encoded(Op::set_sound, x); }

    /// Shift a block of the loaded program “forward” (toward higher addresses).
    /// \param start_pos  First byte of the block to move; 0 → 0x200.
    /// \param block_len  Length of the block in bytes; 0 → to end of RAM.
    /// \param n          Number of bytes to shift the block.
    ///
    /// The gap that opens between `start_pos` and `start_pos + n` is cleared
    /// (filled with 0x00).  Any data that would fall past the end of RAM is
    /// discarded and reported.
    auto shift_program_forward(std::size_t start_pos, std::size_t block_len, std::size_t n) -> void {
        if (n == 0) return;

        const std::size_t MEM_SIZE = c.mem.size();
        std::size_t start = (start_pos == 0) ? Constants::rom_program_start : start_pos;

        if (start >= MEM_SIZE) return;                      // start beyond RAM
        if (block_len == 0 || start + block_len > MEM_SIZE) // “move all”
            block_len = MEM_SIZE - start;
        if (block_len == 0) return;

        // Does the destination fit into RAM at all?
        if (start + n >= MEM_SIZE) {
            std::size_t lost_non_zero = 0;
            for (std::size_t i = start; i < start + block_len; ++i)
                if (c.mem[i] != 0x00) ++lost_non_zero;
            std::fill(c.mem.begin() + start,
                c.mem.begin() + start + block_len, 0x00);

            LOG_WARN("Shift of {} byte(s) from 0x{:03X} exceeds RAM – truncated "
                     "{} non-zero byte(s).  No data moved.",
                n, start, lost_non_zero);
            return;
        }

        // If only part of the block would survive, trim and warn.
        if (start + n + block_len > MEM_SIZE) {
            std::size_t allowed = MEM_SIZE - (start + n);
            std::size_t truncated = block_len - allowed;
            std::size_t lost_non_zero = 0;
            for (std::size_t i = start + allowed; i < start + block_len; ++i)
                if (c.mem[i] != 0x00) ++lost_non_zero;

            LOG_WARN("{} byte(s) at the end of the block would exceed RAM and "
                     "were discarded ({} non-zero).",
                truncated, lost_non_zero);

            block_len = allowed;
            if (block_len == 0) {
                std::fill(c.mem.begin() + start,
                    c.mem.begin() + start + n, 0x00);
                return;
            }
        }

        // Count non-zero 16-bit instructions in the part we keep.
        std::size_t non_zero_instr = 0;
        for (std::size_t i = start; i + 1 < start + block_len; i += 2)
            if ((c.mem[i] | c.mem[i + 1]) != 0x00)
                ++non_zero_instr;

        // Count bytes that will be overwritten at the destination.
        std::size_t overwritten_non_zero = 0;
        for (std::size_t i = start + n; i < start + n + block_len; ++i)
            if (c.mem[i] != 0x00) ++overwritten_non_zero;

        // Move, then clear the gap.
        std::copy_backward(c.mem.begin() + start,
            c.mem.begin() + start + block_len,
            c.mem.begin() + start + n + block_len);
        std::fill(c.mem.begin() + start,
            c.mem.begin() + start + n, 0x00);

        if (overwritten_non_zero)
            LOG_WARN("{} non-zero byte(s) were overwritten during the shift.",
                overwritten_non_zero);

        LOG_INFO("Block [{:#05X}, {:#05X}) shifted forward by {} byte(s); "
                 "{} non-zero instruction(s) moved.",
            start, start + block_len, n, non_zero_instr);
    }
    /// Zero out a contiguous range of instructions in memory,
    /// and report how many non-zero instructions were wiped.
    /// \param start_pos  First byte to clear; 0 → Constants::rom_program_start (0x200).
    /// \param length     Number of bytes to clear; if that overruns RAM, it’s truncated.
    auto zero_instructions(std::size_t start_pos,
        std::size_t length) -> void {
        const std::size_t MEM_SIZE = c.mem.size();
        std::size_t start = (start_pos == 0)
                                ? Constants::rom_program_start
                                : start_pos;
        if (start >= MEM_SIZE || length == 0) return;

        // clamp to end of RAM
        std::size_t end = start + length;
        if (end > MEM_SIZE) {
            std::size_t truncated = MEM_SIZE - start;
            LOG_WARN("zero_instructions: {}-byte clear at 0x{:03X} exceeds RAM, truncating to {} bytes",
                length, start, truncated);
            end = MEM_SIZE;
        }

        // count non-zero instructions (2 bytes each)
        std::size_t wiped_instructions = 0;
        for (std::size_t i = start; i + 1 < end; i += 2) {
            if ((c.mem[i] | c.mem[i + 1]) != 0) {
                ++wiped_instructions;
            }
        }
        // if an odd leftover byte, count it as a partial instruction
        if ((end - start) % 2 != 0 && c.mem[end - 1] != 0) {
            ++wiped_instructions;
        }

        // actually zero the bytes
        std::fill(c.mem.begin() + start, c.mem.begin() + end, 0x00);

        LOG_INFO("Cleared {} byte(s) in [0x{:03X}..0x{:03X}), wiped {} non-zero instruction(s)",
            end - start, start, end, wiped_instructions);
    }

private:
    Chip8 &c;

    /// Encode and write an opcode at `addr`, then advance `addr`.
    void write_encoded(Op id, WORD X = 0, WORD Y = 0,
        WORD N = 0, WORD NN = 0, WORD NNN = 0) {
        const auto *op = find_op(id);
        if (!op) throw std::runtime_error("Unknown opcode ID");
        WORD instr = op->encode(X, Y, N, NN, NNN);
        c.mem[addr++] = BYTE(instr >> 8);
        c.mem[addr++] = BYTE(instr & 0xFF);
    }
};
} // namespace CHIP8