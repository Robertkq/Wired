#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include "wired/message.h"
#include "wired/tools/log.h"
#include "wired/ts_deque.h"
#include "wired/types.h"

#include <asio.hpp>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <utility>

namespace wired {

inline uint64_t connection_incrementor{1};

template <typename T>
class connection {
  public:
    using message_t = message<T>;

  public:
    connection(asio::io_context& io_context, asio::ip::tcp::socket&& socket);
    connection(const connection& other) = delete;
    connection(connection&& other);
    ~connection();

    connection& operator=(const connection&& other) = delete;
    connection& operator=(connection&& other);

    bool is_connected() const;
    std::future<void> send(const message_t& msg);
    std::future<void> disconnect();
    std::size_t incoming_messages_count() const;
    std::size_t outgoing_messages_count() const;
    ts_deque<message_t>& incoming_messages();
    const ts_deque<message_t>& incoming_messages() const;

  private:
    void read_header();
    void read_header_handler(const asio::error_code& error,
                             std::size_t bytes_transferred);
    void read_body();
    void read_body_handler(const asio::error_code& error,
                           std::size_t bytes_transferred);
    void write_header();
    void write_header_handler(const asio::error_code& error,
                              std::size_t bytes_transferred);
    void write_body();
    void write_body_handler(const asio::error_code& error,
                            std::size_t bytes_transferred);

    void append_finished_message();

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    ts_deque<std::pair<message_t, std::promise<void>>> outgoing_messages_;
    ts_deque<message_t> incoming_messages_;
    message_t aux_message_;
    uint64_t id_;
};

template <typename T>
connection<T>::connection(asio::io_context& io_context,
                          asio::ip::tcp::socket&& socket)
    : io_context_(io_context), socket_(std::move(socket)), outgoing_messages_(),
      incoming_messages_(), aux_message_(), id_(connection_incrementor++) {
    if (!is_connected()) {
        return;
    }
    read_header();
}

template <typename T>
connection<T>::connection(connection&& other)
    : io_context_(std::move(other.io_context_)),
      socket_(std::move(other.socket_)),
      outgoing_messages_(std::move(other.outgoing_messages_)),
      incoming_messages_(std::move(other.incoming_messages_)),
      aux_message_(std::move(other.aux_message_)),
      id_(connection_incrementor++) {}

template <typename T>
connection<T>::~connection() {
    if (is_connected()) {
        disconnect();
    }
}

template <typename T>
bool connection<T>::is_connected() const {
    return socket_.is_open();
}

template <typename T>
std::future<void> connection<T>::send(const message_t& msg) {
    std::promise<void> promise;
    std::future<void> future = promise.get_future();
    if (!is_connected()) {
        promise.set_value();
        return future;
    }
    asio::post(io_context_, [this, msg = std::move(msg),
                             promise = std::move(promise)]() mutable {
        bool writing = !this->outgoing_messages_.empty();
        this->outgoing_messages_.emplace_back(
            std::pair{std::move(msg), std::move(promise)});
        if (!writing) {
            this->write_header();
        }
    });
    return future;
}

template <typename T>
std::future<void> connection<T>::disconnect() {
    std::promise<void> promise;
    std::future<void> future = promise.get_future();
    if (!socket_.is_open()) {
        promise.set_value();
        return future;
    }
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG, "Socket disconnecting...");
    asio::post(io_context_, [this, promise = std::move(promise)]() mutable {
        asio::error_code error;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, error);
        if (error) {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "Socket error while shutting down send/write\n"
                              "with error code: {}\n"
                              "and error message: {}",
                              error.value(), error.message());
            promise.set_value();
            return;
        }
        error.clear();
        socket_.close(error);
        if (error) {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "Socket error while closing\n"
                              "with error code: {}\n"
                              "and error message: {}",
                              error.value(), error.message());
            promise.set_value();
        }
        WIRED_LOG_MESSAGE(
            wired::LOG_INFO,
            "disconnect function token completed! is_open: {} {}",
            reinterpret_cast<uintptr_t>(static_cast<const void*>(this)),
            is_connected());
        promise.set_value();
    });
    WIRED_LOG_MESSAGE(wired::LOG_INFO, "disconnect function completed!");
    return future;
}

template <typename T>
std::size_t connection<T>::incoming_messages_count() const {
    return incoming_messages_.size();
}

template <typename T>
std::size_t connection<T>::outgoing_messages_count() const {
    return outgoing_messages_.size();
}

