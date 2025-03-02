#ifndef WIRED_CLIENT_H
#define WIRED_CLIENT_H

#include <wired/connection.h>
#include <wired/message.h>
#include <wired/ts_deque.h>
#include <wired/types.h>

#include <asio.hpp>
#include <deque>
#include <iostream>
#include <memory>

namespace wired {
template <typename T>
class client_interface {
  public:
    using message_t = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    virtual void on_message(message_t& msg, connection_ptr conn) = 0;

  public:
    client_interface();
    client_interface(const client_interface& other) = delete;
    client_interface(client_interface&& other);
    virtual ~client_interface();

    client_interface& operator=(const client_interface&& other) = delete;
    client_interface& operator=(client_interface&& other);

    std::future<bool> connect(const std::string& host, const std::string& port);
    std::future<bool> disconnect();
    bool is_connected() const;
    std::future<bool> send(const message_t& msg);

    bool update();
    void run();

    bool update();

  private:
<<<<<<< HEAD
=======
    connection_ptr connection_;
    ts_deque<message_t> messages_;
>>>>>>> 83019f5 (Proper separation of g++/msvc compiler definitions)
    asio::io_context context_;
    asio::executor_work_guard<asio::io_context::executor_type> idle_work_;
    std::thread thread_;
    connection_ptr connection_;
    ts_deque<message_t> messages_;
}; // class client_interface

template <typename T>
client_interface<T>::client_interface()
<<<<<<< HEAD
    : context_(), idle_work_(asio::make_work_guard(context_)), thread_(),
      connection_(nullptr), messages_() {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "client_interface default constructor");
    thread_ = std::thread(&client_interface<T>::run, this);
}

template <typename T>
client_interface<T>::client_interface(client_interface&& other)
    : context_(std::move(other.context_)),
      idle_work_(std::move(other.idle_work_)),
      thread_(std::move(other.thread_)),
      connection_(std::move(other.connection_)),
      messages_(std::move(other.messages_)) {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "client_interface move constructor");
}
=======
    : connection_(nullptr), messages_(), context_() {}

template <typename T>
client_interface<T>::client_interface(client_interface&& other)
    : connection_(std::move(other.connection_)),
      messages_(std::move(other.messages_)),
      context_(std::move(other.context_)) {}
>>>>>>> 83019f5 (Proper separation of g++/msvc compiler definitions)

template <typename T>
client_interface<T>::~client_interface() {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG, "client_interface destructor");
    if (is_connected()) {
        disconnect();
    }
    context_.stop();
    thread_.join();
}

template <typename T>
client_interface<T>& client_interface<T>::operator=(client_interface&& other) {
    if (this == &other) {
        return *this;
    }

    context_ = std::move(other.context_);
    idle_work_ = std::move(other.idle_work_);
    thread_ = std::move(other.thread_);
    connection_ = std::move(other.connection_);
    messages_ = std::move(other.messages_);
<<<<<<< HEAD
=======
    context_ = std::move(other.context_);
>>>>>>> 83019f5 (Proper separation of g++/msvc compiler definitions)

    return *this;
}

template <typename T>
std::future<bool> client_interface<T>::connect(const std::string& host,
                                               const std::string& port) {
<<<<<<< HEAD

=======
>>>>>>> 83019f5 (Proper separation of g++/msvc compiler definitions)
    if (is_connected()) {
        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                          "Client tried to connect while already connected");
        disconnect();
    }
    try {
        asio::ip::tcp::resolver resolver(context_);

        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(host, port);

        connection_ = std::make_shared<connection_t>(
            context_, asio::ip::tcp::socket(context_), messages_);

        std::future<bool> connection_result = connection_->connect(endpoints);
        return connection_result;

    } catch (std::exception ec) {
        disconnect();
        throw ec;
    }
}

template <typename T>
std::future<bool> client_interface<T>::disconnect() {
<<<<<<< HEAD
    if (!is_connected()) {
        std::promise<bool> promise;
        promise.set_value(false);
        return promise.get_future();
    }
    std::future<bool> connection_result = connection_->disconnect();
    connection_.reset();
    messages_.clear();
=======
    std::future<bool> connection_result = connection_->disconnect();
    connection_.reset();
>>>>>>> 83019f5 (Proper separation of g++/msvc compiler definitions)
    return connection_result;
}

template <typename T>
bool client_interface<T>::is_connected() const {
    if (!connection_) {
        return false;
    }
    return connection_->is_connected();
}

template <typename T>
std::future<bool> client_interface<T>::send(const message_t& msg) {
    if (!is_connected()) {
        std::promise<bool> promise;
        promise.set_value(false);
        return promise.get_future();
    }
    std::future<bool> connection_result = connection_->send(msg);
    return connection_result;
}

template <typename T>
bool client_interface<T>::update() {
    if (messages_.empty()) {
        return false;
    }
    message_t msg = messages_.front();
    messages_.pop_front();
    on_message(msg, msg.from());
    return true;
}

template <typename T>
void client_interface<T>::run() {
    context_.run();
}

template <typename T>
bool client_interface<T>::update() {
    if (messages_.empty()) {
        return false;
    }
    message_t msg = messages_.front();
    messages_.pop_front();
    on_message(msg, msg.from());
    return true;
}

} // namespace wired

#endif // WIRED_CLIENT_H