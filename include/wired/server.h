#ifdef WIRED_SERVER_H
#define WIRED_SERVER_H

#include "wired/message.h"
#include "wired/types.h"
#include <string>

namespace wired {
template <typename T>
class server_interface {
  public:
    using message = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    virtual void on_message(message_t& msg, connection_ptr conn);

  public:
    server_interface();

  public:
    bool start(const std::string& port);
    bool stop();

    bool is_listening();

    bool send(connection_ptr conn, const message& msg,
              message_strategy strategy = message_strategy::normal);

    bool send_all(connection_ptr ignore);
    bool update();

  private:
    void wait_for_client_chain();

  private:
    asio::io_context context_;
    asio::ip::tcp::listener listener_;
    ts_deque<connection_ptr> connections_;
    ts_deque<message> messages_;
}; // class server_interface

} // namespace wired

#endif // WIRED_SERVER_H