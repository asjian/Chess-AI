#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <array>
#include <string>
#include <iostream>

namespace Chess {

using Bitboard = uint64_t;

enum ShiftDirection {
    NORTH_SHIFT = 8,
    SOUTH_SHIFT = -8,
    EAST_SHIFT  = 1,
    WEST_SHIFT  = -1,
    NE_SHIFT    = 9,
    NW_SHIFT    = 7,
    SE_SHIFT    = -7,
    SW_SHIFT    = -9
};

enum FileIndex {
    FILE_A = 0,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H
};

extern const std::array<Bitboard, 8> fileMasks;

void displayBitboard(Bitboard bb);
int countBits(Bitboard bb);
int scanForward(Bitboard bb);
int popLeastSignificantBit(Bitboard* bb);
Bitboard verticalFlip(Bitboard x);
Bitboard horizontalMirror(Bitboard x);
Bitboard diagonalFlipA1H8(Bitboard x);
Bitboard diagonalFlipA8H1(Bitboard x);
Bitboard rotateClockwise90(Bitboard x);
Bitboard rotateClockwise180(Bitboard x);
Bitboard rotateClockwise270(Bitboard x);
Bitboard rotateLeft(Bitboard x, int n);
Bitboard rotateRight(Bitboard x, int n);
Bitboard shiftBitboardDirection(Bitboard x, ShiftDirection d);

} // namespace Chess

#endif // BITBOARD_H
