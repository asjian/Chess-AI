#include "MoveGen.h"
#include "Constants.h"
#include "Bitboard.h"
#include <cstdint>

namespace Chess {

int MoveGenerator::countPins(ChessBoard* board, Color opp, uint64_t occ, Square kingPos) {
    uint64_t diagOpp = board->getPiecesByColor(rook, opp) | board->getPiecesByColor(queen, opp);
    uint64_t orthoOpp = board->getPiecesByColor(bishop, opp) | board->getPiecesByColor(queen, opp);
    uint64_t cand = getBishopAttacks(kingPos, board->colorBitboards[opp]) & diagOpp;
    cand |= getRookAttacks(kingPos, board->colorBitboards[opp]) & orthoOpp;
    int pinCount = 0;
    while (cand) {
        int pos = popLeastSignificantBit(&cand);
        uint64_t between = SQUARES_BETWEEN[kingPos][pos] & board->pieceBitboards[reverseColor(opp)];
        if (between && ((between & (between - 1)) == 0))
            pinCount++;
    }
    return pinCount;
}

uint64_t MoveGenerator::allAttacks(ChessBoard* board, Color opp, uint64_t occ, uint64_t ortho, uint64_t diag) {
    uint64_t att = 0;
    uint64_t kingXray = board->getPiecesByColor(king, reverseColor(opp));
    Square oppKing = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, opp)));
    att |= kingAttacks(oppKing);
    uint64_t oppPawns = board->getPiecesByColor(pawn, opp);
    att |= allPawnAttacks(oppPawns, opp);
    uint64_t oppKnights = board->getPiecesByColor(knight, opp);
    while (oppKnights) {
        int pos = popLeastSignificantBit(&oppKnights);
        att |= knightAttacks(static_cast<Square>(pos));
    }
    uint64_t oppBishops = diag;
    while (oppBishops) {
        int pos = popLeastSignificantBit(&oppBishops);
        att |= getBishopAttacks(static_cast<Square>(pos), occ ^ kingXray);
    }
    uint64_t oppRooks = ortho;
    while (oppRooks) {
        int pos = popLeastSignificantBit(&oppRooks);
        att |= getRookAttacks(static_cast<Square>(pos), occ ^ kingXray);
    }
    return att;
}

uint64_t MoveGenerator::allPawnAttacks(uint64_t pawns, Color c) {
    if (c == WHITE)
        return shiftBitboard(pawns, NE) | shiftBitboard(pawns, NW);
    return shiftBitboard(pawns, SE) | shiftBitboard(pawns, SW);
}

uint64_t MoveGenerator::kingAttacks(Square sq) {
    return KING_ATTACKS[sq];
}

uint64_t MoveGenerator::knightAttacks(Square sq) {
    return KNIGHT_ATTACKS[sq];
}

uint64_t MoveGenerator::getBishopAttacks(Square sq, uint64_t blockers) {
    uint64_t index = (blockers & bishopMasks[sq]) * BISHOP_MAGICS[sq];
    return BISHOP_ATTACKS[sq][index >> bishopShifts[sq]];
}

uint64_t MoveGenerator::getRookAttacks(Square sq, uint64_t blockers) {
    uint64_t index = (blockers & rookMasks[sq]) * ROOK_MAGICS[sq];
    return ROOK_ATTACKS[sq][index >> rookShifts[sq]];
}

