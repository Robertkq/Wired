#include <cstdint>
#include <iostream>

enum class message_type : uint32_t {
    server_accept,
    server_deny,
    server_ping,
    server_message,
    client_ping,
    client_message
};