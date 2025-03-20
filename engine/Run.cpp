#include "Run.h"
#include "Constants.h"
#include "TTable.h"
#include "UCI.h"
#include "Search.h"
#include "Board.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

namespace Chess {

void initialize() {
    initializeKingAttacks();
    initializeKnightAttacks();
    initializePawnAttacks();
    initBishopAttacks();
    initRookAttacks();
    initSquaresBetween();
    initLine();
    initializeSQLookup();
    initZobrist();
    initNeighborMasks();
}

void Run(const std::string& command, const std::string& position, int depth) {
    initializeEverythingExceptTTable();
    initTransTable(256);
    if (command == "perft") {
        // RunPerfTests(position, depth); // Not implemented in this translation.
    }
    if (command == "search") {
        RunSearch(position, depth);
    }
    if (command == "selfplay") {
        RunSelfPlay(position, depth);
    }
    if (command.find("play") != std::string::npos) {
        std::istringstream iss(command);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token)
            tokens.push_back(token);
        if (tokens.size() >= 2) {
            if (tokens[1] == "white")
                RunPlay(position, depth, WHITE);
            else
                RunPlay(position, depth, BLACK);
        }
    }
}

void RunSearch(const std::string& position, int depth) {
    ChessBoard board;
    if (position == "startpos")
        board.initializeStartingPosition();
    else
        board.initializeFEN(position);
    board.printFromBitBoards();
    for (int i = 1; i <= depth; i++) {
        std::vector<Move> pvLine;
        std::cout << "Depth " << i << ": ";
        auto result = pvs(&board, i, i, -WIN_VALUE - 1, WIN_VALUE + 1, board.sideToMove, true, pvLine, 100000000, std::chrono::steady_clock::now());
        int score = result.first * FACTOR[board.sideToMove];
        bool timeout = result.second;
        if (timeout)
            break;
        std::cout << score << " ";
        std::vector<std::string> pvStr;
        for (const auto& mv : pvLine)
            pvStr.push_back(mv.toUCI());
        std::cout << "[";
        for (size_t j = 0; j < pvStr.size(); j++) {
            std::cout << pvStr[j] << (j < pvStr.size() - 1 ? " " : "");
        }
        std::cout << "]" << std::endl;
        if (score == WIN_VALUE || score == -WIN_VALUE) {
            std::cout << "Found mate." << std::endl;
            break;
        }
    }
}

void RunSelfPlay(const std::string& position, int depth) {
    initializeEverythingExceptTTable();
    initTransTable(256);
    ChessBoard board;
    if (position == "startpos")
        board.initializeStartingPosition();
    else
        board.initializeFEN(position);
    std::vector<std::string> movesPlayed;
    auto legalMoves = board.generateLegalMoves();
    while (!legalMoves.empty()) {
        board.printFromBitBoards();
        int score = 0;
        Move bestMove = searchWithTime(&board, 10000);
        board.printFromBitBoards();
        std::cout << "SCORE: " << (score * FACTOR[board.sideToMove]) << std::endl;
        movesPlayed.push_back(bestMove.toUCI());
        board.makeMove(bestMove);
        std::cout << "Moves played: ";
        for (const auto& m : movesPlayed)
            std::cout << m << " ";
        std::cout << std::endl;
        legalMoves = board.generateLegalMoves();
    }
}

void RunPlay(const std::string& position, int depth, int player) {
    initializeEverythingExceptTTable();
    initTransTable(256);
    ChessBoard board;
    if (position == "startpos")
        board.initializeStartingPosition();
    else
        board.initializeFEN(position);
    std::vector<std::string> movesPlayed;
    auto legalMoves = board.generateLegalMoves();
    while (!legalMoves.empty()) {
        board.printFromBitBoards();
        if (board.sideToMove == player) {
            std::cout << "Your turn: " << std::endl;
            std::string moveStr;
            std::cin >> moveStr;
            board.makeMoveFromUCI(moveStr);
            movesPlayed.push_back(moveStr);
        } else {
            std::vector<Move> pvLine;
            int score = 0;
            for (int i = 1; i <= depth; i++) {
                pvLine.clear();
                std::cout << "Depth " << i << ": ";
                auto result = pvs(&board, i, i, -WIN_VALUE - 1, WIN_VALUE + 1, board.sideToMove, true, pvLine, 100000, std::chrono::steady_clock::now());
                score = result.first;
                if (score == WIN_VALUE || score == -WIN_VALUE) {
                    std::cout << "Found mate." << std::endl;
                    break;
                }
                std::cout << (score * FACTOR[board.sideToMove]) << " ";
                std::vector<std::string> pvStr;
                for (const auto& mv : pvLine)
                    pvStr.push_back(mv.toUCI());
                std::cout << "[";
                for (size_t j = 0; j < pvStr.size(); j++) {
                    std::cout << pvStr[j] << (j < pvStr.size() - 1 ? " " : "");
                }
                std::cout << "]" << std::endl;
            }
            Move bestMove = pvLine[0];
            std::cout << "SCORE: " << (score * FACTOR[board.sideToMove]) << std::endl;
            movesPlayed.push_back(bestMove.toUCI());
            board.makeMove(bestMove);
            std::cout << "Moves played: ";
            for (const auto& m : movesPlayed)
                std::cout << m << " ";
            std::cout << std::endl;
        }
        legalMoves = board.generateLegalMoves();
    }
}

}
