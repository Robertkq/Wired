#ifndef WIRED_TOOLS_LOG_H
#define WIRED_TOOLS_LOG_H

#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <mutex>

enum log_level {
    LOG_DISABLE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,
    LOG_CRITICAL = 5
};

inline std::array<const char*, 6> log_level_names = {
    "DISABLE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};

inline log_level current_lvl = LOG_DISABLE;

inline std::mutex log_mutex;

#define LOG_LEVEL(lvl) current_lvl = lvl;

#define LOG_MESSAGE(lvl, msg, ...)                                             \
    log_message_function(lvl, __FILE__, __LINE__, msg, ##__VA_ARGS__)

template <typename... Args>
void log_message_function(log_level lvl, const char* file, int line,
                          const char* msg, Args&&... args) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (current_lvl >= lvl) {
        std::cerr << std::format("[{}:{}][{}]: ", file, line,
                                 log_level_names[static_cast<uint8_t>(lvl)]);
        std::cerr << std::vformat(msg, std::make_format_args(args...));
        std::cerr << std::endl;
    }
}

#endif // WIRED_TOOLS_LOG_H