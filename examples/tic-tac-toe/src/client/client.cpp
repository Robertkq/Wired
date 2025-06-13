#include "client.h"
#include <iostream>

void client::on_message(message_t& msg, connection_ptr conn) {
    switch (msg.id()) {
    case message_types::client_hello: {

        msg >> id;
        game.self = id;
        std::cout << "Connected to server with ID: " << id << std::endl;

        break;
    }
    case message_types::client_game_start: {
        msg >> game.turn;
        msg >> game.name2;
        msg >> game.player2;
        msg >> game.name1;
        msg >> game.player1;

        std::cout << "Game started between " << game.name1
                  << " (ID: " << game.player1 << ") and " << game.name2
                  << " (ID: " << game.player2 << ").\n";

        in_game = true;
        break;
    }
    case message_types::client_move: {
        uint32_t player_id, row, col;
        msg >> col >> row >> player_id;

        opponent_row = row;
        opponent_col = col;
        waiting_for_opponent_move = false;
        std::cout << "Opponent moved: (" << row << ", " << col
                  << ") by player ID: " << player_id << std::endl;

        break;
    }
    }
}

void client::wait_until_opponent_move(uint32_t& row, uint32_t& col) {
    waiting_for_opponent_move = true;
    while (waiting_for_opponent_move) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    row = opponent_row;
    col = opponent_col;
}

void client::play() {
    while (!in_game) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    game.play();
    in_game = false;
}