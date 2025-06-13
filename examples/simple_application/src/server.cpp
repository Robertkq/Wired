#include "common.h"
#include "wired.h"

#include <iostream>

using namespace wired;

class server : public server_interface<common_messages> {
  public:
    server() : server_interface<common_messages>() {}
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
    WIRED_LOG_LEVEL(log_level::LOG_DEBUG);
    wired::tls_options tls_opts;
    tls_opts.set_certificate_file("server.crt")
        .set_private_key_file("server.key")
        .set_verify_mode(wired::tls_verify_mode::peer);
    server simple_server;
    simple_server.set_tls_options(tls_opts);
    std::cout << "[server]: starting\n";
    simple_server.start("60000");

    std::cout << "[server]: running\n";

    simple_server.run(execution_policy::blocking);

    std::cout << "[server]: stopping\n";
    simple_server.shutdown();

    return 0;
}