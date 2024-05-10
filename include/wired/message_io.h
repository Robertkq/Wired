#ifndef WIRED_MESSAGE_IO_H
#define WIRED_MESSAGE_IO_H

#include "wired/concepts.h"
#include "wired/message.h"
#include <concepts>

namespace wired {

/**
 * @brief Write a serializable object to the message
 * serializable objects are objects that have a wired_serialize method
 * checked via the has_wired_serializable concept
 *
 * @param selection_tag_2 1st priority tag
 */
template <typename T>
void write_selection(message<T>& msg, T&& data, selection_tag_2)
    requires has_wired_serializable<T>
{
    std::cout << "Writing serializable object" << std::endl;
    msg << data.wired_serialize();
}

/**
 * @brief Write a range of data to the message
 * ranges are objects that have a begin and end method
 * checked via the std::ranges::range concept
 *
 * @param selection_tag_1 2nd priority tag
 *
 */
template <typename T>
void write_selection(message<T>& msg, T&& data, selection_tag_1)
    requires std::ranges::range<T>
{
    std::cout << "Writing range" << std::endl;
    uint64_t size = std::ranges::size(data);
    for (const auto& item : data) {
        msg << item;
    }
    msg << size;
}

/**
 * @brief Write a single object to the message
 *
 * @param selection_tag_0 3rd priority tag
 */
template <typename T>
void write_selection(message<T>& msg, T&& data, selection_tag_0) {
    auto& msg_vector = msg.body().data();
    auto size = msg_vector.size();
    msg_vector.resize(size + sizeof(T));
    std::memcpy(msg_vector.data() + size, &data, sizeof(T));
    msg.sync();
}

template <typename T, typename U>
message<T>& operator<<(message<T>& msg, U&& data) {
    write_selection(std::forward<U>(data), selection_tag_2);
    return msg;
}

template <typename T>
void read_selection(T& data, selection_tag_2)
    requires has_wired_serializable<T>
{
    std::cout << "Reading serializable object" << std::endl;
    std::vector<uint8_t> buffer;
    data.wired_deserialize(buffer);
}

template <typename T, typename U>
message<T>& operator>>(message<T>& msg, U& data) {
    read_selection(std::forward<U>(data), selection_tag_2);
    return msg;

} // namespace wired

#endif // WIRED_MESSAGE_IO_H