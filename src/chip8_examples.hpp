/* danielsinkin97@gmail.com */
#pragma once

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

#include "chip8.hpp"

auto example_dissamble() -> int {
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