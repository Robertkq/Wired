#pragma once

class client;

#include <iostream>
#include <cstdint>

#include "common.h"
#include "wired/connection.h"

class Game {
  public:
    Game(client* conn);
    uint32_t play();

    void clear_screen();

    uint32_t id_turn();

    bool self_turn() { return self == id_turn(); }

    uint32_t get_self_id() { return self; }
    std::string get_self_name() { return (self == player1) ? name1 : name2; }
    uint32_t get_opponent_id() { return (self == player1) ? player2 : player1; }
    std::string get_opponent_name() {
        return (self == player1) ? name2 : name1;
    }
    void cool_board_print();
    void print_basic_info();

    bool check_win(uint32_t& winner);

  public:
    char board[3][3];
    uint32_t self;
    uint32_t player1;
    std::string name1;
    uint32_t player2;
    std::string name2;
    uint32_t turn;
    client* conn;
    char my_symbol = 'U';
    char opponent_symbol = 'U';
};
