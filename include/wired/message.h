#ifndef WIRED_MESSAGE_H
#define WIRED_MESSAGE_H

#include "wired/connection.h"
#include <memory>

namespace wired {

template <typename T>
class message_header {
  public:
    // TODO: Add constructors
    T id() const { return id_; }
    uint64_t size() const { return size_; }
    uint64_t timestamp() const { return timestamp_; }

  private:
    T id_;
    uint64_t size_;
    uint64_t timestamp_;
}; // class message_header

template <typename T>
class message_body {
  public:
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

    const connection_ptr& from() const { return from_; }
    connection_ptr& from() { return from_; }
    const message_header_t& head() const { return head_; }
    message_header_t& head() { return head_; }
    const message_body_t& body() const { return body_; }
    message_body_t& body() { return body_; }

    void sync() { head_.size() = body_.data().size(); }

  private:
    connection_ptr from_;
    message_header_t head_;
    message_body_t body_;
}; // class message

} // namespace wired

#endif // WIRED_MESSAGE_H