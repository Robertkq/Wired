#ifndef WIRED_TOOLS_LOG_H
#define WIRED_TOOLS_LOG_H

#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace wired {

constexpr uint8_t LOG_LEVEL_COUNT = 6;

enum log_level {
    LOG_DISABLE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,
    LOG_CRITICAL = 5
};

inline std::array<const char*, LOG_LEVEL_COUNT> log_level_names = {
    "DISABLE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};

inline log_level current_lvl = LOG_DISABLE;

inline std::mutex log_mutex;

#define WIRED_LOG_LEVEL(lvl) wired::current_lvl = lvl;

#define WIRED_LOG_MESSAGE(lvl, msg, ...)                                       \
    wired::log_message_function(lvl, __FILE__, __LINE__, msg, ##__VA_ARGS__)

inline std::string shorten_filename(const char* file) {
    std::string filename(file);
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = filename.substr(lastSlash + 1);
    }
    return filename;
}

template <typename... Args>
void log_message_function(log_level lvl, const char* file, int line,
                          const char* msg, Args&&... args) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ostringstream calling_thread;
    calling_thread << std::this_thread::get_id();
    std::string short_filename = shorten_filename(file);
    if (current_lvl <= lvl && current_lvl != LOG_DISABLE) {
        std::cout << std::format("[{}:{}][{}][TID: {}]: ", short_filename, line,
                                 log_level_names[static_cast<uint8_t>(lvl)],
                                 calling_thread.str());
        std::cout << std::vformat(msg, std::make_format_args(args...));
        std::cout << std::endl;
    }
}

} // namespace wired

#endif // WIRED_TOOLS_LOG_H