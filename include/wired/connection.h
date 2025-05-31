#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include "wired/message.h"
#include "wired/tools/log.h"
#include "wired/ts_deque.h"
#include "wired/types.h"

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

namespace wired {

template <typename T>
class connection : public std::enable_shared_from_this<connection<T>> {
  public:
    using message_t = message<T>;

  public:
    connection(asio::io_context& io_context, asio::ssl::context& ssl_context,
               asio::ip::tcp::socket&& socket,
               ts_deque<message_t>& incoming_messages,
               std::condition_variable& cv);
    connection(const connection& other) = delete;
    connection(connection&& other) noexcept;
    ~connection();

    connection& operator=(const connection&& other) = delete;
    connection& operator=(connection&& other) noexcept;

    // asio::ip::tcp::socket& socket();
    // const asio::ip::tcp::socket& socket() const;
    asio::ssl::stream<asio::ip::tcp::socket>& ssl_stream();
    const asio::ssl::stream<asio::ip::tcp::socket>& ssl_stream() const;

    bool is_connected() const;
    std::future<bool> send(const message_t& msg, message_strategy strategy);
    std::future<bool> connect(asio::ip::tcp::resolver::results_type& endpoints);

    std::future<bool> disconnect();
    std::size_t incoming_messages_count() const;
    std::size_t outgoing_messages_count() const;
    ts_deque<message_t>& incoming_messages();
    const ts_deque<message_t>& incoming_messages() const;

    void start_listening() {
        WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                          "Connection object [{}] started listening",
                          (void*)this);
        read_header();
    }

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

    bool is_disconnect_error(const asio::error_code& error) {
        return error == asio::error::eof ||
               error == asio::error::connection_reset ||
               error == asio::error::operation_aborted ||
               error == asio::error::bad_descriptor;
    }

  private:
    asio::io_context& io_context_;
    asio::ssl::context& ssl_context_;
    asio::ssl::stream<asio::ip::tcp::socket> ssl_stream_;
    ts_deque<std::pair<message_t, std::promise<bool>>> outgoing_messages_;
    ts_deque<message_t>& incoming_messages_;
    message_t aux_message_;
    std::condition_variable& cv_;
};

template <typename T>
connection<T>::connection(asio::io_context& io_context,
                          asio::ssl::context& ssl_context,
                          asio::ip::tcp::socket&& socket,
                          ts_deque<message_t>& incoming_messages,
                          std::condition_variable& cv)
    : io_context_(io_context), ssl_context_(ssl_context),
      ssl_stream_(std::move(socket), ssl_context_), outgoing_messages_(),
      incoming_messages_(incoming_messages), aux_message_(), cv_(cv) {
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Connection object [{}] called constructor", (void*)this);
}

template <typename T>
connection<T>::connection(connection&& other) noexcept
    : io_context_(std::move(other.io_context_)),
      ssl_stream_(std::move(other.ssl_stream_)),
      outgoing_messages_(std::move(other.outgoing_messages_)),
      incoming_messages_(std::move(other.incoming_messages_)),
      aux_message_(std::move(other.aux_message_)), cv_(other.cv_) {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "Connection object [{}] called move constructor",
                      (void*)this);
}

template <typename T>
connection<T>::~connection() {
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Connection object [{}] called destructor", (void*)this);
}

template <typename T>
asio::ssl::stream<asio::ip::tcp::socket>& connection<T>::ssl_stream() {
    return ssl_stream_;
}

template <typename T>
const asio::ssl::stream<asio::ip::tcp::socket>&
connection<T>::ssl_stream() const {
    return ssl_stream_;
}

template <typename T>
bool connection<T>::is_connected() const {
    return ssl_stream_.lowest_layer().is_open();
}

template <typename T>
std::future<bool> connection<T>::send(const message_t& msg,
                                      message_strategy strategy) {
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    if (!is_connected()) {
        promise.set_value(false);
        return future;
    }
    asio::post(io_context_, [this, msg = std::move(msg),
                             promise = std::move(promise), strategy]() mutable {
        bool writing = !outgoing_messages_.empty();
        std::pair<message_t, std::promise<bool>> pack{std::move(msg),
                                                      std::move(promise)};
        outgoing_messages_.add_message(strategy, std::move(pack));
        WIRED_LOG_MESSAGE(wired::LOG_DEBUG, "Message added to queue");
        if (!writing) {
            WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                              "Starting sending messages procedure");
            write_header();
        }
    });
    return future;
}

