#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <vector>
#include "Board.h"
#include "Move.h"

namespace Chess {

class MoveGenerator {
public:
    static int countPins(ChessBoard* board, Color opp, uint64_t occ, Square kingPos);
    static uint64_t allAttacks(ChessBoard* board, Color opp, uint64_t occ, uint64_t ortho, uint64_t diag);
    static uint64_t allPawnAttacks(uint64_t pawns, Color c);
    static uint64_t kingAttacks(Square sq);
    static uint64_t knightAttacks(Square sq);
    static uint64_t getBishopAttacks(Square sq, uint64_t blockers);
    static uint64_t getRookAttacks(Square sq, uint64_t blockers);
    static void genMovesFromLocations(ChessBoard* board, std::vector<Move>& moves, Square origin, uint64_t locs, Color c);
    static void getKingMoves(ChessBoard* board, std::vector<Move>& moves, Square kingSq, uint64_t attacked, uint64_t own, Color c);
    static void getKnightMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t knights, uint64_t pinned, uint64_t own, Color c, uint64_t allowed);
    static void getPawnMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t pawns, uint64_t pinned, uint64_t occ, uint64_t opp, Color c, uint64_t allowed);
    static void getBishopMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t bishops, uint64_t pinned, uint64_t own, uint64_t opp, Color c, uint64_t allowed);
    static void getRookMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t rooks, uint64_t pinned, uint64_t own, uint64_t opp, Color c, uint64_t allowed);
    static void getQueenMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t queens, uint64_t pinned, uint64_t own, uint64_t opp, Color c, uint64_t allowed);
    static void getCastlingMoves(ChessBoard* board, std::vector<Move>& moves, Square kingSq, uint64_t attacked, Color c);
    static std::vector<Move> generateLegalMoves(ChessBoard* board);
    static std::vector<Move> generateCaptures(ChessBoard* board);
};

} // namespace Chess

#endif // MOVEGEN_H
