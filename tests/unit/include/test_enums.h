#pragma once

#include <cstdint>

enum class message_type : uint8_t {
    single,
    vector,
    wired_serialize,
    client_message,
    server_message,

    message_type_count // This should be the last enum value
};