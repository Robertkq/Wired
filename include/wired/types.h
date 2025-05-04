#ifndef WIRED_TYPES_H
#define WIRED_TYPES_H

#include <cstdint>

namespace wired {

enum class message_strategy : uint8_t {
    normal = 1 << 0,
    immediate = 1 << 1
}; // enum class message_strategy

enum class execution_policy {
    blocking,
    non_blocking
}; // enum class execution_policy

struct selection_tag_0 {};
struct selection_tag_1 : selection_tag_0 {};
struct selection_tag_2 : selection_tag_1 {};

} // namespace wired

#endif // WIRED_TYPES_H