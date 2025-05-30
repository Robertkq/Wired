#include <iostream>

#include "server.h"

int main() {
    WIRED_LOG_LEVEL(wired::log_level::LOG_DEBUG);
    server serv;
    serv.start(SERVER_PORT);

    std::cout << "Server started on port " << SERVER_PORT << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;

    serv.run(wired::execution_policy::blocking);

    serv.shutdown();

    return 0;
}