#include <iostream>
#include <format>

#include "client.h"

int main() {
    WIRED_LOG_LEVEL(wired::log_level::LOG_DEBUG);
    client chatter;

    std::cout << "Enter your name: ";
    std::cin >> chatter.name;

    auto future = chatter.connect(SERVER_ADDR, SERVER_PORT);
    bool connected = future.get();
    assert(connected);
    assert(chatter.is_connected());
    std::cout << std::format("{}, you have connected to the server\n",
                             chatter.name);

    chatter.update_thread = std::thread([&]() {
        while (chatter.is_connected()) {
            chatter.update();
        }
    });
    chatter.update_thread.detach();
    std::cout << "You can start chatting now. Type 'exit' to quit.\n";

    while (chatter.is_connected()) {
        std::string message;
        std::getline(std::cin, message);
        if (message == "exit") {
            break;
        }
        if (message.empty()) {
            continue;
        }
        wired::message<message_types> msg(message_types::client_message);
        msg << chatter.name;
        msg << message;
        chatter.send(msg);
    }

    chatter.disconnect();

    return 0;
}