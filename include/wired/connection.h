#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include "wired/message.h"
#include "wired/tools/log.h"
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
    connection(asio::io_context& io_context, asio::ip::tcp::socket&& socket,
               ts_deque<message_t>& messages_in);
    connection(const connection& other) = delete;
    connection(connection&& other);
    virtual ~connection();

    connection& operator=(const connection&& other) = delete;
    connection& operator=(connection&& other);

    void bind();
    bool is_connected() const;
    bool send(const message_t& msg);
    void disconnect();

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    ts_deque<message_t>& messages_in_;
    message_t aux_message_;
    bool connected_;
};

template <typename T>
connection<T>::connection(asio::io_context& io_context,
                          asio::ip::tcp::socket&& socket,
                          ts_deque<message_t>& messages_in)
    : io_context_(io_context), socket_(std::move(socket)),
      messages_in_(messages_in), aux_message_(), connected_(false) {}

template <typename T>
connection<T>::~connection() {}

template <typename T>
void connection<T>::bind() {}

template <typename T>
bool connection<T>::is_connected() const {
    return socket_.is_open();
}

template <typename T>
bool connection<T>::send(const message_t& msg) {}

template <typename T>
void connection<T>::disconnect() {
    asio::post(io_context_, [this]() { socket_.close(); });
    connected_ = false;
}

} // namespace wired

#endif