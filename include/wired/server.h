#ifndef WIRED_SERVER_H
#define WIRED_SERVER_H

#include "wired/message.h"
#include "wired/types.h"
#include <future>
#include <string>
#include <vector>

namespace wired {
template <typename T>
class server_interface {
  public:
    using message = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;
    using message_handler_t = (void)(*)(connection_ptr, const message&);

  public:
    server_interface(uint16_t port);
    virtual ~server_interface();

  public:
    void start();
    void stop();

    bool is_open() const;

    void set_message_handler();

    std::future<void> send(connection_ptr connection, const message& msg);
    std::vector<std::future<void>> send_all(connection_ptr ignore,
                                            const message& msg);
    std::future<void> update(uint64_t count);

  private:
    void wait_for_client_chain();
    void accept_handler(const asio::error_code& error,
                        asio::ip::tcp::socket peer);
    bool validate_client(connection_ptr connection);
    bool remove_client(connection_ptr bad_connection);

  private:
    asio::io_context context_;
    asio::ip::tcp::acceptor acceptor_;
    ts_deque<connection_ptr> connections_;
    ts_deque<message> messages_;
}; // class server_interface

template <typename T>
server_interface<T>::server_interface(uint16_t port)
    : context_(),
      acceptor_(context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      connections_(), messages_() {}

template <typename T>
server_interface<T>::~server_interface() {}

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
            connection->disconnect();
        }
    }
}

template <typename T>
bool server_interface<T>::is_open() const {
    return acceptor_.is_open();
}

template <typename T>
std::future<void> server_interface<T>::send(connection_ptr connection,
                                            const message& msg) {
    if (connection && connection->is_connected()) {
        return connection->send();
    } else {
        return std::future<void>{};
    }
}

template <typename T>
std::vector<std::future<void>>
server_interface<T>::send_all(connection_ptr ignore, const message& msg) {
    std::vector<std::future<void>> futures;
    for (auto& connection : connections_) {
        if (connection != ignore) {
            futures.push_back(connection->send(msg));
        }
    }
    return futures;
}

template <typename T>
std::future<void> server_interface<T>::update(uint64_t count) {
    while (count > 0 && !messages_.empty()) {
        auto& msg = messages_.front();
        messages_.pop_front();
        handle_message(msg);
        --count;
    }
}

template <typename T>
void server_interface<T>::wait_for_client_chain() {
    acceptor_.async_accept(std::bind(&server_interface<T>::accept_handler, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

template <typename T>
void server_interface<T>::accept_handler(const asio::error_code& error,
                                         asio::ip::tcp::socket peer) {
    WIRED_LOG_MESSAGE(wired::LOG_INFO, "Someone connected");
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
bool server_interface<T>::remove_client(connection_ptr bad_connection) {
    std::mutex& mutex = connections_.get_mutex();
    std::unique_lock<std::mutex> lock(mutex);
    bool not_found;
    do {
        not_found = true;
        auto& it = connections_.begin();
        for (; it != connections_.end(); ++it) {
            auto& connection = *it;
            if (connection == bad_connection) {
                not_found = false;
                break;
            }
        }
        if (!not_found) {
            connections_.erase(it);
        }
    } while (!not_found);
}

} // namespace wired

#endif // WIRED_SERVER_H