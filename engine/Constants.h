#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <map>

namespace Chess {

using u64 = uint64_t;

// Bitboard constants using little-endian rank-file mapping
extern const std::array<u64, 8> files;
extern const std::array<u64, 8> ranks;
extern const std::array<u64, 15> diagonals;
extern const std::array<u64, 15> antiDiagonals;

extern const u64 a1h8Diagonal;
extern const u64 h1a8Diagonal;
extern const u64 lightSquares;
extern const u64 darkSquares;

extern const u64 queenside;
extern const u64 kingside;

// Color
enum Color { WHITE, BLACK, NONE };
Color reverseColor(Color c);

// Piece and PieceType
enum Piece { wP, wB, wN, wR, wQ, wK, bP, bB, bN, bR, bQ, bK, EMPTY };
std::string pieceToString(Piece p);
extern const std::map<std::string, Piece> stringToPieceMap;
Color getPieceColor(Piece p);
enum PieceType { pawn, bishop, knight, rook, queen, king };
Piece getCP(Color c, PieceType pt);
const int colorIndexOffset = 6;

// MoveType
enum MoveType { QUIET, CAPTURE, KCASTLE, QCASTLE, PROMOTION, ENPASSANT, CAPTUREANDPROMOTION };

// Direction
enum Direction { NORTH = 8, NW = 7, NE = 9, WEST = -1, EAST = 1, SW = -9, SOUTH = -8, SE = -7 };

// File and Rank
enum File { A, B, C, D, E, F, G, H };
extern std::array<u64, 8> fileNeighbors;
void initNeighborMasks();

enum Rank { R1, R2, R3, R4, R5, R6, R7, R8 };
extern const std::vector<Rank> almostPromotion; // [WHITE] = R7, [BLACK] = R2
extern const std::vector<Rank> startingRank;      // [WHITE] = R2, [BLACK] = R7
extern const std::vector<Direction> pawnPushDirection; // [WHITE] = NORTH, [BLACK] = SOUTH

// Square
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

extern const std::map<std::string, Square> stringToSquareMap;
extern const std::map<Square, std::string> squareToStringMap;
Rank sqToRank(Square s);
File sqToFile(Square s);
u64 sqToDiag(Square s);
u64 sqToAntiDiag(Square s);
Square goDirection(Square s, Direction d);

// Lookup tables and initialization functions
extern std::array<u64, 64> kingAttacksSquareLookup;
void initializeKingAttacks();
extern std::array<u64, 64> knightAttacksSquareLookup;
void initializeKnightAttacks();
extern std::array<u64, 64> whitePawnAttacksSquareLookup;
extern std::array<u64, 64> blackPawnAttacksSquareLookup;
void initializePawnAttacks();
extern std::array<const u64*, 2> colorToPawnLookup;
extern std::array<const u64*, 2> colorToPawnLookupReverse;
extern const std::vector<Piece> colorToKingLookup;
extern std::array<u64, 64> sToBB;
void initializeSQLookup();

// Magic bitboard functions and lookup tables
u64 slidingAttacks(Square s, u64 b, u64 locs);
extern std::array<u64, 64> bishopMasks;
extern std::array<u64, 64> rookMasks;
extern std::array<std::array<u64, 512>, 64> bishopAttacks;
extern std::array<std::array<u64, 4096>, 64> rookAttacks;
extern std::array<u64, 64> rookMagics;
extern std::array<u64, 64> bishopMagics;
extern std::array<int, 64> rookShifts;
extern std::array<int, 64> bishopShifts;
u64 slidingBishopAttacksForInitialization(Square s, u64 b);
void initBishopAttacks();
u64 slidingRookAttacksForInitialization(Square s, u64 b);
void initRookAttacks();

// Other lookup tables
extern std::array<std::array<u64, 64>, 64> squaresBetween;
void initSquaresBetween();
extern std::array<std::array<u64, 64>, 64> line;
void initLine();

// Zobrist keys
extern std::array<std::array<u64, 64>, 13> zobristTable;
extern u64 turnHash;
void initZobrist();

// Helper template functions for testing
template<typename T>
bool contains(const std::vector<T>& vec, const T& element);

bool checkSameElements(const std::vector<std::string>& a, const std::vector<std::string>& b);

} // namespace Chess

#endif // CONSTANTS_H
