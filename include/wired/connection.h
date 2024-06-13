#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include "wired/message.h"
#include "wired/ts_deque.h"
#include "wired/types.h"

#include <asio.hpp>
#include <iostream>

namespace wired {

template <typename T>
class connection {
  public:
    using message_t = message<T>;

  public:
    connection(asio::io_context& io_context, ts_deque<message_t>& messages_in);
    connection(const connection& other) = delete;
    connection(connection&& other);
    virtual ~connection();

    connection& operator=(const connection&& other) = delete;
    connection& operator=(connection&& other);

    bool is_connected() const;
    bool send(const message_t& msg);
    bool connect(const asio::ip::tcp::endpoint& endpoints);

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    ts_deque<message_t>& messages_in_;
    message_t aux_message_;
};

template <typename T>
connection<T>::connection(asio::io_context& io_context,
                          ts_deque<message_t>& messages_in)
    : io_context_(io_context), socket_(asio::ip::tcp::socket(io_context)),
      messages_in_(messages_in), aux_message_() {}

template <typename T>
bool connection<T>::connect(const asio::ip::tcp::endpoint& endpoints) {
    asio::async_connect(socket_, endpoints,
                        [this](asio::error_code ec, asio::ip::tcp::endpoint) {
                            if (ec) {
                                std::cerr << "BAD";
                            }
                        });
}

} // namespace wired

#endif