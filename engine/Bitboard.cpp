#include "Bitboard.h"
#include <cstdint>
#include <iostream>
#include <array>

namespace Chess {

const std::array<Bitboard, 8> fileMasks = {
    0x0101010101010101ULL,
    0x0202020202020202ULL,
    0x0404040404040404ULL,
    0x0808080808080808ULL,
    0x1010101010101010ULL,
    0x2020202020202020ULL,
    0x4040404040404040ULL,
    0x8080808080808080ULL
};

static const int deBruijnLookup[64] = {
    0, 47, 1, 56, 48, 27, 2, 60,
    57, 49, 41, 37, 28, 16, 3, 61,
    54, 58, 35, 52, 50, 42, 21, 44,
    38, 32, 29, 23, 17, 11, 4, 62,
    46, 55, 26, 59, 40, 36, 15, 53,
    34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30, 9, 24,
    13, 18, 8, 12, 7, 6, 5, 63
};

void displayBitboard(Bitboard bb) {
    std::string output = "\n";
    for (int i = 56; i >= 0; i -= 8) {
        for (int j = 0; j < 8; j++)
            output += (bb & (Bitboard(1) << (i + j))) ? "1 " : "0 ";
        output += "\n";
    }
    std::cout << output;
}

int countBits(Bitboard bb) {
    int cnt = 0;
    while (bb) {
        cnt++;
        bb &= bb - 1;
    }
    return cnt;
}

int scanForward(Bitboard bb) {
    const Bitboard deBruijn = 0x03f79d71b4cb0a89ULL;
    return deBruijnLookup[((bb ^ (bb - 1)) * deBruijn) >> 58];
}

int popLeastSignificantBit(Bitboard* bb) {
    int pos = scanForward(*bb);
    *bb &= *bb - 1;
    return pos;
}

Bitboard verticalFlip(Bitboard x) {
    const Bitboard mask1 = 0x00FF00FF00FF00FFULL;
    const Bitboard mask2 = 0x0000FFFF0000FFFFULL;
    x = ((x >> 8) & mask1) | ((x & mask1) << 8);
    x = ((x >> 16) & mask2) | ((x & mask2) << 16);
    x = (x >> 32) | (x << 32);
    return x;
}

Bitboard horizontalMirror(Bitboard x) {
    const Bitboard mask1 = 0x5555555555555555ULL;
    const Bitboard mask2 = 0x3333333333333333ULL;
    const Bitboard mask4 = 0x0f0f0f0f0f0f0f0fULL;
    x = ((x >> 1) & mask1) + 2 * (x & mask1);
    x = ((x >> 2) & mask2) + 4 * (x & mask2);
    x = ((x >> 4) & mask4) + 16 * (x & mask4);
    return x;
}

Bitboard diagonalFlipA1H8(Bitboard x) {
    Bitboard temp;
    const Bitboard mask1 = 0x5500550055005500ULL;
    const Bitboard mask2 = 0x3333000033330000ULL;
    const Bitboard mask4 = 0x0f0f0f0f00000000ULL;
    temp = mask4 & (x ^ (x << 28));
    x ^= temp ^ (temp >> 28);
    temp = mask2 & (x ^ (x << 14));
    x ^= temp ^ (temp >> 14);
    temp = mask1 & (x ^ (x << 7));
    x ^= temp ^ (temp >> 7);
    return x;
}

Bitboard diagonalFlipA8H1(Bitboard x) {
    Bitboard temp;
    const Bitboard mask1 = 0xaa00aa00aa00aa00ULL;
    const Bitboard mask2 = 0xcccc0000cccc0000ULL;
    const Bitboard mask4 = 0xf0f0f0f00f0f0f0fULL;
    temp = x ^ (x << 36);
    x ^= mask4 & (temp ^ (x >> 36));
    temp = mask2 & (x ^ (x << 18));
    x ^= temp ^ (temp >> 18);
    temp = mask1 & (x ^ (x << 9));
    x ^= temp ^ (temp >> 9);
    return x;
}

Bitboard rotateClockwise90(Bitboard x) {
    return verticalFlip(diagonalFlipA1H8(x));
}

Bitboard rotateClockwise180(Bitboard x) {
    return horizontalMirror(verticalFlip(x));
}

Bitboard rotateClockwise270(Bitboard x) {
    return diagonalFlipA1H8(verticalFlip(x));
}

Bitboard rotateLeft(Bitboard x, int n) {
    n = n % 64;
    return (x << n) | (x >> (64 - n));
}

Bitboard rotateRight(Bitboard x, int n) {
    n = n % 64;
    return (x >> n) | (x << (64 - n));
}

Bitboard shiftBitboardDirection(Bitboard x, ShiftDirection d) {
    switch (d) {
        case NORTH_SHIFT:
            return x << NORTH_SHIFT;
        case SOUTH_SHIFT:
            return x >> NORTH_SHIFT;
        case 2 * NORTH_SHIFT:
            return x << (2 * NORTH_SHIFT);
        case 2 * SOUTH_SHIFT:
            return x >> (2 * NORTH_SHIFT);
        case EAST_SHIFT:
            return (x & ~fileMasks[FILE_H]) << EAST_SHIFT;
        case WEST_SHIFT:
            return (x & ~fileMasks[FILE_A]) >> EAST_SHIFT;
        case NE_SHIFT:
            return (x & ~fileMasks[FILE_H]) << NE_SHIFT;
        case SE_SHIFT:
            return (x & ~fileMasks[FILE_H]) >> (-SE_SHIFT);
        case NW_SHIFT:
            return (x & ~fileMasks[FILE_A]) << NW_SHIFT;
        case SW_SHIFT:
            return (x & ~fileMasks[FILE_A]) >> (-SW_SHIFT);
        default:
            return 0;
    }
}

} // namespace Chess
