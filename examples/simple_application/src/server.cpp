#include "common.h"
#include "wired.h"

#include <iostream>

class server : public wired::server_interface<common_messages> {
  public:
    server() : wired::server_interface<common_messages>() {}
    virtual ~server() {}

    void on_message(message_t& msg, connection_ptr conn) override {
        switch (msg.id()) {
        case common_messages::client_ping: {
            std::cout << "[server]: I got a ping from the client!\n";
            message_t answer{common_messages::server_ping};
            sleep(1);
            send(conn, answer);
            break;
        }
        default: {
            std::cout << "unhandled message type: "
                      << static_cast<uint32_t>(msg.id()) << "\n";
            break;
        }
        }
    }
};

int main() {
    WIRED_LOG_LEVEL(wired::log_level::LOG_DEBUG);
    server simple_server;
    std::cout << "[server]: starting\n";
    simple_server.start("60000");

    std::cout << "[server]: running\n";
    while (true) {
        simple_server.update();
    }

    std::cout << "[server]: stopping\n";
    simple_server.stop();

    return 0;
}