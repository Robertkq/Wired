#ifdef WIRED_SERVER_H
#define WIRED_SERVER_H

#include <wired/message.h>
#include <wired/types.h>

template <typename T>
class server_interface {
  public:
    using message = wired::message<T>;
    using connection = wired::connection<T>;
    using connection_ptr = std::shared_ptr<connection>;

  public:
    bool start();
    bool stop();

    bool send(connection_ptr conn, const message& msg,
              message_strategy strategy = message_strategy::normal);

  private:
    ts_deque<connection_ptr> connections_;
    ts_deque<message> messages_;
}; // class server_interface

#endif // WIRED_SERVER_H