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
    virtual void on_message(message_t& msg, connection_ptr conn);

  public:
    client_interface();
    client_interface(const client_interface& other) = delete;
    client_interface(client_interface&& other);
    virtual ~client_interface();

    client_interface& operator=(const client_interface&& other) = delete;
    client_interface& operator=(client_interface&& other);

    bool connect(const std::string& host, const std::string& port);
    void disconnect();
    bool is_connected() const;
    void send(const message_t& msg);

  private:
    connection_ptr connection_;
    ts_deque<message_t> messages_in_;
    asio::io_context context_;
}; // class client_interface

template <typename T>
client_interface<T>::client_interface()
    : connection_(nullptr), messages_in_() {}

template <typename T>
client_interface<T>::client_interface(client_interface&& other)
    : connection_(std::move(other.connection_)),
      messages_in_(std::move(other.messages_in_)) {}

template <typename T>
client_interface<T>::~client_interface() {
    if (is_connected()) {
        disconnect();
    }
}
template <typename T>
client_interface<T>& client_interface<T>::operator=(client_interface&& other) {
    if (this == &other) {
        return *this;
    }

    connection_ = std::move(other.connection_);
    messages_in_ = std::move(other.messages_in_);

    return *this;
}

template <typename T>
bool client_interface<T>::connect(const std::string& host,
                                  const std::string& port) {
    if (is_connected()) {
        disconnect();
    }
    try {
        asio::ip::tcp::resolver resolver(context_);
        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(host, port);

        connection_ = std::make_shared<connection_ptr>(
            context_, asio::ip::tcp::socket(context_), messages_in_);

        bool connection_result = connection_->connect(endpoints);
        return connection_result;

    } catch (std::exception ec) {
        disconnect();
        return false;
    }
}

template <typename T>
void client_interface<T>::disconnect() {
    connection_->disconnect();
    connection_.reset();
}

template <typename T>
bool client_interface<T>::is_connected() const {
    return connection_->is_connected();
}

template <typename T>
void client_interface<T>::send(const message_t& msg) {
    connection_->send(msg);
}

} // namespace wired

#endif // WIRED_CLIENT_H