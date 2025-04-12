#include "common.h"
#include "wired.h"

#include <iostream>

struct client : wired::client_interface<common_messages> {
    client() : wired::client_interface<common_messages>() {}
    void on_message(message_t& msg, connection_ptr conn) override {
        if (times_pinged >= 3) {
            std::cout << "[client]: stopping\n";
            disconnect();
            std::cout << "[client]: stopped\n";
        }
        std::cout << "[client]: update\n";
        switch (msg.id()) {
        case common_messages::server_ping: {
            std::cout << "[client]: I got a ping from the server!\n";
            sleep(1);
            wired::message<common_messages> msg(common_messages::client_ping);
            send(msg);
            ++times_pinged;
            break;
        }
        default: {
            std::cout << "unhandled message type: "
                      << static_cast<uint32_t>(msg.id()) << "\n";
            break;
        }
        }
    }
    uint32_t times_pinged{0};
};

int main() {
    WIRED_LOG_LEVEL(wired::log_level::LOG_DEBUG);
    client simple_client;
    std::cout << "[client]: starting\n";
    std::future<bool> connect_result =
        simple_client.connect("localhost", "60000");
    bool connected = connect_result.get();
    assert(connected);
    assert(simple_client.is_connected());
    std::cout << "[client]: connected\n";

    wired::message<common_messages> msg(common_messages::client_ping);
    std::future<bool> send_result = simple_client.send(msg);
    bool sent = send_result.get();
    assert(sent);
    std::cout << "[client]: sent\n";

    std::cout << "[client]: running updated loop\n";
    while (simple_client.is_connected()) {
        simple_client.update();
    }

    std::cout << "[client]: got out of the loop\n";

    return 0;
}