void MoveGenerator::genMovesFromLocations(ChessBoard* board, std::vector<Move>& moves, Square origin, uint64_t locs, Color c) {
    while (locs) {
        int pos = popLeastSignificantBit(&locs);
        uint64_t targetBB = S_TO_BB[pos];
        Piece pc = board->squareArray[origin];
        if (pc == wP && (targetBB & RANK_MASKS[R8]) != 0) {
            Move m1 { origin, static_cast<Square>(pos), PROMOTION, wP, board->squareArray[pos], c, wN, false };
            Move m2 { origin, static_cast<Square>(pos), PROMOTION, wP, board->squareArray[pos], c, wR, false };
            Move m3 { origin, static_cast<Square>(pos), PROMOTION, wP, board->squareArray[pos], c, wQ, false };
            Move m4 { origin, static_cast<Square>(pos), PROMOTION, wP, board->squareArray[pos], c, wB, false };
            moves.push_back(m1); moves.push_back(m2); moves.push_back(m3); moves.push_back(m4);
        } else if (pc == bP && (targetBB & RANK_MASKS[R1]) != 0) {
            Move m1 { origin, static_cast<Square>(pos), PROMOTION, bP, board->squareArray[pos], c, bN, false };
            Move m2 { origin, static_cast<Square>(pos), PROMOTION, bP, board->squareArray[pos], c, bR, false };
            Move m3 { origin, static_cast<Square>(pos), PROMOTION, bP, board->squareArray[pos], c, bQ, false };
            Move m4 { origin, static_cast<Square>(pos), PROMOTION, bP, board->squareArray[pos], c, bB, false };
            moves.push_back(m1); moves.push_back(m2); moves.push_back(m3); moves.push_back(m4);
        } else {
            Move newMv { origin, static_cast<Square>(pos), (board->squareArray[pos] == EMPTY) ? QUIET : CAPTURE, pc, board->squareArray[pos], c, EMPTY, false };
            moves.push_back(newMv);
        }
    }
}

void MoveGenerator::getKingMoves(ChessBoard* board, std::vector<Move>& moves, Square kingSq, uint64_t attacked, uint64_t own, Color c) {
    uint64_t kingMv = kingAttacks(kingSq) & ~attacked & ~own;
    genMovesFromLocations(board, moves, kingSq, kingMv, c);
}

void MoveGenerator::getKnightMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t knights, uint64_t pinned, uint64_t own, Color c, uint64_t allowed) {
    knights &= ~pinned;
    while (knights) {
        int pos = popLeastSignificantBit(&knights);
        uint64_t nMoves = knightAttacks(static_cast<Square>(pos)) & ~own & allowed;
        genMovesFromLocations(board, moves, static_cast<Square>(pos), nMoves, c);
    }
}

void MoveGenerator::getPawnMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t pawns, uint64_t pinned, uint64_t occ, uint64_t opp, Color c, uint64_t allowed) {
    uint64_t pinnedPawns = pawns & pinned;
    uint64_t sqBB;
    Square kingPos;
    if (pinnedPawns) {
        while (pinnedPawns) {
            int pos = popLeastSignificantBit(&pinnedPawns);
            sqBB = S_TO_BB[pos];
            kingPos = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, c)));
            uint64_t pMoves = (shiftBitboard(sqBB, PAWN_PUSH_DIRECTION[c]) & ~occ) & allowed & LINE[kingPos][pos];
            if (pMoves) {
                if (sqBB & RANK_MASKS[STARTING_RANK[c]])
                    pMoves |= (shiftBitboard(sqBB, PAWN_PUSH_DIRECTION[c]*2) & ~occ) & allowed & LINE[kingPos][pos];
            }
            pMoves |= colorToPawnLookup[c][pos] & opp & allowed & LINE[kingPos][pos];
            genMovesFromLocations(board, moves, static_cast<Square>(pos), pMoves, c);
        }
    }
    uint64_t unpinnedPawns = pawns & ~pinned;
    while (unpinnedPawns) {
        int pos = popLeastSignificantBit(&unpinnedPawns);
        sqBB = S_TO_BB[pos];
        uint64_t pMoves = (shiftBitboard(sqBB, PAWN_PUSH_DIRECTION[c]) & ~occ) & allowed;
        if (board->squareArray[pos + PAWN_PUSH_DIRECTION[c]] == EMPTY) {
            if (sqBB & RANK_MASKS[STARTING_RANK[c]])
                pMoves |= (shiftBitboard(sqBB, PAWN_PUSH_DIRECTION[c]*2) & ~occ) & allowed;
        }
        if (c == WHITE)
            pMoves |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[pos] & opp) & allowed;
        else
            pMoves |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[pos] & opp) & allowed;
        genMovesFromLocations(board, moves, static_cast<Square>(pos), pMoves, c);
    }
}

