#include "client.h"
#include <iostream>

client::client() {}

client::~client() {}

void client::on_message(message_t& msg, connection_ptr conn) {
    switch (msg.id()) {
    case message_types::client_message: {
        std::string message;
        std::string name;
        msg >> message >> name;
        std::cout << std::format("{}: {}", name, message) << std::endl;
    } break;
    default:
        std::cerr << "Unknown message type: ";
        break;
    }
}