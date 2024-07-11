#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include "wired/message.h"
#include "wired/tools/log.h"
#include "wired/ts_deque.h"
#include "wired/types.h"

#include <asio.hpp>
#include <functional>
#include <iostream>
#include <limits>

namespace wired {

template <typename T>
class connection {
  public:
    using message_t = message<T>;

  public:
    connection(asio::io_context& io_context, asio::ip::tcp::socket&& socket);
    template <typename Callable>
    connection(asio::io_context& io_context, asio::ip::tcp::socket&& socket)
        requires message_handler<Callable, T>;
    connection(const connection& other) = delete;
    connection(connection&& other);
    virtual ~connection();

    connection& operator=(const connection&& other) = delete;
    connection& operator=(connection&& other);

    bool is_connected() const;
    bool send(const message_t& msg);
    void disconnect();
    void process_incoming_messages(
        std::size_t n = std::numeric_limits<std::size_t>::max());

    void set_message_handler();
    std::size_t incoming_messages_count() const;
    std::size_t outgoing_messages_count() const;

  private:
    void read_header();
    void read_body();
    void write_header();
    void write_body();

    void append_finished_message(message_t& msg);

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    ts_deque<message_t> outgoing_messages_;
    ts_deque<message_t> ingoing_messages_;
    message_t aux_message_;
    bool connected_;
    std::function<void(message_t&)> message_handler_callback;
};

template <typename T>
connection<T>::connection(asio::io_context& io_context,
                          asio::ip::tcp::socket&& socket)
    : io_context_(io_context), socket_(std::move(socket)), outgoing_messages_(),
      ingoing_messages_(), aux_message_(), connected_(false),
      message_handler_callback(nullptr) {
    if (!is_connected()) {
        return;
    }
    read_header();
}

template <typename T>
template <typename Callable>
connection<T>::connection(asio::io_context& io_context,
                          asio::ip::tcp::socket&& socket, Callable callback)
    requires message_handler<Callable, T>
    : io_context_(io_context), socket_(std::move(socket)), outgoing_messages_(),
      ingoing_messages_(), aux_message_(), connected_(false),
      message_handler_callback(callback) {}

template <typename T>
connection<T>::~connection() {}

template <typename T>
bool connection<T>::is_connected() const {
    return socket_.is_open();
}

template <typename T>
bool connection<T>::send(const message_t& msg) {
    asio::post(io_context_, [this, msg]() {
        bool writing = !this->outgoing_messages_.empty();
        this->outgoing_messages_.push_back(msg);
        if (!writing) {
            this->write_head();
        }
    });
}

template <typename T>
void connection<T>::disconnect() {
    if (!socket_.is_open()) {
        return;
    }
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG, "Socket disconnecting...");
    asio::post(io_context_, [this]() {
        asio::error_code error;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, error);
        if (error) {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "Socket error while shutting down send/write\n"
                              "with error code: {}\n"
                              "and error message: {}",
                              error.value(), error.message());
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

template <typename T>
void connection<T>::process_incoming_messages(std::size_t count) {
    while (count > 0) {
        message_t& msg = incoming_messages_.pop_front();
        message_handler_callback(msg);
        --count;
    }
}

template <typename T>
template <typename Callable>
void connection<T>::set_message_handler(Callable handle)
    requires message_handler<Callable>
{
    message_handler_callback = handle;
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
void connection<T>::read_header() {
    socket_.async_read_some(
        asio::buffer(&(aux_message_.head()), sizeof(message_header<T>)),
        [this](const asio::error_code& error, std::size_t bytes_transferred) {
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
                append_finished_message(aux_message_);
            }
        });
}

template <typename T>
void connection<T>::read_body() {
    socket_.async_read_some(
        asio::buffer(&(aux_message_.body().data().data()),
                     aux_message_.body().data().size()),
        [this](const asio::error_code& error, std::size_t bytes_transferred) {
            if (error) {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "Error while reading body\n"
                                  "with error code: {}\n"
                                  "and error message: {}",
                                  error.value(), error.message());
                disconnect();
                return;
            }
            WIRED_LOG_MESSAGE(
                wired::LOG_DEBUG, "Read {} bytes out of expected {} of body",
                bytes_transferred, aux_message_.body().data().size());

            append_finished_message(aux_message_);
        });
}

template <typename T>
void connection<T>::write_header() {
    socket_.async_write_some(
        asio::buffer(&(outgoing_messages_.front().head()),
                     sizeof(message_header<T>)),
        [this](const asio::error_code& error, std::size_t bytes_transferred) {
            if (error) {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "Error while writing header\n"
                                  "with error code: {}\n"
                                  "and error message: {}",
                                  error.value(), error.message());
                disconnect();
                return;
            }
            message& msg = outgoing_messages_.front();
            WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                              "Wrote {} bytes out of expected {} of header",
                              bytes_transferred, sizeof(message_header<T>));
            if (msg.head().size() > 0) {
                write_body();
            } else if (outgoing_messages_.size() > 0) {
                outgoing_messages_.pop_front();
                write_header();
            }
        });
}

template <typename T>
void connection<T>::write_body() {
    socket_.async_write_some(
        asio::buffer(&(outgoing_messages_.front().body().data().data(),
                       outgoing_messages_.front().head().size())),
        [this](const asio::error_code& error, std::size_t bytes_transferred) {
            if (error) {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "Error while writing body\n"
                                  "with error code: {}\n"
                                  "and error message: {}",
                                  error.value(), error.message());
                disconnect();
                return;
            }
            message msg = std::move(outgoing_messages_.pop_front());
            WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                              "Wrote {} bytes out of expected {} of body",
                              bytes_transferred, msg.head().size());
            if (outgoing_messages_.size() > 0) {
                write_header();
            }
        });
}

template <typename T>
void connection<T>::append_finished_message(message_t& msg) {
    outgoing_messages_.emplace_back(std::move(msg));
    msg.reset();
    read_header();
}

} // namespace wired

#endif