#include <iostream>
#include <optional>
#include <queue>
#include <unordered_map>
#include <cstddef>

#include "board.hpp"

typedef bool (game_board::*move_func) (piece_bitboard &);

static size_t print_solution(game_board board,
                             const std::unordered_map<game_board, std::optional<game_board>> &predecessors) {
    size_t move = 0;
    auto predecessor = predecessors.at(board);
    if (predecessor) {
        move = print_solution(*predecessor, predecessors) + 1;
        std::cout << "Move " << move << ":\n" << *predecessor << '\n';
    }

    return move;
}

int main() {
    game_board board;
    std::queue<game_board> queue;
    queue.push(board);
    std::unordered_map<game_board, std::optional<game_board>> predecessors;
    predecessors.insert(std::make_pair(board, std::nullopt));

    move_func move_pairs[12][2] = {
        {&game_board::move_piece_up, &game_board::move_piece_up},
        {&game_board::move_piece_down, &game_board::move_piece_down},
        {&game_board::move_piece_left, &game_board::move_piece_left},
        {&game_board::move_piece_right, &game_board::move_piece_right},
        {&game_board::move_piece_up, &game_board::move_piece_right},
        {&game_board::move_piece_up, &game_board::move_piece_left},
        {&game_board::move_piece_right, &game_board::move_piece_up},
        {&game_board::move_piece_right, &game_board::move_piece_down},
        {&game_board::move_piece_left, &game_board::move_piece_up},
        {&game_board::move_piece_left, &game_board::move_piece_down},
        {&game_board::move_piece_down, &game_board::move_piece_right},
        {&game_board::move_piece_down, &game_board::move_piece_left},  
    };

    while (!queue.empty()) {
        game_board new_board = queue.front();
        queue.pop();
        if (new_board.solved()) {
            std::cout << "Found solution\n\n";
            print_solution(new_board, predecessors);
            std::cout << "Solution:\n" << new_board << '\n';
            break;
        }

        game_board prev_board = new_board;
        for (auto &piece : new_board.pieces) {
            new_board = prev_board;
            if (new_board.move_piece_up(piece) && predecessors.find(new_board) == predecessors.end()) {
                queue.push(new_board);
                predecessors[new_board] = prev_board;
            }

            new_board = prev_board;
            if (new_board.move_piece_down(piece) && predecessors.find(new_board) == predecessors.end()) {
                queue.push(new_board);
                predecessors[new_board] = prev_board;
            }

            new_board = prev_board;
            if (new_board.move_piece_left(piece) && predecessors.find(new_board) == predecessors.end()) {
                queue.push(new_board);
                predecessors[new_board] = prev_board;
            }

            new_board = prev_board;
            if (new_board.move_piece_right(piece) && predecessors.find(new_board) == predecessors.end()) {
                queue.push(new_board);
                predecessors[new_board] = prev_board;
            }

            if (piece.type == green) {
                for (int i = 0; i < 12; i++) {
                    new_board = prev_board;
                    if (new_board.double_move(piece, move_pairs[i][0], move_pairs[i][1]) && predecessors.find(new_board) == predecessors.end()) {
                        queue.push(new_board);
                        predecessors[new_board] = prev_board;
                    }
                }
            } else if (piece.type == blue) {
                for (int i = 2; i < 4; i++) {
                    new_board = prev_board;
                    if (new_board.double_move(piece, move_pairs[i][0], move_pairs[i][1]) && predecessors.find(new_board) == predecessors.end()) {
                        queue.push(new_board);
                        predecessors[new_board] = prev_board;
                    }
                }
            } else if (piece.type == purple) {
                for (int i = 0; i < 2; i++) {
                    new_board = prev_board;
                    if (new_board.double_move(piece, move_pairs[i][0], move_pairs[i][1]) && predecessors.find(new_board) == predecessors.end()) {
                        queue.push(new_board);
                        predecessors[new_board] = prev_board;
                    }
                }
            }
        }
    }
}
