#include "common.h"
#include "wired.h"

#include <iostream>
#include <format>

constexpr uint32_t num_of_pings = 3;

using namespace wired;

struct client : client_interface<common_messages> {
    client() : client_interface<common_messages>() {}
    void on_message(message_t& msg, connection_ptr conn) override {
        std::cout << "[client]: update\n";
        switch (msg.id()) {
        case common_messages::server_ping: {
            ++times_pinged;
            std::cout << std::format(
                "[client]: I got a ping from the server! === {}/{}\n",
                times_pinged, num_of_pings);

            if (times_pinged >= num_of_pings) {
                std::cout << "[client]: stopping\n";
                auto future = disconnect();
                future.wait();
                std::cout << "[client]: stopped\n";
                return;
            }
            message<common_messages> msg(common_messages::client_ping);
            send(msg);

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
    WIRED_LOG_LEVEL(log_level::LOG_DEBUG);
    client simple_client;
    std::cout << "[client]: starting\n";
    std::future<bool> connect_result =
        simple_client.connect("localhost", "60000");
    bool connected = connect_result.get();
    assert(connected);
    assert(simple_client.is_connected());
    std::cout << "[client]: connected\n";

    message<common_messages> msg(common_messages::client_ping);
    std::future<bool> send_result = simple_client.send(msg);
    bool sent = send_result.get();
    assert(sent);
    std::cout << "[client]: sent\n";

    std::cout << "[client]: running loop\n";

    simple_client.run(execution_policy::blocking);

    std::cout << "[client]: got out of the loop\n";

    return 0;
}