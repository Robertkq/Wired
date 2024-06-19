#pragma once

#include "wired.h"

enum class message_type {
    connect,
    disconnect,
    message
};

struct client : public wired::client_interface<message_type> {
  public:
    using message_t = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    void on_message(message_t& msg, connection_ptr conn) override{

    };
};