void MoveGenerator::getBishopMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t bishops, uint64_t pinned, uint64_t own, uint64_t opp, Color c, uint64_t allowed) {
    uint64_t pinnedBishops = bishops & pinned;
    Square kingPos;
    if (pinnedBishops) {
        while (pinnedBishops) {
            int pos = popLeastSignificantBit(&pinnedBishops);
            kingPos = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, c)));
            uint64_t poss = getBishopAttacks(static_cast<Square>(pos), board->occupied) & LINE[kingPos][pos] & allowed & ~own;
            genMovesFromLocations(board, moves, static_cast<Square>(pos), poss, c);
        }
    }
    uint64_t freeBishops = bishops & ~pinned;
    while (freeBishops) {
        int pos = popLeastSignificantBit(&freeBishops);
        uint64_t bMoves = getBishopAttacks(static_cast<Square>(pos), own | opp) & ~own & allowed;
        genMovesFromLocations(board, moves, static_cast<Square>(pos), bMoves, c);
    }
}

void MoveGenerator::getRookMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t rooks, uint64_t pinned, uint64_t own, uint64_t opp, Color c, uint64_t allowed) {
    uint64_t pinnedRooks = rooks & pinned;
    Square kingPos;
    if (pinnedRooks) {
        while (pinnedRooks) {
            int pos = popLeastSignificantBit(&pinnedRooks);
            kingPos = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, c)));
            uint64_t poss = getRookAttacks(static_cast<Square>(pos), board->occupied) & LINE[kingPos][pos] & allowed & ~own;
            genMovesFromLocations(board, moves, static_cast<Square>(pos), poss, c);
        }
    }
    uint64_t freeRooks = rooks & ~pinned;
    while (freeRooks) {
        int pos = popLeastSignificantBit(&freeRooks);
        uint64_t rMoves = getRookAttacks(static_cast<Square>(pos), own | opp) & ~own & allowed;
        genMovesFromLocations(board, moves, static_cast<Square>(pos), rMoves, c);
    }
}

void MoveGenerator::getQueenMoves(ChessBoard* board, std::vector<Move>& moves, uint64_t queens, uint64_t pinned, uint64_t own, uint64_t opp, Color c, uint64_t allowed) {
    uint64_t pinnedQueens = queens & pinned;
    Square kingPos;
    if (pinnedQueens) {
        while (pinnedQueens) {
            int pos = popLeastSignificantBit(&pinnedQueens);
            kingPos = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, c)));
            uint64_t poss = (getBishopAttacks(static_cast<Square>(pos), board->occupied) | getRookAttacks(static_cast<Square>(pos), board->occupied))
                            & LINE[kingPos][pos] & allowed & ~own;
            genMovesFromLocations(board, moves, static_cast<Square>(pos), poss, c);
        }
    }
    uint64_t freeQueens = queens & ~pinned;
    while (freeQueens) {
        int pos = popLeastSignificantBit(&freeQueens);
        uint64_t rMoves = getRookAttacks(static_cast<Square>(pos), own | opp) & ~own & allowed;
        uint64_t bMoves = getBishopAttacks(static_cast<Square>(pos), own | opp) & ~own & allowed;
        genMovesFromLocations(board, moves, static_cast<Square>(pos), rMoves | bMoves, c);
    }
}

void MoveGenerator::getCastlingMoves(ChessBoard* board, std::vector<Move>& moves, Square kingSq, uint64_t attacked, Color c) {
    bool canKingSide = false;
    bool canQueenSide = false;
    uint64_t avail = ~attacked & board->emptyBB;
    if (c == WHITE) {
        canKingSide = ((S_TO_BB[f1] & avail) != 0) && ((S_TO_BB[g1] & avail) != 0);
        canQueenSide = ((S_TO_BB[b1] & board->emptyBB) != 0) && ((S_TO_BB[c1] & avail) != 0) && ((S_TO_BB[d1] & avail) != 0);
        if (board->whiteKingsideCastling && canKingSide && board->squareArray[h1] == wR)
            moves.push_back(Move{ e1, g1, KCASTLE, wK, EMPTY, WHITE, EMPTY, false });
        if (board->whiteQueensideCastling && canQueenSide && board->squareArray[a1] == wR)
            moves.push_back(Move{ e1, c1, QCASTLE, wK, EMPTY, WHITE, EMPTY, false });
    } else {
        canKingSide = ((S_TO_BB[f8] & avail) != 0) && ((S_TO_BB[g8] & avail) != 0);
        canQueenSide = ((S_TO_BB[b8] & board->emptyBB) != 0) && ((S_TO_BB[c8] & avail) != 0) && ((S_TO_BB[d8] & avail) != 0);
        if (board->blackKingsideCastling && canKingSide && board->squareArray[h8] == bR)
            moves.push_back(Move{ e8, g8, KCASTLE, bK, EMPTY, BLACK, EMPTY, false });
        if (board->blackQueensideCastling && canQueenSide && board->squareArray[a8] == bR)
            moves.push_back(Move{ e8, c8, QCASTLE, bK, EMPTY, BLACK, EMPTY, false });
    }
}

