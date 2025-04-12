#include <cstdint>

constexpr const char* SERVER_ADDR = "localhost";
constexpr const char* SERVER_PORT = "60000";

enum class message_types : uint32_t {
    client_name,
    client_message,
    client_name_answer,
    server_message,
};