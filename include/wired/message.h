#ifndef WIRED_MESSAGE_H
#define WIRED_MESSAGE_H

#include <asio.hpp>

#include "wired/concepts.h"
#include "wired/types.h"
#include "wired/tools/log.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

namespace wired {

template <typename T>
class connection;

template <typename T>
class message_header {
  public:
    message_header() : id_(), size_(0), timestamp_(0) {}
    explicit message_header(T id) : id_(id), size_(0), timestamp_(0) {}
    message_header(const message_header& other)
        : id_(other.id_), size_(other.size_), timestamp_(other.timestamp_) {}
    message_header(message_header&& other) noexcept;

    message_header& operator=(const message_header& other);
    message_header& operator=(message_header&& other) noexcept;

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
message_header<T>::message_header(message_header&& other) noexcept
    : id_(std::move(other.id_)), size_(std::move(other.size_)),
      timestamp_(std::move(other.timestamp_)) {
    other.size_ = 0;
    other.timestamp_ = 0;
}

template <typename T>
message_header<T>& message_header<T>::operator=(const message_header& other) {
    id_ = other.id_;
    size_ = other.size_;
    timestamp_ = other.timestamp_;
    return *this;
}

template <typename T>
message_header<T>&
message_header<T>::operator=(message_header&& other) noexcept {
    id_ = std::move(other.id_);
    size_ = std::move(other.size_);
    timestamp_ = std::move(other.timestamp_);
    other.size_ = 0;
    other.timestamp_ = 0;
    return *this;
}

template <typename T>
class message_body {
  public:
    message_body() : data_() {}
    message_body(const message_body& other) : data_(other.data_) {}
    message_body(message_body&& other) noexcept
        : data_(std::move(other.data_)) {}

    message_body& operator=(const message_body& other);
    message_body& operator=(message_body&& other) noexcept;

    const std::vector<uint8_t>& data() const { return data_; }
    std::vector<uint8_t>& data() { return data_; }

  private:
    std::vector<uint8_t> data_;
}; // class message_body

template <typename T>
message_body<T>& message_body<T>::operator=(const message_body& other) {
    data_ = other.data_;
    return *this;
}

template <typename T>
message_body<T>& message_body<T>::operator=(message_body&& other) noexcept {
    data_ = std::move(other.data_);
    return *this;
}

template <typename T>
class message {
  public:
    using value_type = T;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;
    using message_header_t = message_header<T>;
    using message_body_t = message_body<T>;

  public:
    message(connection_ptr from, message_header_t head, message_body_t body)
        : from_(from), head_(head), body_(body) {}
    message() : from_(nullptr), head_(), body_() {}
    explicit message(T id) : from_(nullptr), head_(id), body_() {}
    message(const message& other);
    message(message&& other) noexcept;

    message& operator=(const message& other);
    message& operator=(message&& other) noexcept;

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
    message& operator>>(U& data);

  private:
    template <typename U>
    void write_selection(U&& data,
                         selection_tag_2) requires has_wired_serializable<U>;
    template <typename U>
    void write_selection(U&& data,
                         selection_tag_1) requires std::ranges::range<U>;
    template <typename U>
    void write_selection(U&& data, selection_tag_0);

    template <typename U>
    void read_selection(U& data,
                        selection_tag_2) requires has_wired_deserializable<U>;
    template <typename U>
    void read_selection(U& data,
                        selection_tag_1) requires std::ranges::range<U>;
    template <typename U>
    void read_selection(U& data, selection_tag_0);

    void sync() { head_.sync(body_.data().size()); }

  private:
    connection_ptr from_;
    message_header_t head_;
    message_body_t body_;
}; // class message

template <typename T>
message<T>::message(const message& other)
    : from_(other.from_), head_(other.head_), body_(other.body_) {}

template <typename T>
message<T>::message(message&& other) noexcept
    : from_(std::move(other.from_)), head_(std::move(other.head_)),
      body_(std::move(other.body_)) {}

template <typename T>
message<T>& message<T>::operator=(const message& other) {
    from_ = other.from_;
    head_ = other.head_;
    body_ = other.body_;
    return *this;
}

template <typename T>
message<T>& message<T>::operator=(message&& other) noexcept {
    from_ = std::move(other.from_);
    head_ = std::move(other.head_);
    body_ = std::move(other.body_);
    return *this;
}

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
message<T>& message<T>::operator>>(U& data) {
    read_selection(data, selection_tag_2{});
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
void message<T>::write_selection(U&& data, selection_tag_2) requires
    has_wired_serializable<U> {
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
void message<T>::write_selection(
    U&& data, selection_tag_1) requires std::ranges::range<U> {
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
void message<T>::read_selection(U& data, selection_tag_2) requires
    has_wired_deserializable<U> {
    std::vector<uint8_t> buffer;
    auto& msg = *this;
    msg >> buffer;
    data.wired_deserialize(std::move(buffer));
}

template <typename T>
template <typename U>
void message<T>::read_selection(
    U& data, selection_tag_1) requires std::ranges::range<U> {
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