std::vector<Move> MoveGenerator::generateLegalMoves(ChessBoard* board) {
    std::vector<Move> allMoves;
    Color pl = board->sideToMove;
    Color op = reverseColor(pl);
    uint64_t ownPieces = board->colorBitboards[pl];
    uint64_t oppPieces = board->colorBitboards[op];
    uint64_t pawns = board->getPiecesByColor(pawn, pl);
    uint64_t allowed = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t orthoOwn = board->getPiecesByColor(rook, pl) | board->getPiecesByColor(queen, pl);
    uint64_t diagOwn = board->getPiecesByColor(bishop, pl) | board->getPiecesByColor(queen, pl);
    uint64_t orthoOpp = board->getPiecesByColor(rook, op) | board->getPiecesByColor(queen, op);
    uint64_t diagOpp = board->getPiecesByColor(bishop, op) | board->getPiecesByColor(queen, op);
    uint64_t att = board->getAllAttacks(op, board->occupiedBB, orthoOpp, diagOpp);
    Square kingSq = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, pl)));
    getKingMoves(board, allMoves, kingSq, att, ownPieces, pl);
    uint64_t checks = 0;
    if (pl == WHITE)
        checks |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[bP]);
    else
        checks |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[wP]);
    checks |= (knightAttacks(kingSq) & board->getPiecesByColor(knight, op));
    uint64_t cand = (getBishopAttacks(kingSq, board->colorBitboards[op]) & diagOpp);
    cand |= (getRookAttacks(kingSq, board->colorBitboards[op]) & orthoOpp);
    uint64_t pinned = 0;
    while (cand) {
        int pos = popLeastSignificantBit(&cand);
        uint64_t between = SQUARES_BETWEEN[kingSq][pos] & ownPieces;
        if (between == 0)
            checks ^= S_TO_BB[pos];
        else if (between && ((between & (between - 1)) == 0))
            pinned ^= between;
    }
    int numChecks = popCount(checks);
    if (numChecks == 2)
        return allMoves;
    else if (numChecks == 1) {
        Square chk = static_cast<Square>(bitScanForward(checks));
        Piece chkPiece = board->squareArray[chk];
        Square epShift = board->enPassantSquare + static_cast<Square>(PAWN_PUSH_DIRECTION[op]);
        if (chk == epShift && (chkPiece == wP || chkPiece == bP)) {
            uint64_t pawnsCap = colorToPawnLookup[op][board->enPassantSquare] & pawns;
            uint64_t unpinned = pawnsCap & ~pinned;
            while (unpinned) {
                int pos = popLeastSignificantBit(&unpinned);
                Move mv { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], chkPiece, pl, board->squareArray[static_cast<Square>(pos)], false };
                board->makeMove(mv);
                if (board->isCheck(pl))
                    board->undo();
                else {
                    board->undo();
                    allMoves.push_back(mv);
                }
            }
            uint64_t pinPawns = pawnsCap & pinned & LINE[board->enPassantSquare][kingSq];
            if (pinPawns) {
                int pos = popLeastSignificantBit(&pinPawns);
                allMoves.push_back(Move { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], chkPiece, pl, board->squareArray[static_cast<Square>(pos)], false });
            }
        }
        uint64_t possCaptures = 0;
        if (pl == WHITE) {
            possCaptures |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[chk] & board->pieceBitboards[wP]);
            possCaptures |= (knightAttacks(chk) & board->pieceBitboards[wN]);
            possCaptures |= (getBishopAttacks(chk, board->occupiedBB) & diagOwn);
            possCaptures |= (getRookAttacks(chk, board->occupiedBB) & orthoOwn);
            possCaptures &= ~pinned;
        } else {
            possCaptures |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[chk] & board->pieceBitboards[bP]);
            possCaptures |= (knightAttacks(chk) & board->pieceBitboards[bN]);
            possCaptures |= (getBishopAttacks(chk, board->occupiedBB) & diagOwn);
            possCaptures |= (getRookAttacks(chk, board->occupiedBB) & orthoOwn);
            possCaptures &= ~pinned;
        }
        while (possCaptures) {
            int pos = popLeastSignificantBit(&possCaptures);
            genMovesFromLocations(board, allMoves, static_cast<Square>(pos), S_TO_BB[kingSq], pl);
        }
        if (chkPiece == bN || chkPiece == wN)
            return allMoves;
        else if (chkPiece == bP || chkPiece == wP)
            return allMoves;
        return allMoves;
    }
    uint64_t knights = board->getPiecesByColor(knight, pl);
    getKnightMoves(board, allMoves, knights, pinned, ownPieces, pl, allowed);
    getPawnMoves(board, allMoves, pawns, pinned, board->occupiedBB, oppPieces, pl, allowed);
    getBishopMoves(board, allMoves, diagOwn, pinned, ownPieces, oppPieces, pl, allowed);
    getRookMoves(board, allMoves, orthoOwn, pinned, ownPieces, oppPieces, pl, allowed);
    getCastlingMoves(board, allMoves, kingSq, att, pl);
    if (board->enPassantSquare != EMPTYSQ) {
        uint64_t epPawns = colorToPawnLookup[op][board->enPassantSquare] & pawns;
        uint64_t unpinned = epPawns & ~pinned;
        while (unpinned) {
            int pos = popLeastSignificantBit(&unpinned);
            Move mv { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], board->squareArray[board->enPassantSquare], pl, board->squareArray[static_cast<Square>(pos)], false };
            board->makeMoveNoUpdate(mv);
            if (board->isCheck(pl))
                board->undoNoUpdate(mv);
            else {
                board->undoNoUpdate(mv);
                allMoves.push_back(mv);
            }
        }
        uint64_t pinPawns = epPawns & pinned & LINE[board->enPassantSquare][kingSq];
        if (pinPawns) {
            int pos = popLeastSignificantBit(&pinPawns);
            allMoves.push_back(Move { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], board->squareArray[board->enPassantSquare], pl, board->squareArray[static_cast<Square>(pos)], false });
        }
    }
    return allMoves;
}

