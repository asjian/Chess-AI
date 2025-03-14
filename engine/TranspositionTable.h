#ifndef TTABLE_H
#define TTABLE_H

#include <cstdint>
#include <vector>
#include "Move.h"
#include "Board.h"

namespace Chess {

enum BoundType {
    UPPER_BOUND,
    LOWER_BOUND,
    EXACT_BOUND
};

struct TransEntry {
    uint64_t hashValue;
    Move bestMove;
    int score;
    int depth;
    BoundType boundType;
};

struct TransTable {
    std::vector<TransEntry> entries;
    uint64_t tableSize;
};

extern TransTable transTable;

void initTransTable(int megabytes);
void clearTransTable();
void storeTransEntry(ChessBoard* board, int score, BoundType bType, const Move& mv, int depth);
std::pair<bool, int> probeTransTable(ChessBoard* board, int* score, int* alpha, int* beta, int depth, int rd, Move* mv);

} // namespace Chess

#endif // TTABLE_H
