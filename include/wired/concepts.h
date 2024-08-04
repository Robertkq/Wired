#ifndef WIRED_CONCEPTS_H
#define WIRED_CONCEPTS_H

#include <concepts>
#include <vector>

namespace wired {

template <typename T>
concept has_wired_serializable = requires(T t) {
    { t.wired_serialize() } -> std::same_as<std::vector<uint8_t>>;
};

template <typename T>
concept has_wired_deserializable = requires(T t) {
    { t.wired_deserialize(std::vector<uint8_t>()) } -> std::same_as<void>;
};

template <typename T>
concept is_wired_serializable =
    has_wired_serializable<T> && has_wired_deserializable<T>;

} // namespace wired

#endif