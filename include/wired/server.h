#ifndef WIRED_SERVER_H
#define WIRED_SERVER_H

#include <asio.hpp>

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
    bool start(const std::string& port);
    bool stop();

    bool is_listening();

    bool send(connection_ptr conn, const message_t& msg,
              message_strategy strategy = message_strategy::normal);

    bool send_all(connection_ptr ignore, const message_t& msg,
                  message_strategy strategy = message_strategy::normal);
    bool update();

  private:
    void run();
    void wait_for_client_chain();

  private:
    asio::io_context context_;
    asio::executor_work_guard<asio::io_context::executor_type> idle_work_;
    std::thread thread_;
    asio::ip::tcp::acceptor acceptor_;
    ts_deque<connection_ptr> connections_;
    ts_deque<message_t> messages_;
}; // class server_interface

template <typename T>
server_interface<T>::server_interface()
    : context_(), idle_work_(asio::make_work_guard(context_)), thread_(),
      acceptor_(context_), connections_(), messages_() {
    thread_ = std::thread(&server_interface<T>::run, this);
}

template <typename T>
server_interface<T>::~server_interface() {
    stop();
}

template <typename T>
bool server_interface<T>::start(const std::string& port) {
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::stoi(port));
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    wait_for_client_chain();
    return true;
}

template <typename T>
bool server_interface<T>::stop() {
    context_.stop();
    thread_.join();
    return true;
}

template <typename T>
bool server_interface<T>::is_listening() {
    return acceptor_.is_open();
}

template <typename T>
bool server_interface<T>::send(connection_ptr conn, const message_t& msg,
                               message_strategy strategy) {
    if (conn && conn->is_connected()) {
        conn->send(msg);
        return true;
    }
    return false;
}

template <typename T>
bool server_interface<T>::send_all(connection_ptr ignore, const message_t& msg,
                                   message_strategy strategy) {
    connections_.for_each([&msg, &ignore, strategy](connection_ptr conn) {
        if (conn != ignore && conn->is_connected()) {
            conn->send(msg);
        }
    });
    return true;
}

template <typename T>
bool server_interface<T>::update() {
    if (messages_.empty()) {
        return false;
    }
    message_t msg = messages_.front();
    messages_.pop_front();
    on_message(msg, msg.from());
    return true;
}

template <typename T>
void server_interface<T>::run() {
    context_.run();
}

template <typename T>
void server_interface<T>::wait_for_client_chain() {
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                WIRED_LOG_MESSAGE(
                    log_level::LOG_DEBUG,
                    "wait_for_client_chain successfully accepted a connection");
                connection_ptr conn = std::make_shared<connection_t>(
                    context_, std::move(socket), messages_);
                if (conn->is_connected()) {
                    connections_.push_back(conn);
                }
            } else {

                WIRED_LOG_MESSAGE(log_level::LOG_DEBUG,
                                  "wait_for_client_chain didn't succed "
                                  "accepted a connection");
            }
            wait_for_client_chain();
        });
}

} // namespace wired

#endif // WIRED_SERVER_H