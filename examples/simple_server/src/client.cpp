#include "common.h"
#include "wired.h"


#include <iostream>

class server : public wired::server_interface<message_type> {
  public:
    server() : wired::server_interface<message_type>() {}
    virtual ~server() {}

    void on_message(message_t& msg, connection_ptr conn) override {
        switch (msg.header.type) {
        case message_type::server_ping: {
            std::cout << "[" << conn->get_id() << "]: ping\n";
            break;
        }
        case message_type::client_message: {
            std::cout << "[" << conn->get_id() << "]: " << msg.body << "\n";
            break;
        }
        default: {
            std::cout << "unhandled message type: "
                      << static_cast<uint32_t>(msg.header.type) << "\n";
            break;
        }
        }
    }
};

int main() { return 0; }