#ifndef WIRED_TYPES_H
#define WIRED_TYPES_H

#include <cstdint>

namespace wired {

enum class connection_strategy : uint8_t {
    reconnect = 1 << 0,
    once = 1 << 1,
    persistent = 1 << 2,
    lazy = 1 << 3,
    backoff = 1 << 4,
    none = 1 << 7
}; // enum class connection_strategy

enum class message_strategy : uint8_t {
    immediate = 1 << 0,
    compressed = 1 << 1,
    encrypted = 1 << 2,
    none = 1 << 7
}; // enum class message_strategy

} // namespace wired

#endif // WIRED_TYPES_H