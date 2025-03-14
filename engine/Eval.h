#ifndef EVALUATION_H
#define EVALUATION_H

#include "Board.h"
#include <utility>

namespace Chess {

int evaluatePosition(ChessBoard* board);
std::pair<int, int> totalMaterialAndPieces(ChessBoard* board);
void evaluatePawns(ChessBoard* board, int* score);
void evaluateKnights(ChessBoard* board, int* score);
void evaluateBishops(ChessBoard* board, int* score);
void evaluateRooks(ChessBoard* board, int* score);
void evaluateQueens(ChessBoard* board, int* score);
void evaluateKings(ChessBoard* board, int* score, int totalPieces);
} 

#endif 
