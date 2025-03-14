#include "Evaluation.h"
#include "Constants.h"
#include "Bitboard.h"
#include <tuple>
#include <vector>
#include <map>
#include <array>
#include <utility>

namespace Chess {

static const int WIN_VALUE = 1000000;
static const int PAWN_VALUE = 100;
static const int KNIGHT_VALUE = 350;
static const int BISHOP_VALUE = 350;
static const int ROOK_VALUE = 500;
static const int QUEEN_VALUE = 1000;
static const int KING_VALUE = 100000;

static const int ROOK_OPEN_FILE = 15;
static const int ROOK_SEMI_OPEN_FILE = 7;
static const int TWO_ROOKS_ON_SEVENTH = 15;

static const std::array<int, 8> DOUBLED_PAWN_BY_FILE = { -25, -5, -30, -20, -20, -20, -5, -20 };

static const int TRIPLED_PAWN = -50;
static const int ISOLATED_PAWN = -15;
static const int DOUBLED_AND_ISOLATED = -35;
static const int ISOLATED_PAWN_BLOCKED = -15;
static const int PASSED_PAWN = 15;
static const int PHALANX_VALUE = 30;
static const int PASSED_PAWN_BLOCKED = -20;
static const int CD_PAWN_BLOCKED_BY_PLAYER = -50;

static const std::array<int, 8> PASSED_PAWN_RANK_WHITE = { -5, -5, 5, 5, 25, 45, 150, 0 };
static const std::array<int, 8> PASSED_PAWN_RANK_BLACK = { 0, 150, 45, 25, 5, 5, -5, -5 };

static const int QUEEN_EARLY = -20;
static const int QUEENS_NOT_TRADED_WHEN_NOT_CASTLED = 15;
static const int BISHOP_PAIR = 45;
static const int BISHOP_MOBILITY = 2;
static const int ROOK_MOBILITY = 4;
static const int QUEEN_MOBILITY = 1;

static const int PAWN_SHIELD_LEFT = -15;
static const int PAWN_SHIELD_UPDOWN = -50;
static const int PAWN_SHIELD_RIGHT = -15;
static const int KING_AIR = -10;
static const int NOT_CASTLED = -30;

static const int SAME_PIECE_TWICE = -15;
static const int PIECES_ON_BACK_RANK = -15;

static const std::array<int, 2> FACTOR = { 1, -1 };

static const std::map<Piece, int> MATERIAL = {
    { wP, PAWN_VALUE }, { bP, -PAWN_VALUE },
    { wN, KNIGHT_VALUE }, { bN, -KNIGHT_VALUE },
    { wB, BISHOP_VALUE }, { bB, -BISHOP_VALUE },
    { wR, ROOK_VALUE }, { bR, -ROOK_VALUE },
    { wQ, QUEEN_VALUE }, { bQ, -QUEEN_VALUE },
    { wK, KING_VALUE }, { bK, -KING_VALUE },
    { EMPTY, 0 }
};

static const std::map<Square, bool> CENTER = {
    { e4, true }, { d4, true }, { e5, true }, { d5, true }
};

static const std::array<int, 64> REVERSE_PSQ = {
    56,57,58,59,60,61,62,63,
    48,49,50,51,52,53,54,55,
    40,41,42,43,44,45,46,47,
    32,33,34,35,36,37,38,39,
    24,25,26,27,28,29,30,31,
    16,17,18,19,20,21,22,23,
    8,9,10,11,12,13,14,15,
    0,1,2,3,4,5,6,7
};

static const std::array<int, 64> PAWN_SQUARE_TABLE = {
    0,0,0,0,0,0,0,0,
    5,10,-10,-20,-20,10,10,5,
    5,5,5,0,0,-10,5,5,
    0,0,10,20,20,0,0,0,
    5,5,10,25,25,10,5,5,
    10,10,20,30,30,20,10,10,
    50,50,50,50,50,50,50,50,
    0,0,0,0,0,0,0,0
};

static const std::array<int, 64> KNIGHT_SQUARE_TABLE = {
    -50,-30,-30,-30,-30,-30,-30,-50,
    -40,-20,0,-5,-5,0,-20,-40,
    -40,0,10,15,15,10,0,-40,
    -50,5,15,20,20,15,5,-50,
    -45,0,15,20,20,15,0,-45,
    -50,5,10,15,15,10,5,-50,
    -40,-20,0,5,5,0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

static const std::array<int, 64> BISHOP_SQUARE_TABLE = {
    -20,-10,-5,-10,-10,-10,-10,-20,
    -10,10,0,0,0,0,10,-10,
    -10,10,10,10,10,10,10,-10,
    -10,0,10,10,10,10,0,-10,
    -10,5,5,10,10,5,5,-10,
    -10,0,5,10,10,5,0,-10,
    -10,0,0,0,0,0,0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

static const std::array<int, 64> ROOK_SQUARE_TABLE = {
    -5,0,0,5,5,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    5,10,10,10,10,10,10,5,
    0,0,0,0,0,0,0,0
};

static const std::array<int, 64> QUEEN_SQUARE_TABLE = {
    -20,-10,-10,5,-5,-10,-10,-20,
    -10,0,0,0,0,0,0,-10,
    -10,-5,-5,-5,-5,-5,0,-10,
    0,0,5,5,5,5,0,-5,
    -5,0,5,5,5,5,0,-5,
    -10,0,5,5,5,5,0,-10,
    -10,0,0,0,0,0,0,-10,
    -20,-10,-10,-5,-5,-10,-10,-20
};

static const std::array<int, 64> KING_SQUARE_TABLE_MIDDLEGAME = {
    0,30,10,0,0,10,30,0,
    -30,-30,-30,-30,-30,-30,-30,-30,
    -50,-50,-50,-50,-50,-50,-50,-50,
    -70,-70,-70,-70,-70,-70,-70,-70,
    -70,-70,-70,-70,-70,-70,-70,-70,
    -70,-70,-70,-70,-70,-70,-70,-70,
    -70,-70,-70,-70,-70,-70,-70,-70,
    -70,-70,-70,-70,-70,-70,-70,-70
};

static const std::array<int, 64> KING_SQUARE_TABLE_ENDGAME = {
    -50,-10,0,0,0,0,-10,-50,
    -10,0,10,10,10,10,0,-10,
    0,10,15,15,15,15,10,0,
    0,10,15,20,20,15,10,0,
    0,10,15,20,20,15,10,0,
    0,10,15,15,15,15,10,0,
    -10,0,10,10,10,10,0,-10,
    -50,-10,0,0,0,0,-10,-50
};

int evaluatePosition(ChessBoard* board) {
    auto legalMoves = board->generateLegalMoves();
    if (legalMoves.empty()) {
        if (board->isCheck(board->sideToMove))
            return WIN_VALUE * FACTOR[reverseColor(board->sideToMove)];
        else
            return 0;
    }
    if (board->isThreeFoldRep())
        return 0;
    if (board->isInsufficientMaterial())
        return 0;
    int score = 0;
    int matVal, totPieces;
    std::tie(matVal, totPieces) = totalMaterialAndPieces(board);
    score += matVal;
    evaluateKnights(board, &score);
    evaluateBishops(board, &score);
    evaluateRooks(board, &score);
    evaluateQueens(board, &score);
    evaluateKings(board, &score, totPieces);
    evaluatePawns(board, &score);
    if (board->plyCnt <= 20) {
        int len = board->moveHistory.size();
        for (int i = len - 3; i > 0; i--) {
            if (board->moveHistory[i+2].moveData.from == board->moveHistory[i].moveData.to) {
                if (reverseColor(board->sideToMove) == WHITE && board->moveHistory[i+2].moveData.piece != wP)
                    score += SAME_PIECE_TWICE;
                else if (board->moveHistory[i+2].moveData.piece != bP)
                    score -= SAME_PIECE_TWICE;
            }
        }
    }
    if (board->plyCnt <= 25) {
        uint64_t whiteBackRank = (board->colorBitboards[WHITE] ^ board->pieceBitboards[wR]) & RANK_MASKS[R1];
        uint64_t blackBackRank = (board->colorBitboards[BLACK] ^ board->pieceBitboards[bR]) & RANK_MASKS[R8];
        score += (countBits(whiteBackRank) - countBits(blackBackRank)) * PIECES_ON_BACK_RANK;
    }
    if (board->plyCnt >= 25) {
        if (!board->whiteHasCastled)
            score += NOT_CASTLED;
        if (!board->blackHasCastled)
            score -= NOT_CASTLED;
    }
    if (!board->whiteHasCastled && board->pieceBitboards[bQ] != 0)
        score -= QUEENS_NOT_TRADED_WHEN_NOT_CASTLED;
    if (!board->blackHasCastled && board->pieceBitboards[wQ] != 0)
        score += QUEENS_NOT_TRADED_WHEN_NOT_CASTLED;
    return score;
}

std::pair<int, int> totalMaterialAndPieces(ChessBoard* board) {
    int sum = 0;
    int total = 0;
    for (auto piece : board->squareArray) {
        sum += MATERIAL.at(piece);
        if (piece != EMPTY)
            total++;
    }
    return std::make_pair(sum, total);
}

void evaluatePawns(ChessBoard* board, int* score) {
    uint64_t wpOrig = board->getPiecesByColor(pawn, WHITE);
    uint64_t bpOrig = board->getPiecesByColor(pawn, BLACK);
    uint64_t wp = wpOrig;
    uint64_t bp = bpOrig;
    std::array<int, 8> filesWhite = {0};
    std::array<int, 8> filesBlack = {0};
    bool phalanxFound = false;
    while (wp) {
        Square sq = Square(popLSB(&wp));
        *score += PAWN_SQUARE_TABLE[sq];
        int file = sqToFile(sq);
        filesWhite[file]++;
        uint64_t neighbors = fileNeighbors[file] & wpOrig;
        if (neighbors == 0) {
            if (filesWhite[file] >= 2)
                *score += DOUBLED_AND_ISOLATED;
            else
                *score += ISOLATED_PAWN;
            if (board->squareArray[sq + Square(NORTH)].getColor() == BLACK)
                *score += ISOLATED_PAWN_BLOCKED;
        } else if (!phalanxFound && sqToRank(sq) >= R4 && sqToFile(sq) >= C && sqToFile(sq) <= F) {
            while (neighbors) {
                Square nsq = Square(popLSB(&neighbors));
                if (sqToRank(nsq) == sqToRank(sq)) {
                    phalanxFound = true;
                    *score += PHALANX_VALUE;
                }
            }
        }
        uint64_t enemyNeighbors = (FILE_MASKS[sqToFile(sq)] | fileNeighbors[sqToFile(sq)]) & bpOrig;
        if (enemyNeighbors == 0) {
            *score += PASSED_PAWN;
            *score += PASSED_PAWN_RANK_WHITE[sqToRank(sq)];
        } else {
            bool passedAhead = true;
            while (enemyNeighbors) {
                Square p = Square(popLSB(&enemyNeighbors));
                if (sqToRank(p) > sqToRank(sq))
                    passedAhead = false;
            }
            if (passedAhead) {
                *score += PASSED_PAWN;
                *score += PASSED_PAWN_RANK_WHITE[sqToRank(sq)];
            }
        }
    }
    phalanxFound = false;
    while (bp) {
        Square sq = Square(popLSB(&bp));
        *score -= PAWN_SQUARE_TABLE[REVERSE_PSQ[sq]];
        int file = sqToFile(sq);
        filesBlack[file]++;
        uint64_t neighbors = fileNeighbors[file] & bpOrig;
        if (neighbors == 0) {
            if (filesBlack[file] >= 2)
                *score -= DOUBLED_AND_ISOLATED;
            else
                *score -= ISOLATED_PAWN;
            if (board->squareArray[sq + Square(SOUTH)].getColor() == WHITE)
                *score -= ISOLATED_PAWN_BLOCKED;
        } else if (!phalanxFound && sqToRank(sq) <= R5 && sqToFile(sq) >= C && sqToFile(sq) <= F) {
            while (neighbors) {
                Square nsq = Square(popLSB(&neighbors));
                if (sqToRank(nsq) == sqToRank(sq)) {
                    phalanxFound = true;
                    *score -= PHALANX_VALUE;
                }
            }
        }
        uint64_t enemyNeighbors = (FILE_MASKS[sqToFile(sq)] | fileNeighbors[sqToFile(sq)]) & wpOrig;
        if (enemyNeighbors == 0) {
            *score -= PASSED_PAWN;
            *score -= PASSED_PAWN_RANK_BLACK[sqToRank(sq)];
        } else {
            bool passedAhead = true;
            while (enemyNeighbors) {
                Square p = Square(popLSB(&enemyNeighbors));
                if (sqToRank(p) < sqToRank(sq))
                    passedAhead = false;
            }
            if (passedAhead) {
                *score -= PASSED_PAWN;
                *score -= PASSED_PAWN_RANK_BLACK[sqToRank(sq)];
            }
        }
    }
    for (int i = A_FILE; i <= H_FILE; i++) {
        if (filesWhite[i] == 2)
            *score += DOUBLED_PAWN_BY_FILE[i];
        if (filesWhite[i] == 3)
            *score += TRIPLED_PAWN;
        if (filesBlack[i] == 2)
            *score -= DOUBLED_PAWN_BY_FILE[i];
        if (filesBlack[i] == 3)
            *score -= TRIPLED_PAWN;
    }
}

void evaluateKnights(ChessBoard* board, int* score) {
    uint64_t wKnights = board->getPiecesByColor(knight, WHITE);
    uint64_t bKnights = board->getPiecesByColor(knight, BLACK);
    while (wKnights) {
        Square sq = Square(popLSB(&wKnights));
        if (sq == c3 && board->squareArray[c2] == wP)
            *score += CD_PAWN_BLOCKED_BY_PLAYER;
        *score += KNIGHT_SQUARE_TABLE[sq];
    }
    while (bKnights) {
        Square sq = Square(popLSB(&bKnights));
        if (sq == c6 && board->squareArray[c7] == bP)
            *score -= CD_PAWN_BLOCKED_BY_PLAYER;
        *score -= KNIGHT_SQUARE_TABLE[REVERSE_PSQ[sq]];
    }
}

void evaluateBishops(ChessBoard* board, int* score) {
    uint64_t wBishops = board->getPiecesByColor(bishop, WHITE);
    int wCount = 0;
    uint64_t bBishops = board->getPiecesByColor(bishop, BLACK);
    int bCount = 0;
    while (wBishops) {
        Square sq = Square(popLSB(&wBishops));
        if (sq == d3 && board->squareArray[d2] == wP)
            *score += CD_PAWN_BLOCKED_BY_PLAYER;
        Bitboard atk = getBishopAttacks(sq, board->occupied);
        *score += popCount(atk) * BISHOP_MOBILITY;
        *score += BISHOP_SQUARE_TABLE[sq];
        wCount++;
    }
    if (wCount >= 2)
        *score += BISHOP_PAIR;
    while (bBishops) {
        Square sq = Square(popLSB(&bBishops));
        if (sq == d6 && board->squareArray[d7] == bP)
            *score -= CD_PAWN_BLOCKED_BY_PLAYER;
        Bitboard atk = getBishopAttacks(sq, board->occupied);
        *score -= popCount(atk) * BISHOP_MOBILITY;
        *score -= BISHOP_SQUARE_TABLE[REVERSE_PSQ[sq]];
        bCount++;
    }
    if (bCount >= 2)
        *score -= BISHOP_PAIR;
}

void evaluateRooks(ChessBoard* board, int* score) {
    uint64_t wRooks = board->getPiecesByColor(rook, WHITE);
    uint64_t bRooks = board->getPiecesByColor(rook, BLACK);
    Bitboard pawns = board->pieceBitboards[wP] | board->pieceBitboards[bP];
    while (wRooks) {
        Square sq = Square(popLSB(&wRooks));
        Bitboard atk = getRookAttacks(sq, board->occupied);
        Bitboard pawnsOnFile = FILE_MASKS[sqToFile(sq)] & pawns;
        if (pawnsOnFile == 0)
            *score += ROOK_OPEN_FILE;
        else if (pawnsOnFile == 1)
            *score += ROOK_SEMI_OPEN_FILE;
        *score += popCount(atk) * ROOK_MOBILITY;
        *score += ROOK_SQUARE_TABLE[sq];
    }
    while (bRooks) {
        Square sq = Square(popLSB(&bRooks));
        Bitboard atk = getRookAttacks(sq, board->occupied);
        Bitboard pawnsOnFile = FILE_MASKS[sqToFile(sq)] & pawns;
        if (pawnsOnFile == 0)
            *score -= ROOK_OPEN_FILE;
        else if (pawnsOnFile == 1)
            *score -= ROOK_SEMI_OPEN_FILE;
        *score -= popCount(atk) * ROOK_MOBILITY;
        *score -= ROOK_SQUARE_TABLE[REVERSE_PSQ[sq]];
    }
}

void evaluateQueens(ChessBoard* board, int* score) {
    uint64_t wQueens = board->getPiecesByColor(queen, WHITE);
    uint64_t bQueens = board->getPiecesByColor(queen, BLACK);
    while (wQueens) {
        Square sq = Square(popLSB(&wQueens));
        if (sq != d1 && board->plyCnt <= 15)
            *score += QUEEN_EARLY;
        Bitboard atk = getBishopAttacks(sq, board->occupied) | getRookAttacks(sq, board->occupied);
        *score += popCount(atk) * QUEEN_MOBILITY;
        *score += QUEEN_SQUARE_TABLE[sq];
    }
    while (bQueens) {
        Square sq = Square(popLSB(&bQueens));
        if (sq != d8 && board->plyCnt <= 15)
            *score -= QUEEN_EARLY;
        Bitboard atk = getBishopAttacks(sq, board->occupied) | getRookAttacks(sq, board->occupied);
        *score -= popCount(atk) * QUEEN_MOBILITY;
        *score -= QUEEN_SQUARE_TABLE[REVERSE_PSQ[sq]];
    }
}

void evaluateKings(ChessBoard* board, int* score, int totalPieces) {
    bool noQueens = ((board->pieceBitboards[wQ] | board->pieceBitboards[bQ]) == 0);
    bool endgame = (totalPieces <= 25 && noQueens);
    Square wk = Square(bitScanForward(board->getPiecesByColor(king, WHITE)));
    Square bk = Square(bitScanForward(board->getPiecesByColor(king, BLACK)));
    if (endgame) {
        *score += KING_SQUARE_TABLE_ENDGAME[wk];
        *score -= KING_SQUARE_TABLE_ENDGAME[REVERSE_PSQ[bk]];
    } else {
        *score += KING_SQUARE_TABLE_MIDDLEGAME[wk];
        *score -= KING_SQUARE_TABLE_MIDDLEGAME[REVERSE_PSQ[bk]];
        if (!board->whiteHasCastled && !board->whiteKingsideCastling && !board->whiteQueensideCastling) {
            Bitboard wp = board->pieceBitboards[wP];
            Bitboard nw = shiftBitboard(S_TO_BB[wk], NW) & wp;
            Bitboard n = shiftBitboard(S_TO_BB[wk], NORTH) & wp;
            Bitboard ne = shiftBitboard(S_TO_BB[wk], NE) & wp;
            if (nw == 0)
                *score += PAWN_SHIELD_LEFT;
            if (n == 0)
                *score += PAWN_SHIELD_UPDOWN;
            if (ne == 0)
                *score += PAWN_SHIELD_RIGHT;
        } else if (!board->blackHasCastled && !board->blackKingsideCastling && !board->blackQueensideCastling) {
            Bitboard bp = board->pieceBitboards[bP];
            Bitboard sw = shiftBitboard(S_TO_BB[bk], SW) & bp;
            Bitboard s = shiftBitboard(S_TO_BB[bk], SOUTH) & bp;
            Bitboard se = shiftBitboard(S_TO_BB[bk], SE) & bp;
            if (sw == 0)
                *score -= PAWN_SHIELD_LEFT;
            if (s == 0)
                *score -= PAWN_SHIELD_UPDOWN;
            if (se == 0)
                *score -= PAWN_SHIELD_RIGHT;
        } else {
            int airW = popCount(kingAttacks(wk) & board->emptyBB);
            int airB = popCount(kingAttacks(bk) & board->emptyBB);
            if (airW >= 2)
                *score += airW * KING_AIR;
            if (airB >= 2)
                *score -= airB * KING_AIR;
        }
    }
}

} // namespace Chess
