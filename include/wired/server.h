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
    server_interface();

  public:
    bool start(const std::string& port);
    bool stop();

    bool send(connection_ptr conn, const message& msg,
              message_strategy strategy = message_strategy::normal);

  private:
    void wait_for_client_chain();

  private:
    asio::io_context context_;
    asio::ip::tcp::listener listener_;
    ts_deque<connection_ptr> connections_;
    ts_deque<message> messages_;
}; // class server_interface

template <typename T>
server_interface<T>::server_interface()
    : context_(), listener_(context_), connections_(), messages_() {}

template <typename T>
bool server_interface<T>::start(const std::string& port) {
    try {
        asio::ip::tcp::endpoint myself(asio::ip::tcp::v4(), std::stoi(port));
        listener_.open(myself.protocol());
        listener_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        listener_.bind(myself);
        listener_.listen();
        wait_for_client_chain();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

template <typename T>
void server_interface<T>::wait_for_client_chain() {
    asio::async_accept(
        listener_, [this](asio::error_code ec, asio::ip::tcp::socket socket) {
            if (ec) {

            } else {
                connection_ptr new_connection =
                    std::make_shared<connection_t>(context_);

                std::cout << socket.remote_endpoint() << " connected!\n";

                connections_.push_back(new_connection);
            }
        })
}

} // namespace wired

#endif // WIRED_SERVER_H