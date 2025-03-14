#include "TTable.h"

namespace Chess {

TransTable transTable;

void initTransTable(int mb) {
    transTable.tableSize = static_cast<uint64_t>(mb) * 1024 * 1024 / sizeof(TransEntry);
    transTable.entries.resize(transTable.tableSize);
}

void clearTransTable() {
    transTable.entries.assign(transTable.tableSize, TransEntry());
}

void storeTransEntry(ChessBoard* board, int scr, BoundType bType, const Move& mv, int depth) {
    uint64_t index = board->zobristHash % transTable.tableSize;
    if (!(mv.src == a1 && mv.dest == a1))
        transTable.entries[index] = { board->zobristHash, mv, scr, depth, bType };
}

std::pair<bool, int> probeTransTable(ChessBoard* board, int* scr, int* alpha, int* beta, int depth, int rd, Move* mv) {
    uint64_t index = board->zobristHash % transTable.tableSize;
    const TransEntry& entry = transTable.entries[index];
    if (entry.hashValue == board->zobristHash) {
        *mv = entry.bestMove;
        if (entry.depth > depth) {
            *scr = entry.score;
            switch (entry.boundType) {
                case UPPER_BOUND:
                    if (*scr < *beta && depth != rd) *beta = *scr;
                    break;
                case LOWER_BOUND:
                    if (*scr > *alpha && depth != rd) *alpha = *scr;
                    break;
                case EXACT_BOUND:
                    return { true, *scr };
            }
            if (*alpha >= *beta)
                return { true, *scr };
        }
    }
    return { false, 0 };
}

} // namespace Chess
