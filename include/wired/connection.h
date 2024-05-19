#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include "wired/message.h"
#include "wired/ts_deque.h"
#include "wired/types.h"


namespace wired {

template <typename T>
class connection {
  public:
    using message_t = message<T>;
    using connection_t = connection<T>;
    using connection_ptr = std::shared_ptr<connection_t>;

  public:
    connection(asio::io_context& io_context, asio::ip::tcp::socket socket);
    connection(const connection& other) = delete;
    connection(connection&& other);
    virtual ~connection();

    connection& operator=(const connection&& other) = delete;
    connection& operator=(connection&& other);

    bool is_connected() const;
    bool send(const message_t& msg,
              message_strategy strategy = message_strategy::normal);

    void run(run_strategy strategy = run_strategy::non_blocking);

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    ts_deque<message_t> messages_;
    message_t aux_message_;
};

} // namespace wired

#endif