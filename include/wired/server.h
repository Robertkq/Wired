#ifndef WIRED_SERVER_H
#define WIRED_SERVER_H

#include "wired/message.h"
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

  public:
    bool start(const std::string& port);
    bool stop();

    bool is_listening();

    bool send(connection_ptr conn, const message_t& msg,
              message_strategy strategy = message_strategy::normal);

    bool send_all(connection_ptr ignore);
    bool update();

  private:
    void wait_for_client_chain();

  private:
    asio::io_context context_;
    asio::ip::tcp::acceptor acceptor_;
    ts_deque<connection_ptr> connections_;
    ts_deque<message_t> messages_;
}; // class server_interface

template <typename T>
server_interface<T>::server_interface()
    : context_(), acceptor_(context_), connections_(), messages_() {}

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
    return true;
}

template <typename T>
bool server_interface<T>::is_listening() {
    return acceptor_.is_open();
}

template <typename T>
bool server_interface<T>::send(connection_ptr conn, const message_t& msg,
                               message_strategy strategy) {
    if (conn->is_connected()) {
        conn->send(msg, strategy);
        return true;
    }
    return false;
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
void server_interface<T>::wait_for_client_chain() {
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                connection_ptr conn = std::make_shared<connection_t>(
                    context_, std::move(socket), messages_);
                if (conn->is_connected()) {
                    connections_.push_back(conn);
                }
            }
            wait_for_client_chain();
        });
}

} // namespace wired

#endif // WIRED_SERVER_H