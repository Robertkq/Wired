#include "server.h"
#include <iostream>

server::server() {}

server::~server() {}

void server::on_message(message_t& msg, connection_ptr conn) {
    switch (msg.id()) {
    case message_types::client_message:
        on_client_message(msg, conn);
        break;
    default:
        std::cerr << "Unknown message type: ";
        break;
    }
}

void server::on_client_message(message_t& msg, connection_ptr conn) {
    send_all(conn, msg);
}