std::vector<Move> MoveGenerator::generateCaptures(ChessBoard* board) {
    std::vector<Move> moves;
    Color pl = board->sideToMove;
    Color op = reverseColor(pl);
    uint64_t own = board->colorBitboards[pl];
    uint64_t opp = board->colorBitboards[op];
    uint64_t pawns = board->getPiecesByColor(pawn, pl);
    uint64_t allowed = opp;
    uint64_t orthoOwn = board->getPiecesByColor(rook, pl) | board->getPiecesByColor(queen, pl);
    uint64_t diagOwn = board->getPiecesByColor(bishop, pl) | board->getPiecesByColor(queen, pl);
    uint64_t orthoOpp = board->getPiecesByColor(rook, op) | board->getPiecesByColor(queen, op);
    uint64_t diagOpp = board->getPiecesByColor(bishop, op) | board->getPiecesByColor(queen, op);
    uint64_t att = board->getAllAttacks(op, board->occupiedBB, orthoOpp, diagOpp);
    Square kingSq = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, pl)));
    getKingMoves(board, moves, kingSq, att | ~allowed, own, pl);
    uint64_t checks = 0;
    if (pl == WHITE)
        checks |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[bP]);
    else
        checks |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[wP]);
    checks |= (knightAttacks(kingSq) & board->getPiecesByColor(knight, op));
    uint64_t cand = (getBishopAttacks(kingSq, board->colorBitboards[op]) & diagOpp);
    cand |= (getRookAttacks(kingSq, board->colorBitboards[op]) & orthoOpp);
    uint64_t pinned = 0;
    while (cand) {
        int pos = popLeastSignificantBit(&cand);
        uint64_t between = SQUARES_BETWEEN[kingSq][pos] & own;
        if (between == 0)
            checks ^= S_TO_BB[pos];
        else if (between && ((between & (between - 1)) == 0))
            pinned ^= between;
    }
    int numChecks = popCount(checks);
    if (numChecks == 2)
        return moves;
    else if (numChecks == 1) {
        Square chk = static_cast<Square>(bitScanForward(checks));
        Piece chkPiece = board->squareArray[chk];
        Square epShift = board->enPassantSquare + static_cast<Square>(PAWN_PUSH_DIRECTION[op]);
        if (chk == epShift && (chkPiece == wP || chkPiece == bP)) {
            uint64_t pawnsCap = colorToPawnLookup[op][board->enPassantSquare] & pawns;
            uint64_t unpinned = pawnsCap & ~pinned;
            while (unpinned) {
                int pos = popLeastSignificantBit(&unpinned);
                Move mv { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], chkPiece, pl, board->squareArray[static_cast<Square>(pos)], false };
                board->makeMove(mv);
                if (board->isCheck(pl))
                    board->undo();
                else {
                    board->undo();
                    moves.push_back(mv);
                }
            }
            uint64_t pinPawns = pawnsCap & pinned & LINE[board->enPassantSquare][kingSq];
            if (pinPawns) {
                int pos = popLeastSignificantBit(&pinPawns);
                moves.push_back(Move { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], board->squareArray[board->enPassantSquare], pl, board->squareArray[static_cast<Square>(pos)], false });
            }
        }
        uint64_t possCapt = 0;
        if (pl == WHITE) {
            possCapt |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[wP]);
            possCapt |= (knightAttacks(kingSq) & board->pieceBitboards[wN]);
            possCapt |= (getBishopAttacks(kingSq, board->occupiedBB) & diagOwn);
            possCapt |= (getRookAttacks(kingSq, board->occupiedBB) & orthoOwn);
            possCapt &= ~pinned;
        } else {
            possCapt |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[bP]);
            possCapt |= (knightAttacks(kingSq) & board->pieceBitboards[bN]);
            possCapt |= (getBishopAttacks(kingSq, board->occupiedBB) & diagOwn);
            possCapt |= (getRookAttacks(kingSq, board->occupiedBB) & orthoOwn);
            possCapt &= ~pinned;
        }
        while (possCapt) {
            int pos = popLeastSignificantBit(&possCapt);
            genMovesFromLocations(board, moves, static_cast<Square>(pos), S_TO_BB[kingSq], pl);
        }
        if (chkPiece == bN || chkPiece == wN)
            return moves;
        else if (chkPiece == bP || chkPiece == wP)
            return moves;
        return moves;
    }
    uint64_t knights = board->getPiecesByColor(knight, pl);
    getKnightMoves(board, moves, knights, pinned, own, pl, allowed);
    getPawnMoves(board, moves, pawns, pinned, board->occupiedBB, opp, pl, allowed);
    getBishopMoves(board, moves, diagOwn, pinned, own, opp, pl, allowed);
    getRookMoves(board, moves, orthoOwn, pinned, own, opp, pl, allowed);
    getCastlingMoves(board, moves, kingSq, att, pl);
    if (board->enPassantSquare != EMPTYSQ) {
        uint64_t epPawns = colorToPawnLookup[op][board->enPassantSquare] & pawns;
        uint64_t unpinned = epPawns & ~pinned;
        while (unpinned) {
            int pos = popLeastSignificantBit(&unpinned);
            Move mv { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], board->squareArray[board->enPassantSquare], pl, board->squareArray[static_cast<Square>(pos)], false };
            board->makeMoveNoUpdate(mv);
            if (board->isCheck(pl))
                board->undoNoUpdate(mv);
            else {
                board->undoNoUpdate(mv);
                moves.push_back(mv);
            }
        }
        uint64_t pinPawns = epPawns & pinned & LINE[board->enPassantSquare][kingSq];
        if (pinPawns) {
            int pos = popLeastSignificantBit(&pinPawns);
            moves.push_back(Move { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], board->squareArray[board->enPassantSquare], pl, board->squareArray[static_cast<Square>(pos)], false });
        }
    }
    return moves;
}

