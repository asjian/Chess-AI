#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"
#include "Move.h"
#include <vector>
#include <utility>
#include <chrono>
#include <cstdint>

namespace Chess {

enum Bound { UPPER, LOWER, EXACT };

int quiescenceSearch(ChessBoard* board, int depthLimit, int alpha, int beta, Color col);
void orderPVMoves(ChessBoard* board, std::vector<Move>& moves, const Move& pvMove, Color col, int depth, int rd);
std::pair<int, bool> principalVariationSearch(ChessBoard* board, int depth, int rd, int alpha, int beta, Color col, bool doNull, std::vector<Move>& pvLine, int64_t timeRemaining, std::chrono::steady_clock::time_point startTime);
Move searchWithTime(ChessBoard* board, int64_t moveTime);

// The following functions are assumed to exist in the transposition table module.
bool probeTT(ChessBoard* board, int* bestScore, int* alpha, int* beta, int depth, int rd, Move* bestMove);
void storeEntry(ChessBoard* board, int score, Bound flag, const Move& bestMove, int depth);
void clearTTable();

} // namespace Chess

#endif // SEARCH_H
