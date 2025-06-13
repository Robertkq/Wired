#include "server.h"
#include <iostream>

// client_hello,
//     client_want_to_play,
//     client_move,
//     client_game_over

void server::on_message(message_t& msg, connection_ptr conn) {
    switch (msg.id()) {
    case message_types::client_hello: {
        uint32_t client_id = next_client_id++;
        std::string client_name;
        msg >> client_name;
        clients_name[client_id] = client_name;
        clients[client_id] = conn;
        std::cout << "Client connected: " << clients[client_id]
                  << " (ID: " << client_id << ", name: " << client_name << ")"
                  << std::endl;
        message_t response{message_types::client_hello};
        response << client_id;
        send(conn, response);
        break;
    }
    case message_types::client_want_to_play: {
        uint32_t client_id;
        msg >> client_id;
        std::cout << "Client " << clients[client_id]
                  << " wants to play (ID: " << client_id << ")" << std::endl;
        queued_clients.push_back(client_id);
        check_if_game_can_start();
        break;
    }
    case message_types::client_move: {
        uint32_t player_id, row, col;
        msg >> col >> row >> player_id;

        std::cout << "Player " << clients_name[player_id]
                  << " (ID: " << player_id << ") made a move at (" << row
                  << ", " << col << ")" << std::endl;

        // Broadcast the move to both players in the game
        for (const auto& game : games) {
            if (game.first == player_id || game.second == player_id) {
                message_t move_msg{message_types::client_move};
                move_msg << player_id << row << col;
                if (game.first == player_id) {
                    send(clients[game.second], move_msg);
                } else {
                    send(clients[game.first], move_msg);
                }
                break;
            }
        }
        break;
    }
    case message_types::client_game_over: {
        uint32_t winner_id;
        msg >> winner_id;
        std::cout << "Game over! Winner: " << clients_name[winner_id]
                  << " (ID: " << winner_id << ")" << std::endl;
        for (auto it = games.begin(); it != games.end();) {
            if (it->first == winner_id || it->second == winner_id) {
                it = games.erase(it);
                std::cout << "Game removed from active games." << std::endl;

            } else {
                ++it;
            }
        }
    }
    }
}

void server::check_if_game_can_start() {
    if (queued_clients.size() >= 2) {
        uint32_t player1_id = queued_clients[0];
        uint32_t player2_id = queued_clients[1];
        queued_clients.erase(queued_clients.begin(),
                             queued_clients.begin() + 2);
        games.emplace_back(player1_id, player2_id);

        std::cout << "Starting game between " << clients_name[player1_id]
                  << " (ID: " << player1_id << ") and "
                  << clients_name[player2_id] << " (ID: " << player2_id << ")"
                  << std::endl;

        auto conn1 = clients[player1_id];
        auto conn2 = clients[player2_id];

        uint32_t turn = rand() % 2;

        message_t start_game_msg{message_types::client_game_start};
        start_game_msg << player1_id << clients_name[player1_id] << player2_id
                       << clients_name[player2_id] << turn;
        send(conn1, start_game_msg);
        send(conn2, start_game_msg);
    }
}
