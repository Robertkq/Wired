#ifndef WIRED_TYPES_H
#define WIRED_TYPES_H

#include <cstdint>

namespace wired {

enum class connection_strategy : uint8_t {
    once = 1 << 0,
    reconnect = 1 << 1,
    persistent = 1 << 2,
    lazy = 1 << 3,
    backoff = 1 << 4,
    none = 1 << 7
}; // enum class connection_strategy

enum class message_strategy : uint8_t {
    normal = 1 << 0,
    immediate = 1 << 1,
    compressed = 1 << 2,
    encrypted = 1 << 3
}; // enum class message_strategy

enum class run_strategy : uint8_t {
    blocking = 1 << 0,
    non_blocking = 1 << 1
}; // enum class run_strategy

} // namespace wired

#endif // WIRED_TYPES_H