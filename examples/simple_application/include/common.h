#include <cstdint>
#include <iostream>

enum class common_messages : uint32_t {
    server_accept,
    server_deny,
    server_ping,
    server_message,
    client_ping,
    client_message
};