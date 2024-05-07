#ifndef WIRED_CLIENT_H
#define WIRED_CLIENT_H

#include <wired/types.h>

#include <asio.hpp>
#include <iostream>

namespace wired {
template <typename T>
class client_interface {
  public:
    client_interface();
    client_interface(const client_interface& other) = delete;
    client_interface(client_interface&& other);
    virtual ~client_interface();

    client_interface& operator=(const client_interface&& other) = delete;
    client_interface& operator=(client_interface&& other);

    bool connect(const std::string& host, const std::string& port,
                 connection_strategy strategy = connection_strategy::none);

  private:
};

template <typename T>
client_interface<T>::client_interface() {}

template <typename T>
client_interface<T>::client_interface(client_interface&& other) {}

template <typename T>
client_interface<T>::~client_interface() {}

template <typename T>
client_interface<T>& client_interface<T>::operator=(client_interface&& other) {}

} // namespace wired

#endif // WIRED_CLIENT_H

} // namespace wired