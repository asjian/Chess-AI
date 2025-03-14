#include "Search.h"
#include "Evaluation.h"
#include "MoveGen.h"
#include "Constants.h"
#include "Bitboard.h"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <array>

namespace Chess {

struct MoveBonus {
    Move mv;
    int bonus;
};

static const int NULL_MOVE_RED = 3;
static int nodesExamined = 0;
static std::array<std::array<Move, 2>, 100> killerMoves;
static std::array<std::array<std::array<int, 64>, 64>, 100> historyHeuristic;

int quiescenceSearch(ChessBoard* board, int limit, int alpha, int beta, Color col) {
    nodesExamined++;
    int evalScore = evaluatePosition(board) * FACTOR[col];
    if (evalScore >= beta)
        return beta;
    int delta = QUEEN_VALUE;
    if (evalScore < alpha - delta)
        return alpha;
    if (alpha < evalScore)
        alpha = evalScore;
    if (limit == 0)
        return evalScore;
    std::vector<Move> captureMoves = MoveGenerator::generateCaptures(board);
    for (const Move& mv : captureMoves) {
        if (mv.moveType == CAPTURE || mv.moveType == CAPTUREANDPROMOTION || mv.moveType == ENPASSANT) {
            board->makeMove(mv);
            int score = -quiescenceSearch(board, limit - 1, -beta, -alpha, reverseColor(col));
            board->undo();
            if (score >= beta)
                return beta;
            if (score > alpha)
                alpha = score;
        }
    }
    return alpha;
}

void orderPVMoves(ChessBoard* board, std::vector<Move>& moves, const Move& pvMove, Color col, int depth, int rd) {
    std::vector<MoveBonus> bonuses;
    for (size_t i = 0; i < moves.size(); i++) {
        if (moves[i] == pvMove) {
            std::swap(moves[i], moves[0]);
            bonuses.push_back({ moves[i], 30000 });
        } else {
            int bonusVal = 0;
            if (moves[i].moveType == CAPTUREANDPROMOTION) {
                bonusVal = MATERIAL.at(moves[i].promotionPiece) - MATERIAL.at(moves[i].capturedPiece) - MATERIAL.at(moves[i].mPiece);
                bonusVal *= FACTOR[col];
            } else if (moves[i].moveType == CAPTURE) {
                bonusVal = -MATERIAL.at(moves[i].capturedPiece) - MATERIAL.at(moves[i].mPiece);
                bonusVal *= FACTOR[col];
            } else if (moves[i].moveType == PROMOTION) {
                bonusVal = MATERIAL.at(moves[i].promotionPiece) - MATERIAL.at(moves[i].mPiece);
                bonusVal *= FACTOR[col];
            } else {
                bonusVal = 0;
                if (moves[i].toUCI() == killerMoves[depth][0].toUCI())
                    bonusVal = 150;
                else if (moves[i].toUCI() == killerMoves[depth][1].toUCI())
                    bonusVal = 140;
                if (depth >= rd) {
                    switch (moves[i].mPiece) {
                        case wN: bonusVal += knightSquareTable[moves[i].dest] - knightSquareTable[moves[i].src]; break;
                        case bN: bonusVal += knightSquareTable[REVERSE_PSQ[moves[i].dest]] - knightSquareTable[REVERSE_PSQ[moves[i].src]]; break;
                        case wB: bonusVal += bishopSquareTable[moves[i].dest] - bishopSquareTable[moves[i].src]; break;
                        case bB: bonusVal += bishopSquareTable[REVERSE_PSQ[moves[i].dest]] - bishopSquareTable[REVERSE_PSQ[moves[i].src]]; break;
                        case wR: bonusVal += rookSquareTable[moves[i].dest] - rookSquareTable[moves[i].src]; break;
                        case bR: bonusVal += rookSquareTable[REVERSE_PSQ[moves[i].dest]] - rookSquareTable[REVERSE_PSQ[moves[i].src]]; break;
                        case wQ: bonusVal += queenSquareTable[moves[i].dest] - queenSquareTable[moves[i].src]; break;
                        case bQ: bonusVal += queenSquareTable[REVERSE_PSQ[moves[i].dest]] - queenSquareTable[REVERSE_PSQ[moves[i].src]]; break;
                        case wP: bonusVal += pawnSquareTable[moves[i].dest] - pawnSquareTable[moves[i].src]; break;
                        case bP: bonusVal += pawnSquareTable[REVERSE_PSQ[moves[i].dest]] - pawnSquareTable[REVERSE_PSQ[moves[i].src]]; break;
                        default: break;
                    }
                }
            }
            bonuses.push_back({ moves[i], bonusVal });
        }
    }
    std::sort(bonuses.begin(), bonuses.end(), [](const MoveBonus& a, const MoveBonus& b) {
        return a.bonus > b.bonus;
    });
    for (size_t i = 0; i < bonuses.size(); i++) {
        moves[i] = bonuses[i].mv;
    }
}

std::pair<int, bool> principalVariationSearch(ChessBoard* board, int depth, int rd, int alpha, int beta, Color col, bool doNull, std::vector<Move>& pvLine, int64_t tRem, std::chrono::steady_clock::time_point startTime) {
    nodesExamined++;
    std::vector<Move> localPV;
    if (depth <= 0) {
        return { quiescenceSearch(board, 4, alpha, beta, col), false };
    }
    std::vector<Move> legalMoves = board->generateLegalMoves();
    if (legalMoves.empty()) {
        int sc = evaluatePosition(board) * FACTOR[col];
        return { sc, false };
    }
    if (board->isThreeFoldRep()) {
        return { 0, false };
    }
    int bestScore = 0;
    bool timeOut = false;
    Move bestMove;
    int origAlpha = alpha;
    if (probeTT(board, &bestScore, &alpha, &beta, depth, rd, &bestMove)) {
        pvLine.clear();
        pvLine.push_back(bestMove);
        pvLine.insert(pvLine.end(), localPV.begin(), localPV.end());
        return { bestScore, false };
    }
    if (depth > 1) {
        orderPVMoves(board, legalMoves, bestMove, col, depth, rd);
    }
    bool inCheck = board->isCheck(col);
    if (inCheck)
        depth++;
    uint64_t nonPawn = board->colorBitboards[col] ^ board->getPiecesByColor(pawn, col);
    if (doNull && !inCheck && nonPawn != 0 && board->plyCnt > 0) {
        board->makeNullMove();
        auto result = principalVariationSearch(board, depth - NULL_MOVE_RED - 1, rd, -beta, -beta + 1, reverseColor(col), false, localPV, tRem, std::chrono::steady_clock::now());
        int score = -result.first;
        board->undoNullMove();
        if (score >= beta)
            return { beta, result.second };
        if (score > alpha)
            alpha = score;
    }
    for (size_t i = 0; i < legalMoves.size(); i++) {
        if (timeOut)
            break;
        int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
        if (elapsed > tRem) {
            pvLine.clear();
            return { bestScore, true };
        }
        board->makeMove(legalMoves[i]);
        if (i == 0) {
            auto result = principalVariationSearch(board, depth - 1, rd, -beta, -alpha, reverseColor(col), true, localPV, tRem, std::chrono::steady_clock::now());
            bestScore = -result.first;
            timeOut = result.second;
            board->undo();
            if (bestScore > alpha && !timeOut) {
                bestMove = legalMoves[i];
                pvLine.clear();
                pvLine.push_back(legalMoves[i]);
                pvLine.insert(pvLine.end(), localPV.begin(), localPV.end());
                if (bestScore >= beta) {
                    if (killerMoves[depth][0].toUCI() != legalMoves[i].toUCI()) {
                        killerMoves[depth][1] = killerMoves[depth][0];
                        killerMoves[depth][0] = legalMoves[i];
                    }
                    historyHeuristic[depth][legalMoves[i].src][legalMoves[i].dest]++;
                    break;
                }
                alpha = bestScore;
            }
        } else {
            int score = alpha + 1;
            bool chk = board->isCheck(board->sideToMove);
            if (i >= 4 && depth >= 3 && legalMoves[i].moveType != CAPTURE && legalMoves[i].moveType != CAPTUREANDPROMOTION && !chk) {
                auto result = principalVariationSearch(board, depth - 2, rd, -alpha - 1, -alpha, reverseColor(col), true, localPV, tRem, std::chrono::steady_clock::now());
                score = -result.first;
            }
            if (score > alpha) {
                auto result = principalVariationSearch(board, depth - 1, rd, -alpha - 1, -alpha, reverseColor(col), true, localPV, tRem, std::chrono::steady_clock::now());
                score = -result.first;
                if (timeOut)
                    break;
                if (score > alpha && score < beta) {
                    auto result2 = principalVariationSearch(board, depth - 1, rd, -beta, -alpha, reverseColor(col), true, localPV, tRem, std::chrono::steady_clock::now());
                    score = -result2.first;
                }
                if (score > alpha && !timeOut) {
                    bestMove = legalMoves[i];
                    alpha = score;
                    pvLine.clear();
                    pvLine.push_back(legalMoves[i]);
                    pvLine.insert(pvLine.end(), localPV.begin(), localPV.end());
                }
                board->undo();
                if (score > bestScore && !timeOut) {
                    bestScore = score;
                    if (score >= beta) {
                        if (killerMoves[depth][0].toUCI() != legalMoves[i].toUCI()) {
                            killerMoves[depth][1] = killerMoves[depth][0];
                            killerMoves[depth][0] = legalMoves[i];
                        }
                        historyHeuristic[depth][legalMoves[i].src][legalMoves[i].dest]++;
                        break;
                    }
                }
            } else {
                board->undo();
            }
        }
    }
    if (!timeOut) {
        Bound flag;
        if (bestScore <= origAlpha)
            flag = UPPER;
        else if (bestScore >= beta)
            flag = LOWER;
        else
            flag = EXACT;
        storeEntry(board, bestScore, flag, bestMove, depth);
    }
    return { bestScore, timeOut };
}

Move searchWithTime(ChessBoard* board, int64_t moveTime) {
    board->printFromBitboards();
    auto startTime = std::chrono::steady_clock::now();
    std::vector<Move> pvLine;
    std::vector<Move> legalMoves = board->generateLegalMoves();
    Move prevBest;
    if (legalMoves.size() == 1)
        return legalMoves[0];
    for (int d = 1; d <= 100; d++) {
        nodesExamined = 0;
        int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
        int64_t timeLeft = moveTime - elapsed;
        if (moveTime > timeLeft * 2)
            break;
        auto searchStart = std::chrono::steady_clock::now();
        auto result = principalVariationSearch(board, d, d, -WIN_VALUE - 1, WIN_VALUE + 1, board->sideToMove, true, pvLine, timeLeft, std::chrono::steady_clock::now());
        int score = result.first;
        bool timeout = result.second;
        int64_t timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - searchStart).count();
        if (timeout)
            return prevBest;
        board->makeMove(pvLine[0]);
        if (board->isTwoFold() && score > 0) {
            clearTTable();
            board->undo();
            std::cout << "Two-fold repetition encountered, removing TT entry\n";
            clearTTable();
        } else {
            board->undo();
        }
        std::string pvStr;
        for (const Move& mv : pvLine) {
            pvStr += " " + mv.toUCI();
        }
        int signedScore = score * FACTOR[board->sideToMove];
        std::cout << "info depth " << d << " nodes " << nodesExamined << " time " << timeTaken << " score cp " << signedScore << " pv" << pvStr << "\n";
        if (score == WIN_VALUE || score == -WIN_VALUE) {
            clearTTable();
            return pvLine[0];
        }
        prevBest = pvLine[0];
    }
    return prevBest;
}

} // namespace Chess
