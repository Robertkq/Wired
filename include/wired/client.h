#ifndef WIRED_CLIENT_H
#define WIRED_CLIENT_H

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
    using message = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    virtual void on_message(const message& msg, connection_ptr conn);

  public:
    client_interface();
    client_interface(const client_interface& other) = delete;
    client_interface(client_interface&& other);
    virtual ~client_interface();

    client_interface& operator=(const client_interface&& other) = delete;
    client_interface& operator=(client_interface&& other);

    bool connect(const std::string& host, const std::string& port,
                 connection_strategy strategy = connection_strategy::once);
    bool disconnect();
    bool is_connected() const;
    bool send(const message& msg,
              message_strategy strategy = message_strategy::normal);

    void run(run_strategy strategy = run_strategy::non_blocking);

  private:
    connection_ptr connection_;
    ts_deque<message> messages_;
}; // class client_interface
template <typename T>
client_interface<T>::client_interface() {}

template <typename T>
client_interface<T>::client_interface(client_interface&& other) {}

template <typename T>
client_interface<T>::~client_interface() {}

template <typename T>
client_interface<T>& client_interface<T>::operator=(client_interface&& other) {}

template <typename T>
bool client_interface<T>::connect(const std::string& host,
                                  const std::string& port,
                                  connection_strategy strategy) {}

template <typename T>
bool client_interface<T>::disconnect() {}

template <typename T>
bool client_interface<T>::is_connected() const {}

template <typename T>
bool client_interface<T>::send(const message& msg, message_strategy strategy) {}

template <typename T>
void client_interface<T>::run(run_strategy strategy) {}

} // namespace wired

#endif // WIRED_CLIENT_H