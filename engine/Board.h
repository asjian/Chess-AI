#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include "Types.h"

namespace Chess {

struct MoveHistoryEntry {
    Move moveData;
    bool whiteKingsideCastleStatus;
    bool whiteQueensideCastleStatus;
    bool blackKingsideCastleStatus;
    bool blackQueensideCastleStatus;
    Square enPassantStatus;
    uint64_t prevHash;
    bool whiteCastledBefore;
    bool blackCastledBefore;
};

class ChessBoard {
public:
    std::array<uint64_t, 14> pieceBitboards;
    std::array<Piece, 64> squareArray;
    std::array<uint64_t, 2> colorBitboards;
    uint64_t occupiedBB;
    uint64_t emptyBB;
    Color sideToMove;
    Square enPassantSquare;
    bool whiteKingsideCastling;
    bool whiteQueensideCastling;
    bool blackKingsideCastling;
    bool blackQueensideCastling;
    std::vector<MoveHistoryEntry> moveHistory;
    uint64_t zobristHash;
    int halfMoveCount;
    int fullMoveCount;
    bool whiteHasCastled;
    bool blackHasCastled;

    ChessBoard();
    void initializeStartingPosition();
    void initializeFEN(const std::string &fen);
    uint64_t getPiecesByColor(PieceType p, Color c);
    void placePiece(Piece p, Square s, Color c);
    void movePieceTo(Piece p, Square fromSq, Square toSq, Color c);
    void capturePieceAt(Piece p, Piece capturedPiece, Square fromSq, Square toSq, Color c);
    void promotePiece(Piece p, Piece newPiece, Square s);
    void removePieceFrom(Piece p, Square s, Color c);
    void executeMoveFromUCI(const std::string &uci);
    void executeMoveNoUpdate(Move moveData);
    void executeMove(Move moveData);
    void revertMoveNoUpdate(Move previousMove);
    void revertLastMove();
    void executeNullMove();
    void revertNullMove();
    bool isInCheck(Color c);
    bool hasThreefoldRepetition();
    bool hasTwofoldRepetition();
    bool hasInsufficientMaterial();
    void displayBoard();
    void displayBoardFromBitboards();
    std::string getBoardStringFromBitboards();
};

ChessBoard* createChessBoard();

}

#endif 