std::vector<Move> MoveGenerator::generateCaptures(ChessBoard* board) {
    std::vector<Move> moves;
    Color pl = board->sideToMove;
    Color op = reverseColor(pl);
    uint64_t own = board->colorBitboards[pl];
    uint64_t opp = board->colorBitboards[op];
    uint64_t pawns = board->getPiecesByColor(pawn, pl);
    uint64_t allowed = opp;
    uint64_t orthoOwn = board->getPiecesByColor(rook, pl) | board->getPiecesByColor(queen, pl);
    uint64_t diagOwn = board->getPiecesByColor(bishop, pl) | board->getPiecesByColor(queen, pl);
    uint64_t orthoOpp = board->getPiecesByColor(rook, op) | board->getPiecesByColor(queen, op);
    uint64_t diagOpp = board->getPiecesByColor(bishop, op) | board->getPiecesByColor(queen, op);
    uint64_t att = board->getAllAttacks(op, board->occupiedBB, orthoOpp, diagOpp);
    Square kingSq = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, pl)));
    getKingMoves(board, moves, kingSq, att | ~allowed, own, pl);
    uint64_t checks = 0;
    if (pl == WHITE)
        checks |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[bP]);
    else
        checks |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[wP]);
    checks |= (knightAttacks(kingSq) & board->getPiecesByColor(knight, op));
    uint64_t cand = (getBishopAttacks(kingSq, board->colorBitboards[op]) & diagOpp);
    cand |= (getRookAttacks(kingSq, board->colorBitboards[op]) & orthoOpp);
    uint64_t pinned = 0;
    while (cand) {
        int pos = popLeastSignificantBit(&cand);
        uint64_t between = SQUARES_BETWEEN[kingSq][pos] & own;
        if (between == 0)
            checks ^= S_TO_BB[pos];
        else if (between && ((between & (between - 1)) == 0))
            pinned ^= between;
    }
    int numChecks = popCount(checks);
    if (numChecks == 2)
        return moves;
    else if (numChecks == 1) {
        Square chk = static_cast<Square>(bitScanForward(checks));
        Piece chkPiece = board->squareArray[chk];
        Square epShift = board->enPassantSquare + static_cast<Square>(PAWN_PUSH_DIRECTION[op]);
        if (chk == epShift && (chkPiece == wP || chkPiece == bP)) {
            uint64_t pawnsCap = colorToPawnLookup[op][board->enPassantSquare] & pawns;
            uint64_t unpinned = pawnsCap & ~pinned;
            while (unpinned) {
                int pos = popLeastSignificantBit(&unpinned);
                Move mv { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], chkPiece, pl, board->squareArray[static_cast<Square>(pos)], false };
                board->makeMove(mv);
                if (board->isCheck(pl))
                    board->undo();
                else {
                    board->undo();
                    moves.push_back(mv);
                }
            }
            uint64_t pinPawns = pawnsCap & pinned & LINE[board->enPassantSquare][kingSq];
            if (pinPawns) {
                int pos = popLeastSignificantBit(&pinPawns);
                moves.push_back(Move { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], board->squareArray[board->enPassantSquare], pl, board->squareArray[static_cast<Square>(pos)], false });
            }
        }
        uint64_t possCapt = 0;
        if (pl == WHITE) {
            possCapt |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[wP]);
            possCapt |= (knightAttacks(kingSq) & board->pieceBitboards[wN]);
            possCapt |= (getBishopAttacks(kingSq, board->occupiedBB) & diagOwn);
            possCapt |= (getRookAttacks(kingSq, board->occupiedBB) & orthoOwn);
            possCapt &= ~pinned;
        } else {
            possCapt |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[bP]);
            possCapt |= (knightAttacks(kingSq) & board->pieceBitboards[bN]);
            possCapt |= (getBishopAttacks(kingSq, board->occupiedBB) & diagOwn);
            possCapt |= (getRookAttacks(kingSq, board->occupiedBB) & orthoOwn);
            possCapt &= ~pinned;
        }
        while (possCapt) {
            int pos = popLeastSignificantBit(&possCapt);
            genMovesFromLocations(board, moves, static_cast<Square>(pos), S_TO_BB[kingSq], pl);
        }
        if (chkPiece == bN || chkPiece == wN)
            return moves;
        else if (chkPiece == bP || chkPiece == wP)
            return moves;
        return moves;
    }
    return moves;
}

