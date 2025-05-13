/* danielsinkin97@gmail.com */
#pragma once

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

#include "chip8.hpp"
#include "chip8_writer.hpp"

namespace CHIP8::EXAMPLES {
auto disassemble() -> int {
    try {
        const fs::path roms_dir = "assets/code";

        if (!fs::exists(roms_dir) || !fs::is_directory(roms_dir)) {
            std::cerr << "Directory not found: " << roms_dir << '\n';
            return 1;
        }

        for (const auto &entry : fs::directory_iterator(roms_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".ch8") {
                const std::string &filepath = entry.path().string();
                try {
                    auto out_path = CHIP8::disassemble_rom_to_file(filepath);
                    std::cout << "Disassembled " << entry.path().filename() << " -> " << out_path << '\n';
                } catch (const std::exception &e) {
                    std::cerr << "Failed to disassemble " << filepath << ": " << e.what() << '\n';
                }
            }
        }

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error during disassembly: " << e.what() << '\n';
        return 1;
    }
}
auto test_suite(Chip8 &c, int idx) -> void {
    load_program_from_file(c, CONSTANTS::fp_code_test_suite.at(idx));
}

auto ibm_with_sound(Chip8 &c) -> void {
    load_program_from_file(c, CONSTANTS::fp_code_ibm_logo);
    ProgramWriter pw(c, 0x228);
    pw.jmp(0x300);
    pw.addr = 0x300;
    pw.ld_vx_byte(0x5, 40);
    pw.set_delay(0x5);
    pw.ld_vx_byte(0x5, 40);
    pw.set_sound(0x5);
    pw.ld_vx_dt(0x5);
    pw.skip_eq(0x5, 0x0);
    pw.jmp(pw.addr - 0x4);
    pw.wait_key(0x6);
    pw.jmp(0x200);
}
} // namespace CHIP8::EXAMPLES