#pragma once

#include "chip8.hpp"
#include "types.hpp"

namespace CHIP8 {
class ProgramWriter {
public:
    WORD addr;

    void set_addr(WORD new_addr) { addr = new_addr; }
    explicit ProgramWriter(Chip8 &chip, WORD start = Constants::rom_program_start)
        : c(chip), addr(start) {}

    void sys(WORD nnn) { write_encoded(Op::sys, 0, 0, 0, 0, nnn); }
    void cls() { write_encoded(Op::cls); }
    void ret() { write_encoded(Op::ret); }                                                  // 00EE
    void jmp(WORD nnn) { write_encoded(Op::jmp, 0, 0, 0, 0, nnn); }                         // 1NNN
    void call(WORD nnn) { write_encoded(Op::call_subroutine, 0, 0, 0, 0, nnn); }            // 2NNN
    void jmp_offset(WORD nnn) { write_encoded(Op::jmp_offset, 0, 0, 0, 0, nnn); }           // BNNN
    void skip_eq(BYTE x, BYTE kk) { write_encoded(Op::skip_eq, x, 0, 0, kk); }              // 3xkk
    void skip_not_eq(BYTE x, BYTE kk) { write_encoded(Op::skip_not_eq, x, 0, 0, kk); }      // 4xkk
    void skip_eq_reg(BYTE x, BYTE y) { write_encoded(Op::skip_eq_register, x, y); }         // 5xy0
    void skip_not_eq_reg(BYTE x, BYTE y) { write_encoded(Op::skip_not_eq_register, x, y); } // 9xy0
    void skip_pressed(BYTE x) { write_encoded(Op::skip_pressed, x); }                       // Ex9E
    void skip_not_pressed(BYTE x) { write_encoded(Op::skip_not_pressed, x); }               // ExA1
    void ld_vx_byte(BYTE x, BYTE kk) { write_encoded(Op::set_register, x, 0, 0, kk); }      // 6xkk
    void add_vx_byte(BYTE x, BYTE kk) { write_encoded(Op::add_to_register, x, 0, 0, kk); }  // 7xkk
    void ld_vx_vy(BYTE x, BYTE y) { write_encoded(Op::copy_register, x, y); }               // 8xy0
    void or_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_or, x, y); }                     // 8xy1
    void and_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_and, x, y); }                   // 8xy2
    void xor_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_xor, x, y); }                   // 8xy3
    void add_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_add, x, y); }                   // 8xy4
    void sub_vx_vy(BYTE x, BYTE y) { write_encoded(Op::math_sub, x, y); }                   // 8xy5
    void shr_vx(BYTE x, BYTE y = 0) { write_encoded(Op::shr, x, y); }                       // 8xy6
    void subn_vx_vy(BYTE x, BYTE y) { write_encoded(Op::subn, x, y); }                      // 8xy7
    void shl_vx(BYTE x, BYTE y = 0) { write_encoded(Op::shl, x, y); }                       // 8xyE
    void rnd_vx_byte(BYTE x, BYTE kk) { write_encoded(Op::get_random, x, 0, 0, kk); }       // Cxkk
    void drw(BYTE x, BYTE y, BYTE n) { write_encoded(Op::draw, x, y, n); }                  // Dxyn
    void ld_i_addr(WORD nnn) { write_encoded(Op::set_i, 0, 0, 0, 0, nnn); }                 // ANNN
    void add_i_vx(BYTE x) { write_encoded(Op::add_i, x); }                                  // Fx1E
    void ld_f_vx(BYTE x) { write_encoded(Op::set_i_sprite, x); }                            // Fx29
    void bcd_vx(BYTE x) { write_encoded(Op::store_bcd, x); }                                // Fx33
    void dump_vx(BYTE x) { write_encoded(Op::dump_registers, x); }                          // Fx55
    void fill_vx(BYTE x) { write_encoded(Op::fill_registers, x); }                          // Fx65
    void ld_vx_dt(BYTE x) { write_encoded(Op::load_delay, x); }                             // Fx07
    void ld_vx_k(BYTE x) { write_encoded(Op::wait_key, x); }                                // Fx0A
    void set_delay(BYTE x) { write_encoded(Op::set_delay, x); }                             // Fx15
    void set_sound(BYTE x) { write_encoded(Op::set_sound, x); }                             // Fx18

    /* Make room for new code by moving the existing program forward.
    * Everything from [addr … end-of-RAM) is shifted upward by n bytes.
    * The gap [addr … addr+n) is cleared to 0x00.
    * addr itself does NOT change.
    * If we are asked to write in the interpreter area (< 0x200) we warn.
    * If bytes fall off the end of RAM and at least one of them was non-zero
        we warn about data loss.
    *  We also log how many *instructions* (i.e. 2-byte words that contained at
        least one non-zero byte) were actually moved.
    */
    auto shift_program_forward(size_t n) -> void {
        if (n == 0) return;

        if (addr < Constants::rom_program_start) {
            LOG_WARN("ProgramWriter::shift_program_forward(): addr (0x{:03X}) "
                     "is below rom_program_start (0x{:03X})",
                addr, Constants::rom_program_start);
        }

        size_t MEM_SIZE = c.mem.size();
        if (addr >= MEM_SIZE) return; // cursor already outside RAM – silently ignore

        // When the shift is bigger than the free space, everything after addr dies.
        if (n >= MEM_SIZE - addr) {
            std::size_t lost_non_zero = 0;
            for (size_t i = addr; i < MEM_SIZE; ++i)
                if (c.mem[i] != 0x00) ++lost_non_zero;

            std::fill(c.mem.begin() + addr, c.mem.end(), 0x00);

            LOG_WARN("Shift of {} byte(s) exceeds remaining RAM – truncated {} "
                     "non-zero byte(s). No data moved.",
                n, lost_non_zero);
            return;
        }

        // Count number of moved instructions
        size_t non_zero_instr = 0;
        for (size_t i = addr; i + 1 < MEM_SIZE - n; i += 2)
            if ((c.mem[i] | c.mem[i + 1]) != 0x00)
                ++non_zero_instr;

        // Count truncated instructions
        size_t lost_non_zero = 0;
        for (size_t i = MEM_SIZE - n; i < MEM_SIZE; ++i)
            if (c.mem[i] != 0x00) ++lost_non_zero;

        // Move Data
        std::copy_backward(c.mem.begin() + addr, c.mem.end() - n, c.mem.end());

        // Zero the old memory
        std::fill(c.mem.begin() + addr,
            c.mem.begin() + addr + n,
            0x00);

        if (lost_non_zero)
            LOG_WARN("{} non-zero byte(s) truncated while shifting the program "
                     "forward by {} byte(s).",
                lost_non_zero, n);

        LOG_INFO("Program shifted forward by {} byte(s) from 0x{:03X}; "
                 "{} non-zero instruction(s) moved.",
            n, addr, non_zero_instr);
    }

private:
    Chip8 &c;

    // central helper that does the actual write
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