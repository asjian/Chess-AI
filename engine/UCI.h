#ifndef UCI_H
#define UCI_H

#include <string>
#include "Board.h"

namespace Chess {

ChessBoard processPositionCmd(const std::string& cmd);
void processGoCmd(const std::string& cmd, ChessBoard* board);
void uciLoop();

} // namespace Chess

#endif // UCI_H
