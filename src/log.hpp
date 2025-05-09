/* danielsinkin97@gmail.com */
#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

#ifdef __unix__
#include <execinfo.h>
#include <unistd.h>
#define HAS_STACKTRACE 1
#elif defined(__APPLE__)
#include <execinfo.h>
#include <unistd.h>
#define HAS_STACKTRACE 1
#else
#define HAS_STACKTRACE 0
#endif

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

inline void print_stacktrace() {
#if HAS_STACKTRACE
    void *callstack[128];
    int frames = backtrace(callstack, 128);
    char **symbols = backtrace_symbols(callstack, frames);
    LOG_ERR("Stack trace:");
    for (int i = 1; i < frames; ++i) {
        LOG_ERR(std::string("  ") + symbols[i]);
    }
    free(symbols);
#else
    LOG_ERR("(stack trace not available on this platform)");
#endif
}

[[noreturn]] inline auto panic(const std::string &message) -> void {
    log(LogLevel::Error, "PANIC: " + message);
    print_stacktrace();
    std::exit(EXIT_FAILURE);
}