template <typename T>
ts_deque<message<T>>& connection<T>::incoming_messages() {
    return incoming_messages_;
}

template <typename T>
const ts_deque<message<T>>& connection<T>::incoming_messages() const {
    return incoming_messages_;
}

template <typename T>
void connection<T>::read_header() {
    asio::async_read(
        socket_, asio::buffer(&aux_message_.head(), sizeof(message_header<T>)),
        std::bind(&connection<T>::read_header_handler, this,
                  std::placeholders::_1, std::placeholders::_2));
}

template <typename T>
void connection<T>::read_header_handler(const asio::error_code& error,
                                        std::size_t bytes_transferred) {
    if (error) {
        WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                          "Error while reading header\n"
                          "with error code: {}\n"
                          "and error message: {}",
                          error.value(), error.message());
        disconnect();
        return;
    }
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Read {} bytes out of expected {} of header",
                      bytes_transferred, sizeof(message_header<T>));

    if (aux_message_.head().size() > 0) {
        aux_message_.body().data().resize(aux_message_.head().size());
        read_body();
    } else {
        append_finished_message();
    }
}

template <typename T>
void connection<T>::read_body() {
    asio::async_read(socket_,
                     asio::buffer(aux_message_.body().data().data(),
                                  aux_message_.body().data().size()),
                     std::bind(&connection<T>::read_body_handler, this,
                               std::placeholders::_1, std::placeholders::_2));
}

template <typename T>
void connection<T>::read_body_handler(const asio::error_code& error,
                                      std::size_t bytes_transferred) {
    if (error) {
        WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                          "Error while reading body\n"
                          "with error code: {}\n"
                          "and error message: {}",
                          error.value(), error.message());
        disconnect();
        return;
    }
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Read {} bytes out of expected {} of body",
                      bytes_transferred, aux_message_.body().data().size());

    append_finished_message();
}

template <typename T>
void connection<T>::write_header() {
    auto& pair = outgoing_messages_.front();
    auto& msg = pair.first;
    asio::async_write(socket_,
                      asio::buffer(&msg.head(), sizeof(message_header<T>)),
                      std::bind(&connection<T>::write_header_handler, this,
                                std::placeholders::_1, std::placeholders::_2));
}

template <typename T>
void connection<T>::write_header_handler(const asio::error_code& error,
                                         std::size_t bytes_transferred) {
    auto& pair = outgoing_messages_.front();
    auto& msg = pair.first;
    auto& promise = pair.second;
    if (error) {
        WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                          "Error while writing header\n"
                          "with error code: {}\n"
                          "and error message: {}",
                          error.value(), error.message());
        disconnect();
        promise.set_value();
        // TODO: set exception here
        return;
    }
    WIRED_LOG_MESSAGE(
        wired::LOG_DEBUG,
        "Wrote {} bytes out of expected {} of header successfully",
        bytes_transferred, sizeof(message_header<T>));

    if (msg.head().size() > 0) {
        write_body();
    } else {
        promise.set_value();
        outgoing_messages_.pop_front();
        if (outgoing_messages_.size() > 0) {
            write_header();
        }
    }
}

template <typename T>
void connection<T>::write_body() {
    auto& pair = outgoing_messages_.front();
    auto& msg = pair.first;
    socket_.async_write_some(
        asio::buffer(msg.body().data().data(), msg.head().size()),
        std::bind(&connection<T>::write_body_handler, this,
                  std::placeholders::_1, std::placeholders::_2));
}

template <typename T>
void connection<T>::write_body_handler(const asio::error_code& error,
                                       std::size_t bytes_transferred) {

    auto& pair = outgoing_messages_.front();
    auto& msg = pair.first;
    auto& promise = pair.second;
    if (error) {
        WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                          "Error while writing body\n"
                          "with error code: {}\n"
                          "and error message: {}",
                          error.value(), error.message());
        disconnect();
        promise.set_value();
        // TODO: set exception here
        return;
    }
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Wrote {} bytes out of expected {} of body successfully",
                      bytes_transferred, msg.head().size());
    promise.set_value();
    outgoing_messages_.pop_front();
    if (outgoing_messages_.size() > 0) {
        write_header();
    }
}

template <typename T>
void connection<T>::append_finished_message() {
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Appended message with header size: {} and body size: {}",
                      aux_message_.head().size(),
                      aux_message_.body().data().size());
    incoming_messages_.emplace_back(std::move(aux_message_));
    aux_message_.reset();
    read_header();
}

} // namespace wired

#endif