template <typename T>
std::future<bool>
connection<T>::connect(asio::ip::tcp::resolver::results_type& endpoints) {
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    if (is_connected()) {
        WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                          "Connection object [{}] already connected",
                          (void*)this);
        promise.set_value(false);
        return future;
    }
    asio::async_connect(
        ssl_stream_.lowest_layer(), endpoints,
        [this, promise = std::move(promise)](
            const asio::error_code& error,
            asio::ip::tcp::endpoint endpoint) mutable {
            if (error) {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "Error while connecting\n"
                                  "with error code: {}\n"
                                  "and error message: {}",
                                  error.value(), error.message());
                promise.set_value(false);
                return;
            }
            WIRED_LOG_MESSAGE(
                wired::LOG_INFO,
                "Connection object [{}] Connected to: {}, trying to "
                "perform SSL handshake",
                (void*)this, endpoint.address().to_string());
            ssl_stream_.async_handshake(
                asio::ssl::stream_base::client,
                [this, promise = std::move(promise)](
                    const asio::error_code& error) mutable {
                    if (error) {
                        WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                          "SSL handshake failed\n"
                                          "with error code: {}\n"
                                          "and error message: {}",
                                          error.value(), error.message());
                        promise.set_value(false);
                        return;
                    }
                    WIRED_LOG_MESSAGE(wired::LOG_INFO,
                                      "SSL handshake successful");
                    read_header();
                    promise.set_value(true);
                });
        });
    return future;
}

template <typename T>
std::future<bool> connection<T>::disconnect() {
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    if (!is_connected()) {
        promise.set_value(false);
        return future;
    }

    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Connection object [{}] called disconnect", (void*)this);

    asio::post(io_context_, [this, promise = std::move(promise)]() mutable {
        asio::error_code error;

        // ssl_stream_.shutdown(error);
        // if (error) {
        //     if (is_disconnect_error(error)) {
        //         WIRED_LOG_MESSAGE(
        //             wired::LOG_INFO,
        //             "{} Remote disconnected gracefully from SSL shutdown "
        //             "with error code: {} "
        //             "and error message: {}",
        //             (void*)this, error.value(), error.message());
        //     } else {
        //         WIRED_LOG_MESSAGE(wired::LOG_ERROR,
        //                           "{} SSL shutdown error "
        //                           "with error code: {} "
        //                           "and error message: {}",
        //                           (void*)this, error.value(),
        //                           error.message());
        //         promise.set_exception(
        //             std::make_exception_ptr(std::runtime_error(
        //                 "SSL shutdown error: " +
        //                 std::to_string(error.value()) + " - " +
        //                 error.message())));
        //         return;
        //     }
        // }

        ssl_stream_.lowest_layer().shutdown(
            asio::ip::tcp::socket::shutdown_both, error);
        if (error) {
            if (is_disconnect_error(error)) {
                WIRED_LOG_MESSAGE(
                    wired::LOG_INFO,
                    "{} Remote disconnected gracefully from TCP shutdown "
                    "with error code: {} "
                    "and error message: {}",
                    (void*)this, error.value(), error.message());
            } else {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "{} TCP shutdown error "
                                  "with error code: {} "
                                  "and error message: {}",
                                  (void*)this, error.value(), error.message());
                promise.set_exception(
                    std::make_exception_ptr(std::runtime_error(
                        "TCP shutdown error: " + std::to_string(error.value()) +
                        " - " + error.message())));
                return;
            }
        }

        ssl_stream_.lowest_layer().close(error);
        if (error) {
            if (is_disconnect_error(error)) {
                WIRED_LOG_MESSAGE(
                    wired::LOG_INFO,
                    "{} Remote disconnected gracefully from close "
                    "with error code: {} "
                    "and error message: {}",
                    (void*)this, error.value(), error.message());
            } else {
                WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                                  "{} Socket close error "
                                  "with error code: {} "
                                  "and error message: {}",
                                  (void*)this, error.value(), error.message());
                promise.set_exception(
                    std::make_exception_ptr(std::runtime_error(
                        "Socket close error: " + std::to_string(error.value()) +
                        " - " + error.message())));
                return;
            }
        }

        outgoing_messages_.clear();
        incoming_messages_.clear();

        promise.set_value(true);
    });

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
        ssl_stream_,
        asio::buffer(&aux_message_.head(), sizeof(message_header<T>)),
        std::bind(&connection<T>::read_header_handler, this,
                  std::placeholders::_1, std::placeholders::_2));
}

