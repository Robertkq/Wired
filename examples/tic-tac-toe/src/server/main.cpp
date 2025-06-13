#include <iostream>

#include "server.h"

int main() {
    WIRED_LOG_LEVEL(wired::log_level::LOG_DEBUG);
    wired::tls_options tls_opts;
    tls_opts.set_certificate_file("server.crt")
        .set_private_key_file("server.key")
        .set_verify_mode(wired::tls_verify_mode::peer);
    server serv;
    serv.set_tls_options(tls_opts);
    serv.start(SERVER_PORT);

    std::cout << "Server started on port " << SERVER_PORT << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;

    serv.run(wired::execution_policy::blocking);

    serv.shutdown();

    return 0;
}