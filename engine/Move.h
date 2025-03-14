#ifndef MOVE_H
#define MOVE_H

#include <string>
#include "Constants.h"
#include "Board.h"

namespace Chess {

struct Move {
    Square dest;
    Square src;
    MoveType moveType;
    Piece mPiece;
    Piece capturedPiece;
    Color movedColor;
    Piece promotionPiece;
    bool isNull;

    std::string toUCI() const;
};

Move uciToMove(const std::string& uci, ChessBoard* board);

}

#endif
