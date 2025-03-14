#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <map>

namespace Chess {

using Bitboard = uint64_t;

extern const std::array<Bitboard, 8> FILE_MASKS;
extern const std::array<Bitboard, 8> RANK_MASKS;
extern const std::array<Bitboard, 15> DIAGONALS;
extern const std::array<Bitboard, 15> ANTIDIAGONALS;

extern const Bitboard A1H8_DIAGONAL;
extern const Bitboard H1A8_DIAGONAL;
extern const Bitboard LIGHT_SQUARES;
extern const Bitboard DARK_SQUARES;
extern const Bitboard QUEENSIDE;
extern const Bitboard KINGSIDE;

enum Color { WHITE, BLACK, NONE };
Color reverseColor(Color c);

enum Piece : int { wP, wB, wN, wR, wQ, wK, bP, bB, bN, bR, bQ, bK, EMPTY };
std::string pieceToString(Piece p);
extern const std::map<std::string, Piece> STRING_TO_PIECE;
Color getPieceColor(Piece p);

enum PieceType { PAWN, BISHOP, KNIGHT, ROOK, QUEEN, KING };
Piece getCP(Color c, PieceType pt);
const int COLOR_INDEX_OFFSET = 6;

enum MoveType { QUIET, CAPTURE, KCASTLE, QCASTLE, PROMOTION, ENPASSANT, CAPTUREANDPROMOTION };

enum Direction { NORTH = 8, NW = 7, NE = 9, WEST = -1, EAST = 1, SW = -9, SOUTH = -8, SE = -7 };

enum FileIndexC { A_FILE, B_FILE, C_FILE, D_FILE, E_FILE, F_FILE, G_FILE, H_FILE };

extern std::array<Bitboard, 8> fileNeighbors;
void initNeighborMasks();

enum Rank { R1, R2, R3, R4, R5, R6, R7, R8 };

extern const std::array<Rank, 2> ALMOST_PROMOTION;
extern const std::array<Rank, 2> STARTING_RANK;
extern const std::array<Direction, 2> PAWN_PUSH_DIRECTION;

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    EMPTYSQ
};

extern const std::map<std::string, Square> STRING_TO_SQUARE;
extern const std::map<Square, std::string> SQUARE_TO_STRING;

Rank sqToRank(Square s);
FileIndexC sqToFile(Square s);
Bitboard sqToDiag(Square s);
Bitboard sqToAntiDiag(Square s);
Square goDirection(Square s, Direction d);

extern std::array<Bitboard, 64> KING_ATTACKS;
void initializeKingAttacks();

extern std::array<Bitboard, 64> KNIGHT_ATTACKS;
void initializeKnightAttacks();

extern std::array<Bitboard, 64> WHITE_PAWN_ATTACKS;
extern std::array<Bitboard, 64> BLACK_PAWN_ATTACKS;
void initializePawnAttacks();

extern const std::array<Bitboard, 64> S_TO_BB;
void initializeSQLookup();

Bitboard slidingAttacks(Square s, Bitboard blockers, Bitboard mask);

extern std::array<Bitboard, 64> BISHOP_MASKS;
extern std::array<Bitboard, 64> ROOK_MASKS;

extern std::array<std::array<Bitboard, 512>, 64> BISHOP_ATTACKS;
extern std::array<std::array<Bitboard, 4096>, 64> ROOK_ATTACKS;

extern std::array<Bitboard, 64> ROOK_MAGICS;
extern std::array<Bitboard, 64> BISHOP_MAGICS;

extern std::array<int, 64> ROOK_SHIFTS;
extern std::array<int, 64> BISHOP_SHIFTS;

Bitboard slidingBishopAttacks(Square s, Bitboard blockers);
void initBishopAttacks();
Bitboard slidingRookAttacks(Square s, Bitboard blockers);
void initRookAttacks();

extern std::array<std::array<Bitboard, 64>, 64> SQUARES_BETWEEN;
void initSquaresBetween();

extern std::array<std::array<Bitboard, 64>, 64> LINE;
void initLine();

extern std::array<std::array<Bitboard, 64>, 13> ZOBRIST_TABLE;
extern Bitboard TURN_HASH;
void initZobrist();

template<typename T>
bool contains(const std::vector<T>& vec, const T& element);

bool checkSameElements(const std::vector<std::string>& a, const std::vector<std::string>& b);

template<typename T>
void UNUSED(const T& x);

}

#endif
