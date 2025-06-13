#include <iostream>
#include <format>

#include "client.h"
#include "common.h"
#include "tic-tac-toe.h"

int main() {
    WIRED_LOG_LEVEL(wired::log_level::LOG_DEBUG);
    client chatter;

    std::cout << "Enter your name: ";
    std::cin >> chatter.name;

    auto future = chatter.connect(SERVER_ADDR, SERVER_PORT);
    bool connected = future.get();
    assert(connected);

    wired::message<message_types> msg(message_types::client_hello);
    msg << chatter.name;
    future = chatter.send(msg);
    future.wait();

    std::cout << std::format("{}, you have connected to the server\n",
                             chatter.name);

    std::cout << "You can start playing now.\n"
                 "Type 'play' to queue up for a game.\n"
                 "Type 'exit' to quit.\n";

    chatter.run(wired::execution_policy::non_blocking);

    while (chatter.is_connected()) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string input;
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "play") {
            wired::message<message_types> start_msg(
                message_types::client_want_to_play);
            start_msg << chatter.id;
            auto send_future = chatter.send(start_msg);
            send_future.wait();
            std::cout << "You have queued up for a game.\n";
            chatter.play();
        } else if (input == "exit") {
            std::cout << "Exiting...\n";
            break;
        } else {
            std::cout << "Unknown command. Please type 'play' or 'exit'.\n";
        }
    }

    chatter.disconnect();

    return 0;
}
