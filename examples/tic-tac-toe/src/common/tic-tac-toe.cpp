#include "tic-tac-toe.h"
#include "client.h"
#include <chrono>
#include <thread>

Game::Game(client* conn) : conn(conn) {
    // Initialize the board to empty state
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            board[i][j] = ' ';
        }
    }
}

void Game::cool_board_print() {
    std::cout << "Current board state:\n";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            std::cout << board[i][j];
            if (j < 2)
                std::cout << " | ";
        }
        std::cout << "\n";
        if (i < 2)
            std::cout << "---------\n";
    }
}

uint32_t Game::play() {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            board[i][j] = ' ';
        }
    }
    if (self_turn()) {
        my_symbol = 'X';
        opponent_symbol = 'O';
    } else {
        my_symbol = 'O';
        opponent_symbol = 'X';
    }
    while (true) {
        clear_screen();
        print_basic_info();
        cool_board_print();
        if (self_turn()) {
            std::cout << "Your turn! Enter your move (row and column):\n";
            uint32_t row, col;
            do {
                std::cout << "Row (0-2): ";
                std::cin >> row;
                std::cout << "Column (0-2): ";
                std::cin >> col;
            } while (row < 0 || row > 2 || col < 0 || col > 2 ||
                     board[row][col] != ' ');
            board[row][col] = my_symbol;
            wired::message<message_types> move_msg(message_types::client_move);
            move_msg << self << row << col;
            auto send_future = conn->send(move_msg);
            send_future.wait();
            std::cout << "Move sent: (" << row << ", " << col << ")\n";

        } else {
            std::cout << "Waiting for opponent's move...\n";
            uint32_t row, col;
            conn->wait_until_opponent_move(row, col);
            std::cout << "Opponent moved: (" << row << ", " << col << ")\n";
            board[row][col] = opponent_symbol;
        }
        uint32_t winner;
        bool game_over = check_win(winner);
        if (game_over) {
            clear_screen();
            print_basic_info();
            cool_board_print();
            std::cout << "Game over! ";
            if (winner == self) {
                std::cout << "You win!\n";
                wired::message<message_types> game_over_msg(
                    message_types::client_game_over);
                game_over_msg << self;
                conn->send(game_over_msg);
            } else if (winner == UINT32_MAX) {
                std::cout << "It's a draw!\n";
            } else {
                std::cout << "You lose! Opponent wins.\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            clear_screen();
            std::cout << "You can start playing again.\n"
                         "Type 'play' to queue up for a game.\n"
                         "Type 'exit' to quit.\n";
            return winner;
        }

        turn++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool Game::check_win(uint32_t& winner) {
    for (int i = 0; i < 3; ++i) {
        // Check rows
        if (board[i][0] != ' ' && board[i][0] == board[i][1] &&
            board[i][1] == board[i][2]) {
            winner = (board[i][0] == my_symbol) ? self : get_opponent_id();
            return true;
        }
        // Check columns
        if (board[0][i] != ' ' && board[0][i] == board[1][i] &&
            board[1][i] == board[2][i]) {
            winner = (board[0][i] == my_symbol) ? self : get_opponent_id();
            return true;
        }
    }
    // Check diagonals
    if (board[0][0] != ' ' && board[0][0] == board[1][1] &&
        board[1][1] == board[2][2]) {
        winner = (board[0][0] == my_symbol) ? self : get_opponent_id();
        return true;
    }
    if (board[0][2] != ' ' && board[0][2] == board[1][1] &&
        board[1][1] == board[2][0]) {
        winner = (board[0][2] == my_symbol) ? self : get_opponent_id();
        return true;
    }
    // Check for draw
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == ' ') {
                return false; // game is still ongoing
            }
        }
    }
    winner = UINT32_MAX; // Indicate a draw
    return true;
}

void Game::clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

uint32_t Game::id_turn() {
    if (turn % 2 == 0) {
        return player1;
    } else {
        return player2;
    }
}

void Game::print_basic_info() {
    std::cout << "Player 1: " << name1 << " (ID: " << player1 << ")\n";
    std::cout << "Player 2: " << name2 << " (ID: " << player2 << ")\n";
    std::cout << "Your ID: " << self << "\n";
    std::cout << "Your name: " << get_self_name() << "\n";
    std::cout << "Opponent's name: " << get_opponent_name() << "\n";
    std::cout << "Current turn: " << turn << "\n";
    std::cout << "Current owner: " << id_turn() << "\n";
}