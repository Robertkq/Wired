#pragma once
#include <cstdint>

constexpr const char* SERVER_ADDR = "localhost";
constexpr const char* SERVER_PORT = "60000";

enum class message_types : uint32_t {
    client_hello,
    client_want_to_play,
    client_game_start,
    client_move,
    client_game_over
};