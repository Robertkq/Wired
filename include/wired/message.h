#ifndef WIRED_MESSAGE_H
#define WIRED_MESSAGE_H

#include "wired/connection.h"
#include <memory>

namespace wired {

template <typename T>
class message_header {
  public:
  private:
    T id_;
    uint64_t size_;
    uint64_t timestamp_;
}; // class message_header

template <typename T>
class message_body {
  public:
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
  private:
    connection_ptr from_;
    message_header_t head_;
    message_body_t body_;
}; // class message

} // namespace wired

#endif // WIRED_MESSAGE_H