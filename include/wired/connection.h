#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include "wired/message.h"
#include "wired/ts_deque.h"
#include "wired/types.h"

#include <asio.hpp>

namespace wired {

template <typename T>
class connection {
  public:
    using message_t = message<T>;

  public:
    connection(asio::io_context& io_context, asio::ip::tcp::socket socket);
    connection(const connection& other) = delete;
    connection(connection&& other);
    virtual ~connection();

    connection& operator=(const connection&& other) = delete;
    connection& operator=(connection&& other);

    bool is_connected() const;
    bool send(const message_t& msg);

    void run();

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    ts_deque<message_t> messages_;
    message_t aux_message_;
};

} // namespace wired

#endif