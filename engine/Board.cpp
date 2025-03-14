#include "Board.h"
#include "Bitboard.h"
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace Chess {

ChessBoard::ChessBoard() {
    pieceBitboards.fill(0);
    squareArray.fill(EMPTY);
    colorBitboards.fill(0);
    occupiedBB = 0;
    emptyBB = ~0ULL;
    sideToMove = WHITE;
    enPassantSquare = EMPTYSQ;
    whiteKingsideCastling = true;
    whiteQueensideCastling = true;
    blackKingsideCastling = true;
    blackQueensideCastling = true;
    moveHistory.clear();
    zobristHash = 0;
    halfMoveCount = 0;
    fullMoveCount = 0;
    whiteHasCastled = false;
    blackHasCastled = false;
    initializeStartingPosition();
}

void ChessBoard::initializeStartingPosition() {
    zobristHash = 0;
    squareArray = {
        wR, wN, wB, wQ, wK, wB, wN, wR,
        wP, wP, wP, wP, wP, wP, wP, wP,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        bP, bP, bP, bP, bP, bP, bP, bP,
        bR, bN, bB, bQ, bK, bB, bN, bR
    };
    for (int i = 0; i < 64; i++) {
        if (i < 16)
            placePiece(squareArray[i], Square(i), WHITE);
        else if (i >= 48)
            placePiece(squareArray[i], Square(i), BLACK);
        else
            emptyBB ^= sToBB[i];
    }
    sideToMove = WHITE;
    enPassantSquare = EMPTYSQ;
    whiteKingsideCastling = true;
    whiteQueensideCastling = true;
    blackKingsideCastling = true;
    blackQueensideCastling = true;
    zobristHash ^= turnHash;
}

void ChessBoard::initializeFEN(const std::string &fen) {
    zobristHash = 0;
    for (Square s = a1; s <= h8; s = Square(s + 1))
        placePiece(EMPTY, s, WHITE);
    std::istringstream iss(fen);
    std::string piecesStr, turnStr, castling, enpass, halfMoveClock, fullMoveCountStr;
    iss >> piecesStr >> turnStr >> castling >> enpass >> halfMoveClock >> fullMoveCountStr;
    std::vector<std::string> ranks;
    std::istringstream rankStream(piecesStr);
    std::string rankStr;
    while (std::getline(rankStream, rankStr, '/'))
        ranks.push_back(rankStr);
    for (size_t i = 0; i < ranks.size(); i++) {
        int rank = 8 * (7 - i);
        Square sq(rank);
        for (char ch : ranks[i]) {
            std::string s(1, ch);
            if (std::string("PBNRQKpbnrqk").find(ch) != std::string::npos) {
                Piece piece = stringToPieceMap[s];
                placePiece(piece, sq, piece.getColor());
                sq = Square(int(sq) + 1);
            } else {
                int num = std::atoi(s.c_str());
                for (int k = 0; k < num; k++) {
                    Square newSq = Square(int(sq) + k);
                    placePiece(EMPTY, newSq, WHITE);
                }
                sq = Square(int(sq) + num);
            }
        }
    }
    if (turnStr == "w")
        sideToMove = WHITE;
    else
        sideToMove = BLACK;
    if (castling.find("K") != std::string::npos)
        whiteKingsideCastling = true;
    if (castling.find("Q") != std::string::npos)
        whiteQueensideCastling = true;
    if (castling.find("k") != std::string::npos)
        blackKingsideCastling = true;
    if (castling.find("q") != std::string::npos)
        blackQueensideCastling = true;
    if (enpass == "-")
        enPassantSquare = EMPTYSQ;
    else
        enPassantSquare = stringToSquareMap[enpass];
    halfMoveCount = std::atoi(halfMoveClock.c_str());
    fullMoveCount = std::atoi(fullMoveCountStr.c_str());
    zobristHash ^= turnHash;
    halfMoveCount = fullMoveCount * 2;
}

uint64_t ChessBoard::getPiecesByColor(PieceType p, Color c) {
    return pieceBitboards[int(p) + int(c) * colorIndexOffset];
}

void ChessBoard::placePiece(Piece p, Square s, Color c) {
    squareArray[s] = p;
    if (p != EMPTY) {
        uint64_t sqBit = sToBB[s];
        colorBitboards[c] |= sqBit;
        pieceBitboards[int(p)] |= sqBit;
        occupiedBB |= sqBit;
        emptyBB &= ~sqBit;
        zobristHash ^= zobristTable[int(p)][s];
    } else {
        uint64_t sqBit = sToBB[s];
        emptyBB |= sqBit;
        occupiedBB &= ~sqBit;
        zobristHash ^= zobristTable[int(p)][s];
    }
}

void ChessBoard::movePieceTo(Piece p, Square fromSq, Square toSq, Color c) {
    uint64_t fromBit = sToBB[fromSq];
    uint64_t toBit = sToBB[toSq];
    uint64_t deltaBit = fromBit ^ toBit;
    pieceBitboards[int(p)] ^= deltaBit;
    colorBitboards[c] ^= deltaBit;
    occupiedBB ^= deltaBit;
    emptyBB ^= deltaBit;
    squareArray[fromSq] = EMPTY;
    squareArray[toSq] = p;
    zobristHash ^= zobristTable[int(p)][fromSq] ^ zobristTable[int(p)][toSq] ^ zobristTable[int(EMPTY)][toSq] ^ zobristTable[int(EMPTY)][fromSq];
}

void ChessBoard::capturePieceAt(Piece p, Piece capturedPiece, Square fromSq, Square toSq, Color c) {
    uint64_t fromBit = sToBB[fromSq];
    uint64_t toBit = sToBB[toSq];
    uint64_t deltaBit = fromBit ^ toBit;
    pieceBitboards[int(p)] ^= deltaBit;
    colorBitboards[c] ^= deltaBit;
    pieceBitboards[int(capturedPiece)] ^= toBit;
    colorBitboards[reverseColor(c)] ^= toBit;
    occupiedBB ^= fromBit;
    emptyBB ^= fromBit;
    squareArray[fromSq] = EMPTY;
    squareArray[toSq] = p;
    zobristHash ^= zobristTable[int(p)][fromSq] ^ zobristTable[int(p)][toSq] ^ zobristTable[int(capturedPiece)][toSq] ^ zobristTable[int(EMPTY)][fromSq];
}

void ChessBoard::promotePiece(Piece p, Piece newPiece, Square s) {
    uint64_t sqBit = sToBB[s];
    pieceBitboards[int(p)] ^= sqBit;
    pieceBitboards[int(newPiece)] ^= sqBit;
    squareArray[s] = newPiece;
    zobristHash ^= zobristTable[int(p)][s] ^ zobristTable[int(newPiece)][s];
}

void ChessBoard::removePieceFrom(Piece p, Square s, Color c) {
    uint64_t sqBit = sToBB[s];
    pieceBitboards[int(p)] ^= sqBit;
    occupiedBB ^= sqBit;
    emptyBB ^= sqBit;
    colorBitboards[c] ^= sqBit;
    squareArray[s] = EMPTY;
    zobristHash ^= zobristTable[int(p)][s];
}

void ChessBoard::executeMoveFromUCI(const std::string &uci) {
    executeMove(fromUCI(uci, *this));
}

void ChessBoard::executeMoveNoUpdate(Move moveData) {
    switch (moveData.movetype) {
        case QUIET:
            movePieceTo(moveData.piece, moveData.from, moveData.to, moveData.colorMoved);
            break;
        case CAPTURE:
            capturePieceAt(moveData.piece, moveData.captured, moveData.from, moveData.to, moveData.colorMoved);
            break;
        case PROMOTION:
            movePieceTo(moveData.piece, moveData.from, moveData.to, moveData.colorMoved);
            promotePiece(moveData.piece, moveData.promote, moveData.to);
            break;
        case CAPTUREANDPROMOTION:
            capturePieceAt(moveData.piece, moveData.captured, moveData.from, moveData.to, moveData.colorMoved);
            promotePiece(moveData.piece, moveData.promote, moveData.to);
            break;
        case KCASTLE:
            if (moveData.colorMoved == WHITE) {
                movePieceTo(wK, e1, g1, WHITE);
                movePieceTo(wR, h1, f1, WHITE);
            } else {
                movePieceTo(bK, e8, g8, BLACK);
                movePieceTo(bR, h8, f8, BLACK);
            }
            break;
        case QCASTLE:
            if (moveData.colorMoved == WHITE) {
                movePieceTo(wK, e1, c1, WHITE);
                movePieceTo(wR, a1, d1, WHITE);
            } else {
                movePieceTo(bK, e8, c8, BLACK);
                movePieceTo(bR, a8, d8, BLACK);
            }
            break;
        case ENPASSANT:
            movePieceTo(moveData.piece, moveData.from, moveData.to, moveData.colorMoved);
            if (moveData.colorMoved == WHITE)
                removePieceFrom(bP, moveData.to.goDirection(SOUTH), BLACK);
            else
                removePieceFrom(wP, moveData.to.goDirection(NORTH), WHITE);
            break;
    }
}

void ChessBoard::executeMove(Move moveData) {
    MoveHistoryEntry historyEntry;
    historyEntry.moveData = moveData;
    historyEntry.whiteKingsideCastleStatus = whiteKingsideCastling;
    historyEntry.whiteQueensideCastleStatus = whiteQueensideCastling;
    historyEntry.blackKingsideCastleStatus = blackKingsideCastling;
    historyEntry.blackQueensideCastleStatus = blackQueensideCastling;
    historyEntry.enPassantStatus = enPassantSquare;
    historyEntry.prevHash = zobristHash;
    historyEntry.whiteCastledBefore = whiteHasCastled;
    historyEntry.blackCastledBefore = blackHasCastled;
    moveHistory.push_back(historyEntry);
    if (!moveData.null) {
        switch (moveData.movetype) {
            case QUIET:
                movePieceTo(moveData.piece, moveData.from, moveData.to, moveData.colorMoved);
                break;
            case CAPTURE:
                capturePieceAt(moveData.piece, moveData.captured, moveData.from, moveData.to, moveData.colorMoved);
                break;
            case PROMOTION:
                movePieceTo(moveData.piece, moveData.from, moveData.to, moveData.colorMoved);
                promotePiece(moveData.piece, moveData.promote, moveData.to);
                break;
            case CAPTUREANDPROMOTION:
                capturePieceAt(moveData.piece, moveData.captured, moveData.from, moveData.to, moveData.colorMoved);
                promotePiece(moveData.piece, moveData.promote, moveData.to);
                break;
            case KCASTLE:
                if (moveData.colorMoved == WHITE) {
                    movePieceTo(wK, e1, g1, WHITE);
                    movePieceTo(wR, h1, f1, WHITE);
                    whiteHasCastled = true;
                } else {
                    movePieceTo(bK, e8, g8, BLACK);
                    movePieceTo(bR, h8, f8, BLACK);
                    blackHasCastled = true;
                }
                break;
            case QCASTLE:
                if (moveData.colorMoved == WHITE) {
                    movePieceTo(wK, e1, c1, WHITE);
                    movePieceTo(wR, a1, d1, WHITE);
                    whiteHasCastled = true;
                } else {
                    movePieceTo(bK, e8, c8, BLACK);
                    movePieceTo(bR, a8, d8, BLACK);
                    blackHasCastled = true;
                }
                break;
            case ENPASSANT:
                movePieceTo(moveData.piece, moveData.from, moveData.to, moveData.colorMoved);
                if (moveData.colorMoved == WHITE)
                    removePieceFrom(bP, moveData.to.goDirection(SOUTH), BLACK);
                else
                    removePieceFrom(wP, moveData.to.goDirection(NORTH), WHITE);
                break;
        }
    }
    sideToMove = reverseColor(sideToMove);
    if (moveData.piece == wK) {
        whiteKingsideCastling = false;
        whiteQueensideCastling = false;
    }
    if (moveData.piece == bK) {
        blackKingsideCastling = false;
        blackQueensideCastling = false;
    }
    if (moveData.piece == wR) {
        if (moveData.from == a1)
            whiteQueensideCastling = false;
        if (moveData.from == h1)
            whiteKingsideCastling = false;
    }
    if (moveData.piece == bR) {
        if (moveData.from == a8)
            blackQueensideCastling = false;
        if (moveData.from == h8)
            blackKingsideCastling = false;
    }
    int distance = int(moveData.to) - int(moveData.from);
    if (distance == 2 * NORTH && moveData.piece == wP)
        enPassantSquare = Square(int(moveData.from) + NORTH);
    else if (distance == 2 * SOUTH && moveData.piece == bP)
        enPassantSquare = Square(int(moveData.from) + SOUTH);
    else
        enPassantSquare = EMPTYSQ;
    halfMoveCount++;
    zobristHash ^= turnHash;
}

void ChessBoard::revertMoveNoUpdate(Move previousMove) {
    switch (previousMove.movetype) {
        case QUIET:
            movePieceTo(previousMove.piece, previousMove.to, previousMove.from, previousMove.colorMoved);
            break;
        case CAPTURE:
            movePieceTo(previousMove.piece, previousMove.to, previousMove.from, previousMove.colorMoved);
            placePiece(previousMove.captured, previousMove.to, reverseColor(previousMove.colorMoved));
            break;
        case PROMOTION:
            removePieceFrom(previousMove.promote, previousMove.to, previousMove.colorMoved);
            placePiece(previousMove.piece, previousMove.from, previousMove.colorMoved);
            break;
        case CAPTUREANDPROMOTION:
            removePieceFrom(previousMove.promote, previousMove.to, previousMove.colorMoved);
            placePiece(previousMove.piece, previousMove.from, previousMove.colorMoved);
            placePiece(previousMove.captured, previousMove.to, reverseColor(previousMove.colorMoved));
            break;
        case KCASTLE:
            if (previousMove.colorMoved == WHITE) {
                movePieceTo(wK, g1, e1, WHITE);
                movePieceTo(wR, f1, h1, WHITE);
            } else {
                movePieceTo(bK, g8, e8, BLACK);
                movePieceTo(bR, f8, h8, BLACK);
            }
            break;
        case QCASTLE:
            if (previousMove.colorMoved == WHITE) {
                movePieceTo(wK, c1, e1, WHITE);
                movePieceTo(wR, d1, a1, WHITE);
            } else {
                movePieceTo(bK, c8, e8, BLACK);
                movePieceTo(bR, d8, a8, BLACK);
            }
            break;
        case ENPASSANT:
            movePieceTo(previousMove.piece, previousMove.to, previousMove.from, previousMove.colorMoved);
            if (previousMove.colorMoved == WHITE)
                placePiece(bP, previousMove.to.goDirection(SOUTH), BLACK);
            else
                placePiece(wP, previousMove.to.goDirection(NORTH), WHITE);
            break;
    }
}

void ChessBoard::revertLastMove() {
    MoveHistoryEntry lastEntry = moveHistory.back();
    Move previousMove = lastEntry.moveData;
    if (!previousMove.null) {
        switch (previousMove.movetype) {
            case QUIET:
                movePieceTo(previousMove.piece, previousMove.to, previousMove.from, previousMove.colorMoved);
                break;
            case CAPTURE:
                movePieceTo(previousMove.piece, previousMove.to, previousMove.from, previousMove.colorMoved);
                placePiece(previousMove.captured, previousMove.to, reverseColor(previousMove.colorMoved));
                break;
            case PROMOTION:
                removePieceFrom(previousMove.promote, previousMove.to, previousMove.colorMoved);
                placePiece(previousMove.piece, previousMove.from, previousMove.colorMoved);
                break;
            case CAPTUREANDPROMOTION:
                removePieceFrom(previousMove.promote, previousMove.to, previousMove.colorMoved);
                placePiece(previousMove.piece, previousMove.from, previousMove.colorMoved);
                placePiece(previousMove.captured, previousMove.to, reverseColor(previousMove.colorMoved));
                break;
            case KCASTLE:
                if (previousMove.colorMoved == WHITE) {
                    movePieceTo(wK, g1, e1, WHITE);
                    movePieceTo(wR, f1, h1, WHITE);
                } else {
                    movePieceTo(bK, g8, e8, BLACK);
                    movePieceTo(bR, f8, h8, BLACK);
                }
                break;
            case QCASTLE:
                if (previousMove.colorMoved == WHITE) {
                    movePieceTo(wK, c1, e1, WHITE);
                    movePieceTo(wR, d1, a1, WHITE);
                } else {
                    movePieceTo(bK, c8, e8, BLACK);
                    movePieceTo(bR, d8, a8, BLACK);
                }
                break;
            case ENPASSANT:
                movePieceTo(previousMove.piece, previousMove.to, previousMove.from, previousMove.colorMoved);
                if (previousMove.colorMoved == WHITE)
                    placePiece(bP, previousMove.to.goDirection(SOUTH), BLACK);
                else
                    placePiece(wP, previousMove.to.goDirection(NORTH), WHITE);
                break;
        }
    }
    whiteKingsideCastling = lastEntry.whiteKingsideCastleStatus;
    whiteQueensideCastling = lastEntry.whiteQueensideCastleStatus;
    blackKingsideCastling = lastEntry.blackKingsideCastleStatus;
    blackQueensideCastling = lastEntry.blackQueensideCastleStatus;
    enPassantSquare = lastEntry.enPassantStatus;
    zobristHash = lastEntry.prevHash;
    sideToMove = reverseColor(sideToMove);
    whiteHasCastled = lastEntry.whiteCastledBefore;
    blackHasCastled = lastEntry.blackCastledBefore;
    moveHistory.pop_back();
    halfMoveCount--;
}

void ChessBoard::executeNullMove() {
    executeMove(Move{true});
}

void ChessBoard::revertNullMove() {
    revertLastMove();
}

bool ChessBoard::isInCheck(Color c) {
    uint64_t attackerBB = 0;
    Square kingSq = Square(scanForward(getPiecesByColor(king, c)));
    uint64_t orthoOpp = getPiecesByColor(rook, reverseColor(c)) | getPiecesByColor(queen, reverseColor(c));
    uint64_t diagOpp = getPiecesByColor(bishop, reverseColor(c)) | getPiecesByColor(queen, reverseColor(c));
    if (c == WHITE)
        attackerBB |= (whitePawnAttacksSquareLookup[kingSq] & pieceBitboards[bP]);
    else
        attackerBB |= (blackPawnAttacksSquareLookup[kingSq] & pieceBitboards[wP]);
    if (attackerBB != 0)
        return true;
    attackerBB |= (knightAttacks(kingSq) & getPiecesByColor(knight, reverseColor(c)));
    if (attackerBB != 0)
        return true;
    uint64_t potentialPins = (getBishopAttacks(kingSq, colorBitboards[reverseColor(c)]) & diagOpp);
    potentialPins |= (getRookAttacks(kingSq, colorBitboards[reverseColor(c)]) & orthoOpp);
    int lsb;
    uint64_t betweenPieces;
    while (potentialPins != 0) {
        lsb = popLeastSignificantBit(&potentialPins);
        betweenPieces = squaresBetween[kingSq][lsb] & colorBitboards[c];
        if (betweenPieces == 0) {
            attackerBB ^= sToBB[lsb];
            return true;
        }
    }
    return attackerBB != 0;
}

bool ChessBoard::hasThreefoldRepetition() {
    uint64_t currentHash = zobristHash;
    int repetitions = 0;
    for (int i = moveHistory.size() - 1; i >= 0; i--) {
        MoveHistoryEntry entry = moveHistory[i];
        if (entry.prevHash == currentHash && !entry.moveData.null)
            repetitions++;
        if (repetitions >= 2)
            break;
    }
    return repetitions >= 2;
}

bool ChessBoard::hasTwofoldRepetition() {
    uint64_t currentHash = zobristHash;
    int repetitions = 1;
    for (int i = moveHistory.size() - 1; i >= 0; i--) {
        MoveHistoryEntry entry = moveHistory[i];
        if (entry.prevHash == currentHash && !entry.moveData.null)
            repetitions++;
        if (repetitions >= 2)
            break;
    }
    return repetitions >= 2;
}

bool ChessBoard::hasInsufficientMaterial() {
    int totalPieces = countBits(occupiedBB);
    if (totalPieces == 2)
        return true;
    if (totalPieces == 3 && (pieceBitboards[wN] != 0 || pieceBitboards[bN] != 0 || pieceBitboards[wB] != 0 || pieceBitboards[bB] != 0))
        return true;
    if (totalPieces == 4) {
        if (pieceBitboards[wN] != 0) {
            int cnt = countBits(pieceBitboards[wN]);
            if (cnt == 2)
                return true;
        }
        if (pieceBitboards[bN] != 0) {
            int cnt = countBits(pieceBitboards[bN]);
            if (cnt == 2)
                return true;
        }
    }
    return false;
}

void ChessBoard::displayBoard() {
    std::string out = "\n";
    for (int i = 56; i >= 0; i -= 8) {
        for (int j = 0; j < 8; j++)
            out += squareArray[i+j].toString() + " ";
        out += "\n";
    }
    std::cout << out;
}

void ChessBoard::displayBoardFromBitboards() {
    std::string out = "   +---+---+---+---+---+---+---+---+\n";
    for (int i = 56; i >= 0; i -= 8) {
        out += " " + std::to_string(i/8+1) + " ";
        for (int j = 0; j < 8; j++) {
            if (occupiedBB & (uint64_t(1) << (i+j))) {
                bool found = false;
                for (int k = 0; k < 14; k++) {
                    if (pieceBitboards[k] & (uint64_t(1) << (i+j))) {
                        if (found)
                            std::cout << "Duplicate pieces..." << std::endl;
                        else {
                            found = true;
                            out += "| " + Piece(k).toString() + " ";
                        }
                    }
                }
                if (!found)
                    std::cout << "Piece is in occupied bitboard not present in any of the pieces bitboard..." << std::endl;
            } else if (emptyBB & (uint64_t(1) << (i+j)))
                out += "| " + EMPTY.toString() + " ";
            else
                std::cout << "Square is not represented in either occupied or empty..." << std::endl;
        }
        out += "| " + "\n";
        out += "   +---+---+---+---+---+---+---+---+\n";
    }
    out += "     A   B   C   D   E   F   G   H\n";
    std::cout << out;
}

std::string ChessBoard::getBoardStringFromBitboards() {
    std::string out = "\n";
    for (int i = 56; i >= 0; i -= 8) {
        for (int j = 0; j < 8; j++) {
            if (occupiedBB & (uint64_t(1) << (i+j))) {
                bool found = false;
                for (int k = 0; k < 14; k++) {
                    if (pieceBitboards[k] & (uint64_t(1) << (i+j))) {
                        if (found)
                            std::cout << "Duplicate pieces..." << std::endl;
                        else {
                            found = true;
                            out += Piece(k).toString() + " ";
                        }
                    }
                }
                if (!found)
                    std::cout << "Piece is in occupied bitboard not present in any of the pieces bitboard..." << std::endl;
            } else if (emptyBB & (uint64_t(1) << (i+j)))
                out += EMPTY.toString() + " ";
            else
                std::cout << "Square is not represented in either occupied or empty..." << std::endl;
        }
        out += "\n";
    }
    return out;
}

ChessBoard* createChessBoard() {
    ChessBoard* boardPtr = new ChessBoard();
    return boardPtr;
}
}
