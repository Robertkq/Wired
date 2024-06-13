#pragma once

#include <cstdint>

enum class message_type : uint8_t {
    single,
    vector,
    wired_serialize
};