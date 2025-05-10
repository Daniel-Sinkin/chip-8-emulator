/* danielsinkin97@gmail.com */
#pragma once

#include "global.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

enum class LogLevel {
    Info,
    Warn,
    Error
};

inline auto log(LogLevel level, const std::string &msg) -> void {
    switch (level) {
    case LogLevel::Info:
        std::cout << "[INFO]  " << msg << "\n";
        break;
    case LogLevel::Warn:
        std::cout << "[WARN]  " << msg << "\n";
        break;
    case LogLevel::Error:
        std::cerr << "[ERROR] " << msg << "\n";
        break;
    }
}

#define LOG_INFO(msg) log(LogLevel::Info, msg)
#define LOG_WARN(msg) log(LogLevel::Warn, msg)
#define LOG_ERR(msg) log(LogLevel::Error, msg)

[[noreturn]] inline auto
panic_impl(const std::string &msg, const char *file, int line) -> void {
    LOG_ERR("PANIC: '" + msg + "' at '" + file + ":" + std::to_string(line) + "'");
    std::exit(EXIT_FAILURE);
}
#define PANIC(msg) panic_impl((msg), __FILE__, __LINE__)

[[noreturn]] inline auto
PANIC_not_implemented(uint16_t opcode) -> void {
    PANIC(std::format("Instruction not Implemented: {:#06x}", opcode));
}

// For instructions that are not defined in the CHIP-8 spec
[[noreturn]] inline auto
PANIC_undefined(uint16_t opcode) -> void {
    PANIC(std::format("Undefined instruction: {:#06x}", opcode));
}