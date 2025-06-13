#pragma once

#include <unordered_map>

#include "common.h"
#include "wired/server.h"

class server : public wired::server_interface<message_types> {
  public:
    server() = default;

    server(const server&) = delete;
    server& operator=(const server&) = delete;

    ~server() = default;

    void on_message(message_t& msg, connection_ptr conn) override;
    void check_if_game_can_start();

  private:
    uint32_t next_client_id = 0;
    std::unordered_map<uint32_t, std::string> clients_name;
    std::unordered_map<uint32_t, connection_ptr> clients;
    std::vector<uint32_t> queued_clients;
    std::vector<std::pair<uint32_t, uint32_t>> games;
};