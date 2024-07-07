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
    if (!socket_.is_open()) {
        return;
    }
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG, "Socket disconnected");
    asio::post(io_context_, [this]() {
        asio::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        if (ec) {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "Socket error while shutting down send/write\n"
                              "with error code: {}\n"
                              "and error message: {}",
                              ec.value(), ec.message());
            return;
        }
        ec.clear();
        socket_.close(ec);
        if (ec) {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "Socket error while closing\n"
                              "with error code: {}\n"
                              "and error message: {}",
                              ec.value(), ec.message());
        }
        WIRED_LOG_MESSAGE(
            wired::LOG_INFO,
            "disconnect function token completed! is_open: {} {}",
            reinterpret_cast<uintptr_t>(static_cast<const void*>(this)),
            is_connected());
    });
    connected_ = false;
    WIRED_LOG_MESSAGE(wired::LOG_INFO, "disconnect function completed!");
}

} // namespace wired

#endif