std::vector<Move> MoveGenerator::generateCaptures(ChessBoard* board) {
    std::vector<Move> moves;
    Color pl = board->sideToMove;
    Color op = reverseColor(pl);
    uint64_t own = board->colorBitboards[pl];
    uint64_t opp = board->colorBitboards[op];
    uint64_t pawns = board->getPiecesByColor(pawn, pl);
    uint64_t allowed = opp;
    uint64_t orthoOwn = board->getPiecesByColor(rook, pl) | board->getPiecesByColor(queen, pl);
    uint64_t diagOwn = board->getPiecesByColor(bishop, pl) | board->getPiecesByColor(queen, pl);
    uint64_t orthoOpp = board->getPiecesByColor(rook, op) | board->getPiecesByColor(queen, op);
    uint64_t diagOpp = board->getPiecesByColor(bishop, op) | board->getPiecesByColor(queen, op);
    uint64_t att = board->getAllAttacks(op, board->occupiedBB, orthoOpp, diagOpp);
    Square kingSq = static_cast<Square>(bitScanForward(board->getPiecesByColor(king, pl)));
    getKingMoves(board, moves, kingSq, att | ~allowed, own, pl);
    uint64_t checks = 0;
    if (pl == WHITE)
        checks |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[bP]);
    else
        checks |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[wP]);
    checks |= (knightAttacks(kingSq) & board->getPiecesByColor(knight, op));
    uint64_t cand = (getBishopAttacks(kingSq, board->colorBitboards[op]) & diagOpp);
    cand |= (getRookAttacks(kingSq, board->colorBitboards[op]) & orthoOpp);
    uint64_t pinned = 0;
    while (cand) {
        int pos = popLeastSignificantBit(&cand);
        uint64_t between = SQUARES_BETWEEN[kingSq][pos] & own;
        if (between == 0)
            checks ^= S_TO_BB[pos];
        else if (between && ((between & (between - 1)) == 0))
            pinned ^= between;
    }
    int numChecks = popCount(checks);
    if (numChecks == 2)
        return moves;
    else if (numChecks == 1) {
        Square chk = static_cast<Square>(bitScanForward(checks));
        Piece chkPiece = board->squareArray[chk];
        Square epShift = board->enPassantSquare + static_cast<Square>(PAWN_PUSH_DIRECTION[op]);
        if (chk == epShift && (chkPiece == wP || chkPiece == bP)) {
            uint64_t pawnsCap = colorToPawnLookup[op][board->enPassantSquare] & pawns;
            uint64_t unpinned = pawnsCap & ~pinned;
            while (unpinned) {
                int pos = popLeastSignificantBit(&unpinned);
                Move mv { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], chkPiece, pl, board->squareArray[static_cast<Square>(pos)], false };
                board->makeMove(mv);
                if (board->isCheck(pl))
                    board->undo();
                else {
                    board->undo();
                    moves.push_back(mv);
                }
            }
            uint64_t pinPawns = pawnsCap & pinned & LINE[board->enPassantSquare][kingSq];
            if (pinPawns) {
                int pos = popLeastSignificantBit(&pinPawns);
                moves.push_back(Move { static_cast<Square>(pos), board->enPassantSquare, ENPASSANT, board->squareArray[static_cast<Square>(pos)], board->squareArray[board->enPassantSquare], pl, board->squareArray[static_cast<Square>(pos)], false });
            }
        }
        uint64_t possCapt = 0;
        if (pl == WHITE) {
            possCapt |= (BLACK_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[wP]);
            possCapt |= (knightAttacks(kingSq) & board->pieceBitboards[wN]);
            possCapt |= (getBishopAttacks(kingSq, board->occupiedBB) & diagOwn);
            possCapt |= (getRookAttacks(kingSq, board->occupiedBB) & orthoOwn);
            possCapt &= ~pinned;
        } else {
            possCapt |= (WHITE_PAWN_ATTACKS_SQUARE_LOOKUP[kingSq] & board->pieceBitboards[bP]);
            possCapt |= (knightAttacks(kingSq) & board->pieceBitboards[bN]);
            possCapt |= (getBishopAttacks(kingSq, board->occupiedBB) & diagOwn);
            possCapt |= (getRookAttacks(kingSq, board->occupiedBB) & orthoOwn);
            possCapt &= ~pinned;
        }
        while (possCapt) {
            int pos = popLeastSignificantBit(&possCapt);
            genMovesFromLocations(board, moves, static_cast<Square>(pos), S_TO_BB[kingSq], pl);
        }
        if (chkPiece == bN || chkPiece == wN)
            return moves;
        else if (chkPiece == bP || chkPiece == wP)
            return moves;
        return moves;
    }
    return moves;
}

} // namespace Chess
