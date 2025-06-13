#pragma once

class Game;

#include <cstdint>
#include <thread>
#include <string>

#include "common.h"
#include "tic-tac-toe.h"
#include "wired/client.h"

class client : public wired::client_interface<message_types> {
  public:
    client() : game(this) {}

    client(const client&) = delete;
    client& operator=(const client&) = delete;

    ~client() = default;

    void on_message(message_t& msg, connection_ptr conn) override;

    void wait_until_opponent_move(uint32_t& row, uint32_t& col);

    void play();

  public:
    bool in_game = false;
    std::string name;
    uint32_t id;
    Game game;
    uint32_t opponent_row = 0;
    uint32_t opponent_col = 0;
    bool waiting_for_opponent_move = false;
};
