#include <cstdint>
#include <iostream>

enum class common_messages : uint8_t {
    server_ping,
    client_ping,
};