template <typename T>
void connection<T>::read_header_handler(const asio::error_code& error,
                                        std::size_t bytes_transferred) {
    if (error) {
        if (is_disconnect_error(error)) {
            WIRED_LOG_MESSAGE(
                wired::LOG_INFO,
                "{} Remote disconnected gracefully from read_header "
                "with error code: "
                "{} and error message: {}",
                (void*)this, error.value(), error.message());
        } else {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "{} Error while reading header "
                              "with error code: {} "
                              "and error message: {}",
                              (void*)this, error.value(), error.message());
        }
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
    asio::async_read(ssl_stream_,
                     asio::buffer(aux_message_.body().data().data(),
                                  aux_message_.body().data().size()),
                     std::bind(&connection<T>::read_body_handler, this,
                               std::placeholders::_1, std::placeholders::_2));
}

template <typename T>
void connection<T>::read_body_handler(const asio::error_code& error,
                                      std::size_t bytes_transferred) {
    if (error) {
        if (is_disconnect_error(error)) {
            WIRED_LOG_MESSAGE(
                wired::LOG_INFO,
                "{} Remote disconnected gracefully from read_body "
                "with error code: "
                "{} and error message: {}",
                (void*)this, error.value(), error.message());
        } else {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "{} Error while reading body "
                              "with error code: {} "
                              "and error message: {}",
                              (void*)this, error.value(), error.message());
        }
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
    asio::async_write(ssl_stream_,
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
        if (is_disconnect_error(error)) {
            WIRED_LOG_MESSAGE(
                wired::LOG_INFO,
                "{} Remote disconnected gracefully from write_header "
                "with error code: "
                "{} and error message: {}",
                (void*)this, error.value(), error.message());
        } else {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "{} Error while writing header "
                              "with error code: {} "
                              "and error message: {}",
                              (void*)this, error.value(), error.message());
        }
        disconnect();
        promise.set_exception(std::make_exception_ptr(std::runtime_error(
            "Error while writing header: " + std::to_string(error.value()) +
            " - " + error.message())));
        return;
    }
    WIRED_LOG_MESSAGE(
        wired::LOG_DEBUG,
        "Wrote {} bytes out of expected {} of header successfully",
        bytes_transferred, sizeof(message_header<T>));

    if (msg.head().size() > 0) {
        write_body();
    } else {
        promise.set_value(true);
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
    asio::async_write(ssl_stream_,
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
        if (is_disconnect_error(error)) {
            WIRED_LOG_MESSAGE(
                wired::LOG_INFO,
                "{} Remote disconnected gracefully from write_body "
                "with error code: {} "
                "and error message: {}",
                (void*)this, error.value(), error.message());
        } else {
            WIRED_LOG_MESSAGE(wired::LOG_ERROR,
                              "{} Error while writing body "
                              "with error code: {} "
                              "and error message: {}",
                              (void*)this, error.value(), error.message());
        }
        disconnect();
        promise.set_exception(std::make_exception_ptr(std::runtime_error(
            "Error while writing body: " + std::to_string(error.value()) +
            " - " + error.message())));
        return;
    }
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Wrote {} bytes out of expected {} of body successfully",
                      bytes_transferred, msg.head().size());
    promise.set_value(true);
    outgoing_messages_.pop_front();
    if (outgoing_messages_.size() > 0) {
        write_header();
    }
}

template <typename T>
void connection<T>::append_finished_message() {
    WIRED_LOG_MESSAGE(wired::LOG_DEBUG,
                      "Appended message with header id: {}, header size: {} "
                      "and body size: {}",
                      static_cast<unsigned int>(aux_message_.id()),
                      aux_message_.head().size(),
                      aux_message_.body().data().size());
    aux_message_.from() = this->shared_from_this();
    incoming_messages_.emplace_back(std::move(aux_message_));
    aux_message_.reset();
    cv_.notify_all();
    read_header();
}

} // namespace wired

#endif