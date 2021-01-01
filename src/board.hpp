#pragma once

#include <array>
#include <iostream>
#include <typeindex>
#include <cassert>
#include <cstddef>
#include <cstdint>

#define XXH_INLINE_ALL
#include "xxhash.h"

enum piece_type : uint8_t {
    red,
    green,
    blue,
    purple
};

// This probably only works on little-endian systems (and using a union for type-punning is a GNU extension).
union piece_bitboard {
    uint32_t bits;
    __extension__ struct {
        uint32_t row_4  : 5;
        uint32_t row_3  : 5;
        uint32_t row_2  : 5;
        uint32_t row_1  : 5;
        uint32_t type   : 2;
        uint32_t unused : 10;
    };

    piece_bitboard(piece_type type, uint8_t row_1, uint8_t row_2, uint8_t row_3, uint8_t row_4)
      : row_4 {row_4}, row_3 {row_3}, row_2 {row_2}, row_1 {row_1}, type {type}, unused {0} { }

    piece_bitboard(uint32_t type, uint32_t row_1, uint32_t row_2, uint32_t row_3, uint32_t row_4)
      : row_4 {row_4}, row_3 {row_3}, row_2 {row_2}, row_1 {row_1}, type {type}, unused {0} { }

    piece_bitboard(uint32_t bits)
      : bits {bits} { }

    bool operator==(const piece_bitboard &rhs) const {
        return bits == rhs.bits;
    }
};

static_assert(sizeof(piece_bitboard) == sizeof(uint32_t));

class game_board {
private:
    inline bool move_piece_common(piece_bitboard &piece, piece_bitboard new_piece) {
        uint32_t all_pieces_mask = 0;
        for (auto piece : pieces) {
            all_pieces_mask |= piece.bits;
        }

        all_pieces_mask &= ~(piece.bits);
        if (new_piece.bits & all_pieces_mask) {
            return false;
        }

        piece = new_piece;
        return true;
    }

public:
    static constexpr uint32_t solution_mask     = 0b00000'00011'00011'00000UL;
    static constexpr uint32_t top_row_mask      = 0b11111'00000'00000'00000UL;
    static constexpr uint32_t bottom_row_mask   = 0b00000'00000'00000'11111UL;
    static constexpr uint32_t left_column_mask  = 0b10000'10000'10000'10000UL;
    static constexpr uint32_t right_column_mask = 0b00001'00001'00001'00001UL;

    std::array<piece_bitboard, 10> pieces = {{
        { piece_type::blue,   0b11000, 0b00000, 0b00000, 0b00000 },
        { piece_type::blue,   0b00110, 0b00000, 0b00000, 0b00000 },
        { piece_type::green,  0b00001, 0b00000, 0b00000, 0b00000 },
        { piece_type::red,    0b00000, 0b11000, 0b11000, 0b00000 },
        { piece_type::purple, 0b00000, 0b00100, 0b00100, 0b00000 },
        { piece_type::green,  0b00000, 0b00010, 0b00000, 0b00000 },
        { piece_type::green,  0b00000, 0b00000, 0b00010, 0b00000 },
        { piece_type::blue,   0b00000, 0b00000, 0b00000, 0b11000 },
        { piece_type::blue,   0b00000, 0b00000, 0b00000, 0b00110 },
        { piece_type::green,  0b00000, 0b00000, 0b00000, 0b00001 }
    }};

    static constexpr size_t red_index = 3;

    bool move_piece_up(piece_bitboard &piece) {
        if (piece.bits & top_row_mask) {
            return false;
        }

        piece_bitboard new_piece = { piece.type, piece.row_2, piece.row_3, piece.row_4, 0 };
        return move_piece_common(piece, new_piece);
    }

    bool move_piece_down(piece_bitboard &piece) {
        if (piece.bits & bottom_row_mask) {
            return false;
        }

        piece_bitboard new_piece = { piece.type, 0, piece.row_1, piece.row_2, piece.row_3 };
        return move_piece_common(piece, new_piece);
    }

    bool move_piece_left(piece_bitboard &piece) {
        if (piece.bits & left_column_mask) {
            return false;
        }

        piece_bitboard new_piece = {
            piece.type,
            static_cast<uint32_t>(piece.row_1 << 1U),
            static_cast<uint32_t>(piece.row_2 << 1U),
            static_cast<uint32_t>(piece.row_3 << 1U),
            static_cast<uint32_t>(piece.row_4 << 1U)
        };

        return move_piece_common(piece, new_piece);
    }

    bool move_piece_right(piece_bitboard &piece) {
        if (piece.bits & right_column_mask) {
            return false;
        }

        piece_bitboard new_piece = {
            piece.type,
            static_cast<uint32_t>(piece.row_1 >> 1U),
            static_cast<uint32_t>(piece.row_2 >> 1U),
            static_cast<uint32_t>(piece.row_3 >> 1U),
            static_cast<uint32_t>(piece.row_4 >> 1U)
        };

        return move_piece_common(piece, new_piece);
    }

    bool solved() const {
        return (pieces[red_index].bits & solution_mask) == solution_mask;
    }

    bool operator==(const game_board &rhs) const {
        return pieces == rhs.pieces;
    }
};

std::ostream& operator<<(std::ostream &os, const game_board &board) {
    static constexpr std::array<char, 4> piece_type_chars = {
        'R', // piece_type::red
        'G', // piece_type::green
        'B', // piece_type::blue
        'P'  // piece_type::purple
    };

    uint32_t mask = 0b10000'00000'00000'00000UL;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 5; j++) {
            bool piece_present = false;
            for (auto piece : board.pieces) {
                if (piece.bits & mask) {
                    os << piece_type_chars[piece.type];
                    piece_present = true;
                    break;
                }
            }

            if (!piece_present) {
                os << '_';
            }

            if (j < 4) {
                os << ' ';
            }

            mask >>= 1;
        }

        if (i < 3) {
            os << '\n';
        }
    }

    return os;
}

namespace std {
    template<> struct hash<game_board> {
        size_t operator()(const game_board& board) const noexcept {
            return XXH64(board.pieces.data(), board.pieces.size() * sizeof(board.pieces[0]), 39);
        }
    };
}
