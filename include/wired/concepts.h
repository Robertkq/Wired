#ifndef WIRED_CONCEPTS_H
#define WIRED_CONCEPTS_H

#include <concepts>

namespace wired {

template <typename T>
concept has_wired_serializable = requires(T t) {
    { t.wired_serialize() } -> std::same_as<std::vector<uint8_t>>;
};

} // namespace wired

#endif