#ifndef WIRED_MESSAGE_IO_H
#define WIRED_MESSAGE_IO_H

#include "message.h"
#include <concepts>

namespace wired {

template <typename T>
concept has_wired_serializable = requires(T t) {
    { t.wired_serialize() } -> std::same_as<std::vector<uint8_t>>;
};

template <typename T>
void write_selection(T&& data)
    requires has_wired_serializable<T>
{
    std::cout << "Writing serializable object" << std::endl;
}

template <typename T>
void write_selection(T&& data)
    requires std::ranges::range<T>
{
    std::cout << "Writing range" << std::endl;
}

template <typename T>
void write_selection(T&& data) {
    std::cout << "Writing something random" << std::endl;
}

template <typename T, typename U>
message<T>& operator<<(message<T>& msg, U&& data) {
    write_selection(std::forward<U>(data));
    return msg;
}

} // namespace wired

#endif // WIRED_MESSAGE_IO_H