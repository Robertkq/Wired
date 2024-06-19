#ifndef WIRED_MESSAGE_H
#define WIRED_MESSAGE_H

#include "wired/concepts.h"
#include "wired/types.h"
#include <algorithm>
#include <cstring>
#include <memory>

namespace wired {

template <typename T>
class connection;

template <typename T>
class message_header {
  public:
    message_header() : id_(), size_(0), timestamp_(0) {}
    message_header(T id) : id_(id), size_(0), timestamp_(0) {}

    T id() const { return id_; }
    uint64_t size() const { return size_; }
    uint64_t timestamp() const { return timestamp_; }

    void id(T id) { id_ = id; }
    void timestamp(uint64_t timestamp) { timestamp_ = timestamp; }
    void sync(uint64_t size) { size_ = size; }

  private:
    T id_;
    uint64_t size_;
    uint64_t timestamp_;
}; // class message_header

template <typename T>
class message_body {
  public:
    message_body() : data_() {}
    // TODO: Add constructors
    const std::vector<uint8_t>& data() const { return data_; }
    std::vector<uint8_t>& data() { return data_; }

  private:
    std::vector<uint8_t> data_;
}; // class message_body

template <typename T>
class message {
  public:
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;
    using message_header_t = message_header<T>;
    using message_body_t = message_body<T>;

  public:
    message(connection_ptr from, message_header_t head, message_body_t body)
        : from_(from), head_(head), body_(body) {}
    message() : from_(nullptr), head_(), body_() {}
    message(T id) : from_(nullptr), head_(id), body_() {}

    const connection_ptr& from() const { return from_; }
    connection_ptr& from() { return from_; }
    const message_header_t& head() const { return head_; }
    message_header_t& head() { return head_; }
    const message_body_t& body() const { return body_; }
    message_body_t& body() { return body_; }

    T id() const { return head_.id(); }
    void id(T id) { head_.id(id); }
    void reset();

    template <typename U>
    message& operator<<(U&& data);

    template <typename U>
    message& operator>>(U&& data);

  private:
    template <typename U>
    void write_selection(U&& data, selection_tag_2)
        requires has_wired_serializable<U>;
    template <typename U>
    void write_selection(U&& data, selection_tag_1)
        requires std::ranges::range<U>;
    template <typename U>
    void write_selection(U&& data, selection_tag_0);

    template <typename U>
    void read_selection(U& data, selection_tag_2)
        requires has_wired_deserializable<U>;
    template <typename U>
    void read_selection(U& data, selection_tag_1)
        requires std::ranges::range<U>;
    template <typename U>
    void read_selection(U& data, selection_tag_0);

    void sync() { head_.sync(body_.data().size()); }

  private:
    connection_ptr from_;
    message_header_t head_;
    message_body_t body_;
}; // class message

template <typename T>
void message<T>::reset() {
    head_ = message_header<T>();
    body_.data().clear();
}

template <typename T>
template <typename U>
message<T>& message<T>::operator<<(U&& data) {
    write_selection(std::forward<U>(data), selection_tag_2{});
    sync();
    return *this;
}

template <typename T>
template <typename U>
message<T>& message<T>::operator>>(U&& data) {
    read_selection(std::forward<U>(data), selection_tag_2{});
    sync();
    return *this;
}

/**
 * @brief Write a serializable object to the message
 * serializable objects are objects that have a wired_serialize method
 * checked via the has_wired_serializable concept
 *
 * @param selection_tag_2 1st priority tag
 */
template <typename T>
template <typename U>
void message<T>::write_selection(U&& data, selection_tag_2)
    requires has_wired_serializable<U>
{
    auto& msg = *this;
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
template <typename U>
void message<T>::write_selection(U&& data, selection_tag_1)
    requires std::ranges::range<U>
{
    using size_type = typename std::remove_reference_t<U>::size_type;
    auto& msg = *this;
    size_type size = std::ranges::size(data);
    for (const auto& item : data) {
        msg << item;
    }
    msg << static_cast<size_type>(size);
}

/**
 * @brief Write a single object to the message
 *
 * @param selection_tag_0 3rd priority tag
 */
template <typename T>
template <typename U>
void message<T>::write_selection(U&& data, selection_tag_0) {
    auto& msg = *this;
    auto& msg_vector = msg.body().data();
    auto size = msg_vector.size();
    msg_vector.resize(size + sizeof(U));
    std::memcpy(msg_vector.data() + size, &data, sizeof(U));
}

template <typename T>
template <typename U>
void message<T>::read_selection(U& data, selection_tag_2)
    requires has_wired_deserializable<U>
{
    std::vector<uint8_t> buffer;
    auto& msg = *this;
    msg >> buffer;
    data.wired_deserialize(std::move(buffer));
}

template <typename T>
template <typename U>
void message<T>::read_selection(U& data, selection_tag_1)
    requires std::ranges::range<U>
{
    using value_type = typename U::value_type;
    using size_type = typename U::size_type;

    auto& msg = *this;
    size_type size;
    msg >> size;
    data.clear();
    data.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        value_type item;
        msg >> item;
        data.push_back(item);
    }
    std::reverse(data.begin(), data.end());
}

template <typename T>
template <typename U>
void message<T>::read_selection(U& data, selection_tag_0) {
    auto& msg = *this;
    auto& vector = msg.body().data();
    std::memcpy(&data, (vector.data() + vector.size()) - sizeof(U), sizeof(U));
    vector.resize(vector.size() - sizeof(U));
}

} // namespace wired

#endif // WIRED_MESSAGE_H