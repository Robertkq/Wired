#pragma once

#include <cstdint>
#include <fstream>
#include <gtest/gtest.h>

enum class message_type : uint8_t {
    single,
    vector,
    wired_serialize,
    client_message,
    server_message,

    message_type_count // This should be the last enum value
};

inline void verify_certificates() {
    std::ifstream cert_file("../server.crt");
    if (!cert_file.is_open()) {
        FAIL() << "Server certificate not found at \"../server.crt\". "
                  "Generate them using generate_cert.py script.";
    }
    cert_file.close();
    std::ifstream key_file("../server.key");
    if (!key_file.is_open()) {
        FAIL() << "Server key not found at \"../server.key\". Generate "
                  "them using generate_cert.py script.";
    }
    key_file.close();
}