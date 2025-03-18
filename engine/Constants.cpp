#include "Constants.h"
#include "Bitboard.h"
#include <random>
#include <algorithm>
#include <functional>

namespace Chess {

const std::array<u64, 8> files = {
    0x101010101010101ULL,
    0x202020202020202ULL,
    0x404040404040404ULL,
    0x808080808080808ULL,
    0x1010101010101010ULL,
    0x2020202020202020ULL,
    0x4040404040404040ULL,
    0x8080808080808080ULL
};

const std::array<u64, 8> ranks = {
    0xffULL, 0xff00ULL, 0xff0000ULL, 0xff000000ULL,
    0xff00000000ULL, 0xff0000000000ULL, 0xff000000000000ULL, 0xff00000000000000ULL
};

const std::array<u64, 15> diagonals = {
    0x80ULL, 0x8040ULL, 0x804020ULL,
    0x80402010ULL, 0x8040201008ULL, 0x804020100804ULL,
    0x80402010080402ULL, 0x8040201008040201ULL, 0x4020100804020100ULL,
    0x2010080402010000ULL, 0x1008040201000000ULL, 0x804020100000000ULL,
    0x402010000000000ULL, 0x201000000000000ULL, 0x100000000000000ULL
};

const std::array<u64, 15> antiDiagonals = {
    0x1ULL, 0x102ULL, 0x10204ULL,
    0x1020408ULL, 0x102040810ULL, 0x10204081020ULL,
    0x1020408102040ULL, 0x102040810204080ULL, 0x204081020408000ULL,
    0x408102040800000ULL, 0x810204080000000ULL, 0x1020408000000000ULL,
    0x2040800000000000ULL, 0x4080000000000000ULL, 0x8000000000000000ULL
};

const u64 a1h8Diagonal = 0x8040201008040201ULL;
const u64 h1a8Diagonal = 0x0102040810204080ULL;
const u64 lightSquares = 0x55AA55AA55AA55AAULL;
const u64 darkSquares = 0xAA55AA55AA55AA55ULL;

const u64 queenside = 0xf0f0f0f0f0f0f0fULL;
const u64 kingside  = 0xf0f0f0f0f0f0f0f0ULL;

Color reverseColor(Color c) {
    return static_cast<Color>(c ^ 1);
}

std::string pieceToString(Piece p) {
    switch (p) {
        case wP: return "P";
        case wB: return "B";
        case wN: return "N";
        case wR: return "R";
        case wQ: return "Q";
        case wK: return "K";
        case bP: return "p";
        case bB: return "b";
        case bN: return "n";
        case bR: return "r";
        case bQ: return "q";
        case bK: return "k";
        case EMPTY: return ".";
    }
    return ".";
}

const std::map<std::string, Piece> stringToPieceMap = {
    {"P", wP}, {"B", wB}, {"N", wN}, {"R", wR}, {"Q", wQ}, {"K", wK},
    {"p", bP}, {"b", bB}, {"n", bN}, {"r", bR}, {"q", bQ}, {"k", bK}
};

Color getPieceColor(Piece p) {
    if (p == EMPTY)
        return NONE;
    if (p == wP || p == wN || p == wB || p == wR || p == wQ || p == wK)
        return WHITE;
    return BLACK;
}

Piece getCP(Color c, PieceType pt) {
    return static_cast<Piece>(static_cast<int>(pt) + colorIndexOffset * static_cast<int>(c));
}

std::array<u64, 8> fileNeighbors;
void initNeighborMasks() {
    for (int f = A; f <= H; f++) {
        if (f == A)
            fileNeighbors[f] = files[B];
        else if (f == H)
            fileNeighbors[f] = files[G];
        else
            fileNeighbors[f] = files[f - 1] | files[f + 1];
    }
}

const std::vector<Rank> almostPromotion = { R7, R2 };
const std::vector<Rank> startingRank = { R2, R7 };
const std::vector<Direction> pawnPushDirection = { NORTH, SOUTH };

const std::map<std::string, Square> stringToSquareMap = {
    {"a1", a1}, {"a2", a2}, {"a3", a3}, {"a4", a4}, {"a5", a5}, {"a6", a6}, {"a7", a7}, {"a8", a8},
    {"b1", b1}, {"b2", b2}, {"b3", b3}, {"b4", b4}, {"b5", b5}, {"b6", b6}, {"b7", b7}, {"b8", b8},
    {"c1", c1}, {"c2", c2}, {"c3", c3}, {"c4", c4}, {"c5", c5}, {"c6", c6}, {"c7", c7}, {"c8", c8},
    {"d1", d1}, {"d2", d2}, {"d3", d3}, {"d4", d4}, {"d5", d5}, {"d6", d6}, {"d7", d7}, {"d8", d8},
    {"e1", e1}, {"e2", e2}, {"e3", e3}, {"e4", e4}, {"e5", e5}, {"e6", e6}, {"e7", e7}, {"e8", e8},
    {"f1", f1}, {"f2", f2}, {"f3", f3}, {"f4", f4}, {"f5", f5}, {"f6", f6}, {"f7", f7}, {"f8", f8},
    {"g1", g1}, {"g2", g2}, {"g3", g3}, {"g4", g4}, {"g5", g5}, {"g6", g6}, {"g7", g7}, {"g8", g8},
    {"h1", h1}, {"h2", h2}, {"h3", h3}, {"h4", h4}, {"h5", h5}, {"h6", h6}, {"h7", h7}, {"h8", h8}
};

std::map<Square, std::string> reversedMap(const std::map<std::string, Square>& m) {
    std::map<Square, std::string> n;
    for (const auto& p : m) {
        n[p.second] = p.first;
    }
    return n;
}

const std::map<Square, std::string> squareToStringMap = reversedMap(stringToSquareMap);

Rank sqToRank(Square s) {
    return static_cast<Rank>(s >> 3);
}

File sqToFile(Square s) {
    return static_cast<File>(s & 0b111);
}

u64 sqToDiag(Square s) {
    return diagonals[static_cast<int>(sqToRank(s)) - static_cast<int>(sqToFile(s)) + 7];
}

u64 sqToAntiDiag(Square s) {
    return antiDiagonals[static_cast<int>(sqToRank(s)) + static_cast<int>(sqToFile(s))];
}

Square goDirection(Square s, Direction d) {
    return static_cast<Square>(static_cast<int>(s) + static_cast<int>(d));
}

std::array<u64, 64> kingAttacksSquareLookup;
void initializeKingAttacks() {
    u64 bb = 1;
    for (int i = 0; i < 64; i++) {
        u64 copy = bb;
        u64 atk = shiftBitboard(copy, EAST) | shiftBitboard(copy, WEST);
        copy |= atk;
        atk |= shiftBitboard(copy, NORTH) | shiftBitboard(copy, SOUTH);
        kingAttacksSquareLookup[i] = atk;
        bb <<= 1;
    }
}

std::array<u64, 64> knightAttacksSquareLookup;
void initializeKnightAttacks() {
    u64 bb = 1;
    for (int i = 0; i < 64; i++) {
        u64 atk = 0;
        atk |= (bb << 17) & ~files[A];
        atk |= (bb << 10) & ~files[A] & ~files[B];
        atk |= (bb >> 6)  & ~files[A] & ~files[B];
        atk |= (bb >> 15) & ~files[A];
        atk |= (bb << 15) & ~files[H];
        atk |= (bb << 6)  & ~files[G] & ~files[H];
        atk |= (bb >> 10) & ~files[G] & ~files[H];
        atk |= (bb >> 17) & ~files[H];
        knightAttacksSquareLookup[i] = atk;
        bb <<= 1;
    }
}

std::array<u64, 64> whitePawnAttacksSquareLookup;
std::array<u64, 64> blackPawnAttacksSquareLookup;
void initializePawnAttacks() {
    u64 bb = 1;
    for (int i = 0; i < 64; i++) {
        u64 atk = shiftBitboard(bb, NE) | shiftBitboard(bb, NW);
        whitePawnAttacksSquareLookup[i] = atk;
        atk = shiftBitboard(bb, SE) | shiftBitboard(bb, SW);
        blackPawnAttacksSquareLookup[i] = atk;
        bb <<= 1;
    }
}

std::array<const u64*, 2> colorToPawnLookup = { &whitePawnAttacksSquareLookup, &blackPawnAttacksSquareLookup };
std::array<const u64*, 2> colorToPawnLookupReverse = { &blackPawnAttacksSquareLookup, &whitePawnAttacksSquareLookup };

std::vector<Piece> colorToKingLookup = { wK, bK };

std::array<u64, 64> sToBB;
void initializeSQLookup() {
    for (int i = 0; i < 64; i++) {
        sToBB[i] = 1ULL << i;
    }
}

u64 slidingAttacks(Square s, u64 b, u64 locs) {
    u64 a = (locs & b) - (sToBB[s] << 1);
    auto reverseBits = [](u64 val) -> u64 {
        u64 rev = 0;
        for (int i = 0; i < 64; i++) {
            rev <<= 1;
            rev |= (val & 1);
            val >>= 1;
        }
        return rev;
    };
    u64 c = reverseBits(b & locs) - (reverseBits(sToBB[s]) << 1);
    u64 d = reverseBits(c);
    return (a ^ d) & locs;
}

std::array<u64, 64> bishopMasks;
std::array<u64, 64> rookMasks;

std::array<std::array<u64, 512>, 64> bishopAttacks;
std::array<std::array<u64, 4096>, 64> rookAttacks;

std::array<u64, 64> rookMagics = {
    0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL,
    0x280028004000800ULL, 0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL,
    0xa000800080400034ULL, 0x4808020004000ULL, 0x2290802004801000ULL, 0x411000d00100020ULL,
    0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL, 0x1002100004082ULL,
    0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
    0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL,
    0x800080204005ULL, 0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL,
    0x12040280080080ULL, 0x100040080020080ULL, 0x9020010080800200ULL, 0x813241200148449ULL,
    0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL, 0x400402202000812ULL,
    0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
    0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL,
    0x804040008008080ULL, 0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL,
    0x280009023410300ULL, 0xe0100040002240ULL, 0x200100401700ULL, 0x2244100408008080ULL,
    0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL, 0x2000009044210200ULL,
    0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
    0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL,
};

std::array<u64, 64> bishopMagics = {
    0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL,
    0x4042004000000ULL, 0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL,
    0x400492088408100ULL, 0x201c401040c0084ULL, 0x840800910a0010ULL, 0x82080240060ULL,
    0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL, 0x8144042209100900ULL,
    0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
    0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL,
    0x520040470104290ULL, 0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL,
    0x941408200c002000ULL, 0x658810000806011ULL, 0x188071040440a00ULL, 0x4800404002011c00ULL,
    0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL, 0x80c0040400080120ULL,
    0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
    0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL,
    0x3308082008200100ULL, 0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL,
    0x111040120082000ULL, 0x6803040141280a00ULL, 0x2101004202410000ULL, 0x8200000041108022ULL,
    0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL, 0x822088220820214ULL,
    0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
    0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL,
};

std::array<int, 64> rookShifts;
std::array<int, 64> bishopShifts;

u64 slidingBishopAttacksForInitialization(Square s, u64 b) {
    u64 atk = 0;
    atk |= slidingAttacks(s, b, sqToAntiDiag(s));
    atk |= slidingAttacks(s, b, sqToDiag(s));
    return atk;
}

void initBishopAttacks() {
    for (int s = a1; s <= h8; s++) {
        u64 edgeMask = (files[A] | files[H]) & ~files[sqToFile(static_cast<Square>(s))];
        edgeMask |= (ranks[R1] | ranks[R8]) & ~ranks[sqToRank(static_cast<Square>(s))];
        bishopMasks[s] = (sqToAntiDiag(static_cast<Square>(s)) ^ sqToDiag(static_cast<Square>(s))) & ~edgeMask;
        bishopShifts[s] = 64 - popCount(bishopMasks[s]);
        u64 i = 0;
        while (true) {
            u64 j = i;
            j *= bishopMagics[s];
            j >>= bishopShifts[s];
            bishopAttacks[s][j] = slidingBishopAttacksForInitialization(static_cast<Square>(s), i);
            i = (i - bishopMasks[s]) & bishopMasks[s];
            if (i == 0)
                break;
        }
    }
}

u64 slidingRookAttacksForInitialization(Square s, u64 b) {
    u64 atk = 0;
    atk |= slidingAttacks(s, b, files[sqToFile(static_cast<Square>(s))]);
    atk |= slidingAttacks(s, b, ranks[sqToRank(static_cast<Square>(s))]);
    return atk;
}

void initRookAttacks() {
    for (int s = a1; s <= h8; s++) {
        u64 edgeMask = (files[A] | files[H]) & ~files[sqToFile(static_cast<Square>(s))];
        edgeMask |= (ranks[R1] | ranks[R8]) & ~ranks[sqToRank(static_cast<Square>(s))];
        rookMasks[s] = (files[sqToFile(static_cast<Square>(s))] ^ ranks[sqToRank(static_cast<Square>(s))]) & ~edgeMask;
        rookShifts[s] = 64 - popCount(rookMasks[s]);
        u64 i = 0;
        while (true) {
            u64 j = i;
            j *= rookMagics[s];
            j >>= rookShifts[s];
            rookAttacks[s][j] = slidingRookAttacksForInitialization(static_cast<Square>(s), i);
            i = (i - rookMasks[s]) & rookMasks[s];
            if (i == 0)
                break;
        }
    }
}

std::array<std::array<u64, 64>, 64> squaresBetween;
void initSquaresBetween() {
    u64 sq;
    for (int i = a1; i <= h8; i++) {
        for (int j = a1; j <= h8; j++) {
            sq = (1ULL << i) | (1ULL << j);
            if (sqToFile(static_cast<Square>(i)) == sqToFile(static_cast<Square>(j)) ||
                sqToRank(static_cast<Square>(i)) == sqToRank(static_cast<Square>(j)))
                squaresBetween[i][j] = slidingRookAttacksForInitialization(static_cast<Square>(i), sq) & slidingRookAttacksForInitialization(static_cast<Square>(j), sq);
            else if (sqToDiag(static_cast<Square>(i)) == sqToDiag(static_cast<Square>(j)) ||
                     sqToAntiDiag(static_cast<Square>(i)) == sqToAntiDiag(static_cast<Square>(j)))
                squaresBetween[i][j] = slidingBishopAttacksForInitialization(static_cast<Square>(i), sq) & slidingBishopAttacksForInitialization(static_cast<Square>(j), sq);
        }
    }
}

std::array<std::array<u64, 64>, 64> line;
void initLine() {
    for (int i = a1; i <= h8; i++) {
        for (int j = a1; j <= h8; j++) {
            if (sqToFile(static_cast<Square>(i)) == sqToFile(static_cast<Square>(j)) ||
                sqToRank(static_cast<Square>(i)) == sqToRank(static_cast<Square>(j)))
                line[i][j] = (slidingRookAttacksForInitialization(static_cast<Square>(i), 0) & slidingRookAttacksForInitialization(static_cast<Square>(j), 0)) | (1ULL << i) | (1ULL << j);
            else if (sqToDiag(static_cast<Square>(i)) == sqToDiag(static_cast<Square>(j)) ||
                     sqToAntiDiag(static_cast<Square>(i)) == sqToAntiDiag(static_cast<Square>(j)))
                line[i][j] = (slidingBishopAttacksForInitialization(static_cast<Square>(i), 0) & slidingBishopAttacksForInitialization(static_cast<Square>(j), 0)) | (1ULL << i) | (1ULL << j);
        }
    }
}

std::array<std::array<u64, 64>, 13> zobristTable;
u64 turnHash;
void initZobrist() {
    std::mt19937_64 rng(42);
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 64; j++) {
            zobristTable[i][j] = rng();
        }
    }
    turnHash = rng();
}

template<typename T>
bool contains(const std::vector<T>& vec, const T& element) {
    for (const auto& a : vec) {
        if (a == element)
            return true;
    }
    return false;
}
template bool contains<int>(const std::vector<int>&, const int&);
template bool contains<std::string>(const std::vector<std::string>&, const std::string&);

bool checkSameElements(const std::vector<std::string>& a, const std::vector<std::string>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    std::vector<std::string> a_copy = a;
    std::vector<std::string> b_copy = b;
    std::sort(a_copy.begin(), a_copy.end());
    std::sort(b_copy.begin(), b_copy.end());
    return a_copy == b_copy;
}

} // namespace Chess
