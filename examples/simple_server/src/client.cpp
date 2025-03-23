#include "common.h"
#include "wired.h"

#include <iostream>

struct client : wired::client_interface<common_messages> {
    client() : wired::client_interface<common_messages>() {}
    void on_message(message_t& msg, connection_ptr conn) override {
        switch (msg.id()) {
        case common_messages::server_ping: {
            std::cout << "I got a ping from the server!\n";

            wired::message<common_messages> msg(common_messages::client_ping);
            send(msg);
            break;
        }
        case common_messages::client_message: {
            std::cout << "[]: something \n";
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
    client simple_client;
    std::cout << "[client]: starting\n";
    simple_client.connect("localhost", "60000");

    sleep(2);
    wired::message<common_messages> msg(common_messages::client_ping);
    simple_client.send(msg);

    std::cout << "[client]: running\n";
    while (true) {
        simple_client.update();
    }

    std::cout << "[client]: stopping\n";

    return 0;
}