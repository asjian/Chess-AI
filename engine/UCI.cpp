#include "UCI.h"
#include "Constants.h"
#include "TTable.h"
#include "Search.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>

namespace Chess {

ChessBoard processPositionCmd(const std::string& cmd) {
    ChessBoard board;
    std::istringstream iss(cmd);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    int tokenCount = tokens.size();
    int moveStart = 2;
    if (tokens[1] != "fen") {
        board.initializeStartingPosition();
        if (tokenCount == 2)
            return board;
    } else {
        std::string fen = tokens[2] + " " + tokens[3] + " " + tokens[4] + " " +
                          tokens[5] + " " + tokens[6] + " " + tokens[7];
        board.initializeFEN(fen);
        if (tokenCount == 8)
            return board;
        moveStart = 8;
    }
    for (size_t i = moveStart + 1; i < tokens.size(); i++) {
        board.makeMoveFromUCI(tokens[i]);
    }
    return board;
}

void processGoCmd(const std::string& cmd, ChessBoard* board) {
    std::istringstream iss(cmd);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    int64_t whiteTime = 0, blackTime = 0, whiteInc = 0, blackInc = 0;
    int64_t moveTime = 30000;
    bool moveTimeSet = false;
    for (size_t i = 0; i < words.size(); i++) {
        if (words[i] == "movetime") {
            moveTimeSet = true;
            moveTime = std::stoll(words[i + 1]);
        } else if (words[i] == "wtime") {
            whiteTime = std::stoll(words[i + 1]);
        } else if (words[i] == "winc") {
            whiteInc = std::stoll(words[i + 1]);
        } else if (words[i] == "btime") {
            blackTime = std::stoll(words[i + 1]);
        } else if (words[i] == "binc") {
            blackInc = std::stoll(words[i + 1]);
        }
    }
    Move bestMove;
    if (moveTimeSet) {
        bestMove = searchWithTime(board, moveTime);
    } else {
        if (board->sideToMove == WHITE) {
            if (whiteInc >= 0)
                moveTime = whiteTime / 25 + whiteInc - 200;
            else
                moveTime = whiteTime / 30;
            bestMove = searchWithTime(board, moveTime);
        } else {
            if (blackInc >= 0)
                moveTime = blackTime / 25 + blackInc - 200;
            else
                moveTime = blackTime / 30;
            bestMove = searchWithTime(board, moveTime);
        }
    }
    std::cout << "bestmove " << bestMove.toUCI() << std::endl;
    if (board->plyCnt % 10 == 0)
        clearTransTable();
}

void uciLoop() {
    int64_t ttSize = 256;
    ChessBoard board;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "uci") {
            initializeEverythingExceptTTable();
            std::cout << "id name Maelstrom" << std::endl;
            std::cout << "id author saisree27" << std::endl;
            std::cout << "option name Hash type spin default 256 min 1 max 1024" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        if (line == "isready") {
            initTransTable(static_cast<int>(ttSize));
            std::cout << "readyok" << std::endl;
        }
        if (line == "ucinewgame") {
            board = ChessBoard();
            board.initializeStartingPosition();
        }
        if (line == "quit") {
            std::exit(0);
        }
        if (line.find("position") != std::string::npos) {
            board = processPositionCmd(line);
        }
        if (line.find("go") != std::string::npos) {
            processGoCmd(line, &board);
        }
        if (line.find("setoption") != std::string::npos) {
            std::istringstream iss(line);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) {
                parts.push_back(part);
            }
            ttSize = std::stoll(parts.back());
        }
    }
}

} // namespace Chess
