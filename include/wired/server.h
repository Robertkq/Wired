#ifndef WIRED_SERVER_H
#define WIRED_SERVER_H

#include <asio.hpp>
#include <asio/ssl.hpp>

#include "wired/connection.h"
#include "wired/message.h"
#include "wired/tools/log.h"
#include "wired/ts_deque.h"
#include "wired/types.h"

#include <string>

namespace wired {
template <typename T>
class server_interface {
  public:
    using message_t = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    virtual void on_message(message_t& msg, connection_ptr conn) = 0;

  public:
    server_interface();
    virtual ~server_interface();

  public:
    void start(const std::string& port);
    void shutdown();

    bool is_listening();

    void set_tls_options(const tls_options& options) { options_ = options; }

    std::future<bool>
    send(connection_ptr conn, const message_t& msg,
         message_strategy strategy = message_strategy::normal);

    std::vector<std::future<bool>>
    send_all(connection_ptr ignore, const message_t& msg,
             message_strategy strategy = message_strategy::normal);

    std::future<bool> kick(connection_ptr conn);

    void run(execution_policy policy = execution_policy::blocking);

    ts_deque<connection_ptr>& connections() { return connections_; }

  private:
    void messaging_loop();
    void contribute_to_context_pool();
    void on_message_notify_callback();
    void wait_for_client_chain();

  private:
    asio::io_context context_;
    asio::ssl::context ssl_context_;
    asio::executor_work_guard<asio::io_context::executor_type> idle_work_;
    std::thread asio_thread_;
    asio::ip::tcp::acceptor acceptor_;
    ts_deque<connection_ptr> connections_;
    ts_deque<message_t> messages_;
    std::thread messages_thread_;
    std::condition_variable cv_;
    std::mutex mutex_;
    std::atomic<bool> stop_messaging_loop_;
    tls_options options_;
}; // class server_interface

template <typename T>
server_interface<T>::server_interface()
    : context_(), ssl_context_{asio::ssl::context::tls_server},
      idle_work_(asio::make_work_guard(context_)), asio_thread_(),
      acceptor_(context_), connections_(), messages_(), messages_thread_(),
      cv_(), mutex_(), stop_messaging_loop_(false), options_() {
    asio_thread_ =
        std::thread(&server_interface<T>::contribute_to_context_pool, this);
}

template <typename T>
server_interface<T>::~server_interface() {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "server_interface object [{}] destructor called",
                      (void*)this);
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
void server_interface<T>::start(const std::string& port) {
    tls_options::set_context_options(ssl_context_, options_);
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(port));
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    wait_for_client_chain();
}

template <typename T>
void server_interface<T>::shutdown() {
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "server_interface shutdown started");

    std::vector<std::future<bool>> results;
    connections_.for_each([&results](connection_ptr conn) {
        if (conn->is_connected()) {
            results.push_back(conn->disconnect());
        }
    });
    for (auto& result : results) {
        result.wait();
    }

    stop_messaging_loop_ = true;
    cv_.notify_all();
    if (messages_thread_.joinable()) {
        messages_thread_.join();
    }

    connections_.clear();
    messages_.clear();
    acceptor_.close();

    context_.stop();
    asio_thread_.join();
    idle_work_.reset();
    WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                      "server_interface shutdown completed");
}

template <typename T>
bool server_interface<T>::is_listening() {
    return acceptor_.is_open();
}

template <typename T>
std::future<bool> server_interface<T>::send(connection_ptr conn,
                                            const message_t& msg,
                                            message_strategy strategy) {
    if (conn && conn->is_connected()) {
        return conn->send(msg, strategy);
    }
    std::promise<bool> promise;
    promise.set_value(false);
    return promise.get_future();
}

template <typename T>
std::vector<std::future<bool>>
server_interface<T>::send_all(connection_ptr ignore, const message_t& msg,
                              message_strategy strategy) {
    std::vector<std::future<bool>> results;
    connections_.for_each(
        [&results, &msg, &ignore, strategy](connection_ptr conn) {
            if (conn != ignore && conn->is_connected()) {
                results.push_back(conn->send(msg, strategy));
            }
        });
    return results;
}

template <typename T>
std::future<bool> server_interface<T>::kick(connection_ptr conn) {
    if (conn && conn->is_connected()) {
        auto ret = conn->disconnect();
        connections_.erase_remove(conn);
        return ret;
    }
    // FIXME: After disconnecting, the connection is not removed from the list
    std::promise<bool> promise;
    promise.set_value(false);
    return promise.get_future();
}

template <typename T>
void server_interface<T>::run(execution_policy policy) {
    if (policy == execution_policy::blocking) {
        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                          "Server message handler is running in blocking mode");
        messaging_loop();
    } else if (policy == execution_policy::non_blocking) {
        if (messages_thread_.joinable()) {
            WIRED_LOG_MESSAGE(log_level::LOG_ERROR,
                              "Messaging thread is already running");
            throw std::runtime_error("Messaging thread is already running");
        }
        WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                          "Server message handler is running in non-blocking "
                          "mode");
        messages_thread_ =
            std::thread(&server_interface<T>::messaging_loop, this);
    }
}

template <typename T>
void server_interface<T>::messaging_loop() {
    while (is_listening()) {
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
void server_interface<T>::contribute_to_context_pool() {
    context_.run();
}

template <typename T>
void server_interface<T>::wait_for_client_chain() {
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                connection_ptr conn = std::make_shared<connection_t>(
                    context_, ssl_context_, std::move(socket), messages_, cv_);
                WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                                  "wait_for_client_chain successfully accepted "
                                  "a connection, obj addr {}",
                                  (void*)conn.get());
                conn->ssl_stream().async_handshake(
                    asio::ssl::stream_base::server,
                    [this, conn](const asio::error_code& handshake_error) {
                        if (!handshake_error) {
                            WIRED_LOG_MESSAGE(log_level::LOG_INFO,
                                              "SSL handshake successful for "
                                              "connection obj addr {}",
                                              (void*)conn.get());
                            if (conn->is_connected()) {
                                conn->start_listening();
                                connections_.push_back(conn);
                            }
                            wait_for_client_chain();
                        } else {
                            WIRED_LOG_MESSAGE(log_level::LOG_ERROR,
                                              "SSL handshake failed for "
                                              "connection obj addr {} with "
                                              "error code: {} and error "
                                              "message: {}",
                                              (void*)conn.get(),
                                              handshake_error.value(),
                                              handshake_error.message());
                        }
                    });
            } else {
                WIRED_LOG_MESSAGE(
                    log_level::LOG_DEBUG,
                    "wait_for_client_chain didn't succeed to accept a "
                    "connection with error code: {} and error message: {}",
                    ec.value(), ec.message());
            }
        });
}

} // namespace wired

#endif // WIRED_SERVER_H