#ifndef WIRED_SERVER_H
#define WIRED_SERVER_H

#include "wired/message.h"
#include "wired/types.h"
#include <future>
#include <string>

namespace wired {
template <typename T>
class server_interface {
  public:
    using message = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    server_interface(uint16_t port);

  public:
    void start();
    void stop();

    std::pair<bool, std::future<void>> send(connection_ptr connection,
                                            const message& msg);
    std::pair<bool, std::future<void>> send_all(connection_ptr ignore,
                                                const message& msg);
    std::pair<bool, std::future<void>> update(uint64_t count);

  private:
    void wait_for_client_chain();
    bool validate_client(connection_ptr connection);
    bool remove_client(connection_ptr connection);

  private:
    asio::io_context context_;
    asio::ip::tcp::acceptor acceptor_;
    ts_deque<connection_ptr> connections_;
    ts_deque<message> messages_;
}; // class server_interface

template <typename T>
server_interface<T>::server_interface(uint16_t port)
    : context_(), acceptor_(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      connections_(), messages_() {}

template <typename T>
void server_interface<T>::start() {
    WIRED_LOG_MESSAGE(wired::LOG_INFO, "Server started on: {}",
                      acceptor_.local_endpoint().address().to_string());
    wait_for_client_chain();
}

template <typename T>
void server_interface<T>::stop() {
    WIRED_LOG_MESSAGE(wired::LOG_INFO, "Server closing...");
    acceptor_.cancel();
    acceptor_.close();
    for (auto& connection : connections_) {
        if (connection) {
            conenction->disconnect();
        }
    }
}

template <typename T>
std::pair<bool, std::future<void>>
server_interface<T>::send(connection_ptr connection, const message& msg) {
    if (connection && connection->is_connected()) {
        return {true, connection->send()};
    } else {
        return {false, std::future<void>{}};
    }
}

template <typename T>
std::pair<bool, std::future<void>>
server_interface<T>::send_all(connection_ptr ignore, const message& msg) {
    for (auto& connection : connections_) {
        // TODO
    }
}

template <typename T>
std::pair<bool, std::future<void>> server_interface<T>::update(uint64_t count) {
    while (count > 0) {
        // TODO
        --count;
    }
}

template <typename T>
void server_interface<T>::wait_for_client_chain() {
    acceptor_.async_accept();
}

template <typename T>
void server_interface<T>::accept_handler(const asio::error_code& error,
                                         asio::ip::tcp::socket peer) {
    // something
}

template <typename T>
bool server_interface<T>::validate_client(connection_ptr connection) {
    if (connection) {
        return true;
    }
    if (connection->is_connected()) {
        return true;
    }
    remove_client(connection);
}

template <typename T>
bool server_interface<T>::remove_client(connection_ptr connection) {
    std::erase_if(connections_.begin(), connections_.end(), connection);
}

} // namespace wired

#endif // WIRED_SERVER_H