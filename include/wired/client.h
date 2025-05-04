#ifndef WIRED_CLIENT_H
#define WIRED_CLIENT_H

#include <asio.hpp>

#include "wired/connection.h"
#include "wired/message.h"
#include "wired/ts_deque.h"
#include "wired/types.h"

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

    bool is_connected() const;

    // std::future<bool> ping();

    std::future<bool> connect(const std::string& host, const std::string& port);
    std::future<bool> disconnect();
    std::future<bool>
    send(const message_t& msg,
         message_strategy strategy = message_strategy::normal);

    void run(execution_policy policy = execution_policy::blocking);

  private:
    void messaging_loop();
    void contribute_to_context_pool();

  private:
    asio::io_context context_;
    asio::executor_work_guard<asio::io_context::executor_type> idle_work_;
    std::thread asio_thread_;
    connection_ptr connection_;
    ts_deque<message_t> messages_;
    std::thread messages_thread_;
    std::condition_variable cv_;
    std::mutex mutex_;
    std::atomic<bool> stop_messaging_loop_{false};
}; // class client_interface

template <typename T>
client_interface<T>::client_interface()
    : context_(), idle_work_(asio::make_work_guard(context_)), asio_thread_(),
      connection_(nullptr), messages_(), messages_thread_(), cv_(), mutex_() {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "client_interface object [{}] called default constructor",
                      (void*)this);
    asio_thread_ =
        std::thread(&client_interface<T>::contribute_to_context_pool, this);
}

template <typename T>
client_interface<T>::client_interface(client_interface&& other)
    : context_(std::move(other.context_)),
      idle_work_(std::move(other.idle_work_)),
      asio_thread_(std::move(other.asio_thread_)),
      connection_(std::move(other.connection_)),
      messages_(std::move(other.messages_)), messages_thread_(),
      cv_(std::move(other.cv_)), mutex_(std::move(other.mutex_)) {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "client_interface object [{}] called move constructor",
                      (void*)this);
}

template <typename T>
client_interface<T>::~client_interface() {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "client_interface object [{}] called destructor",
                      (void*)this);
    context_.stop();
    if (asio_thread_.joinable()) {
        asio_thread_.join();
    }

    stop_messaging_loop_ = true;
    cv_.notify_all();
    if (messages_thread_.joinable()) {
        messages_thread_.join();
    }
}

template <typename T>
client_interface<T>& client_interface<T>::operator=(client_interface&& other) {
    if (this == &other) {
        return *this;
    }

    context_ = std::move(other.context_);
    idle_work_ = std::move(other.idle_work_);
    asio_thread_ = std::move(other.asio_thread_);
    connection_ = std::move(other.connection_);
    messages_ = std::move(other.messages_);
    messages_thread_ = std::move(other.messages_thread_);
    cv_ = std::move(other.cv_);
    mutex_ = std::move(other.mutex_);

    other.connection_ = nullptr;
    other.messages_.clear();

    return *this;
}

template <typename T>
bool client_interface<T>::is_connected() const {
    if (!connection_) {
        return false;
    }
    return connection_->is_connected();
}

template <typename T>
std::future<bool> client_interface<T>::connect(const std::string& host,
                                               const std::string& port) {
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
            context_, asio::ip::tcp::socket(context_), messages_, cv_);

        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG, "connection object address: {}",
                          (void*)connection_.get());

        std::future<bool> connection_result = connection_->connect(endpoints);
        return connection_result;

    } catch (const std::exception& ec) {
        disconnect();
        throw ec;
    }
}

template <typename T>
std::future<bool> client_interface<T>::disconnect() {
    if (!is_connected()) {
        std::promise<bool> promise;
        promise.set_value(false);
        return promise.get_future();
    }
    std::future<bool> connection_result = connection_->disconnect();
    return connection_result;
}

template <typename T>
std::future<bool> client_interface<T>::send(const message_t& msg,
                                            message_strategy strategy) {
    if (!is_connected()) {
        std::promise<bool> promise;
        promise.set_value(false);
        return promise.get_future();
    }
    std::future<bool> connection_result = connection_->send(msg, strategy);
    return connection_result;
}

template <typename T>
void client_interface<T>::run(execution_policy policy) {
    stop_messaging_loop_ = false;
    if (policy == execution_policy::blocking) {
        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                          "Client message handler is running in blocking mode");
        messaging_loop();
    } else if (policy == execution_policy::non_blocking) {
        if (messages_thread_.joinable()) {
            WIRED_LOG_MESSAGE(log_level::LOG_ERROR,
                              "Messaging thread is already running");
            throw std::runtime_error("Messaging thread is already running");
        }
        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                          "Client message handler is running in non-blocking "
                          "mode");
        messages_thread_ =
            std::thread(&client_interface<T>::messaging_loop, this);
    }
}

template <typename T>
void client_interface<T>::messaging_loop() {
    while (is_connected()) {
        std::unique_lock<std::mutex> lock(mutex_);
        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                          "Waiting for messages in the queue");
        cv_.wait(lock, [this] {
            return (!messages_.empty() || stop_messaging_loop_);
        });
        if (stop_messaging_loop_) {
            WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                              "Stop messaging loop, exiting");
            return;
        }
        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                          "Messages in the queue, processing them");

        while (!messages_.empty()) {
            if (stop_messaging_loop_) {
                WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                                  "Stop messaging loop, exiting");
                return;
            }
            message_t msg = messages_.front();
            messages_.pop_front();
            WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                              "Processing message with id {}",
                              static_cast<uint32_t>(msg.head().id()));
            on_message(msg, msg.from());
        }
    }
}

template <typename T>
void client_interface<T>::contribute_to_context_pool() {
    context_.run();
}

} // namespace wired

#endif // WIRED_CLIENT_H