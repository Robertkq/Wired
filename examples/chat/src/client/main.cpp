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

    std::cout << std::format("{}, you have connected to the server\n",
                             chatter.name);

    std::cout << "You can start chatting now. Type 'exit' to quit.\n";

    chatter.run(wired::execution_policy::non_blocking);

    while (chatter.is_connected()) {
        std::string message;
        std::getline(std::cin, message);
        if (message == "exit") {
            break;
        }
        if (message.empty()) {
            continue;
        }
        std::cout << "DEBUG: Sending message: " << message << std::endl;
        wired::message<message_types> msg(message_types::client_message);
        msg << chatter.name;
        msg << message;
        chatter.send(msg);
    }

    chatter.disconnect();

    return 0;
}