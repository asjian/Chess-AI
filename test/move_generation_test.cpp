#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include "Board.h"
#include "Evaluation.h"
#include "Constants.h"
#include "Move.h"

// Helper function to compare two vectors of strings (ignoring order)
bool checkSameElements(const std::vector<std::string>& a, const std::vector<std::string>& b) {
    if(a.size() != b.size()) return false;
    std::vector<std::string> a_sorted = a;
    std::vector<std::string> b_sorted = b;
    std::sort(a_sorted.begin(), a_sorted.end());
    std::sort(b_sorted.begin(), b_sorted.end());
    return a_sorted == b_sorted;
}

void testStartPos() {
    ChessBoard board;
    board.initializeStartingPosition();
    bool gotCastling = board.oo && board.ooo && board.OO && board.OOO;
    if (!gotCastling) {
        std::cerr << "TestStartPos (castlingRights): got false, wanted true" << std::endl;
        assert(false);
    }
    Square expectedEP = EMPTYSQ;
    if (board.enpassant != expectedEP) {
        std::cerr << "TestStartPos (enpassant): got " << SQUARE_TO_STRING.at(board.enpassant)
                  << ", wanted " << SQUARE_TO_STRING.at(expectedEP) << std::endl;
        assert(false);
    }
}

void testEnPassantPseudoPin() {
    ChessBoard board;
    board.initializeFEN("rnbq1bnr/ppp1pppp/8/8/k2p3R/8/PPPPPPPP/RNBQKBN1 w - - 0 1");
    board.makeMoveFromUCI("e2e4");
    if (board.enpassant != e3) {
        std::cerr << "TestEnPassantPseudoPin (square): got " << SQUARE_TO_STRING.at(board.enpassant)
                  << ", wanted " << SQUARE_TO_STRING.at(e3) << std::endl;
        assert(false);
    }
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves) {
        uciMoves.push_back(move.toUCI());
    }
    bool found = std::any_of(uciMoves.begin(), uciMoves.end(), [](const std::string& s) {
        return s == "d4e3";
    });
    if (found) {
        std::cerr << "TestEnPassantPseudoPin (enpassant move): got true, wanted false" << std::endl;
        assert(false);
    }
}

void testTwoEnPassant() {
    std::string fen = "7k/8/8/8/pPp5/8/8/7K b - b3 0 1";
    ChessBoard board;
    board.initializeFEN(fen);
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves) {
        uciMoves.push_back(move.toUCI());
    }
    std::vector<std::string> expectedMoves = {
        "a4a3", "c4c3", "a4b3", "c4b3", "h8h7", "h8g7", "h8g8"
    };
    if (!checkSameElements(expectedMoves, uciMoves)) {
        std::cerr << "TestTwoEnPassant: got ";
        for (const auto& s : uciMoves) std::cerr << s << " ";
        std::cerr << ", wanted ";
        for (const auto& s : expectedMoves) std::cerr << s << " ";
        std::cerr << std::endl;
        assert(false);
    }
}

void testTwoEnpassantOneLegal() {
    std::string fen = "8/8/4k3/8/2pPp3/8/B7/7K b - d3 0 1";
    ChessBoard board;
    board.initializeFEN(fen);
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves) {
        uciMoves.push_back(move.toUCI());
    }
    std::vector<std::string> expectedMoves = {
        "e4e3", "e4d3", "e6d5", "e6f5", "e6d6", "e6f6", "e6d7", "e6e7", "e6f7"
    };
    if (!checkSameElements(expectedMoves, uciMoves)) {
        std::cerr << "TestTwoEnpassantOneLegal: got ";
        for (const auto& s : uciMoves) std::cerr << s << " ";
        std::cerr << ", wanted ";
        for (const auto& s : expectedMoves) std::cerr << s << " ";
        std::cerr << std::endl;
        assert(false);
    }
}

void testNoPawnMoves() {
    std::string fen = "8/4k3/1p1p1p1p/pPpPpPpP/P1P1P1P1/8/5K2/8 w - - 0 1";
    ChessBoard board;
    board.initializeFEN(fen);
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves) {
        uciMoves.push_back(move.toUCI());
    }
    std::vector<std::string> expectedMoves = {
        "f2e3", "f2f3", "f2g3", "f2e2", "f2g2", "f2e1", "f2f1", "f2g1"
    };
    if (!checkSameElements(expectedMoves, uciMoves)) {
        std::cerr << "TestNoPawnMoves: got ";
        for (const auto& s : uciMoves) std::cerr << s << " ";
        std::cerr << ", wanted ";
        for (const auto& s : expectedMoves) std::cerr << s << " ";
        std::cerr << std::endl;
        assert(false);
    }
}

void testCastlingIfSquaresAttacked() {
    ChessBoard board;
    board.initializeFEN("rnbq1rk1/pppp1ppp/5n2/2b1p3/2B1P3/5P2/PPPPN1PP/RNBQK2R w KQ - 5 5");
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves) {
        uciMoves.push_back(move.toUCI());
    }
    bool found = std::any_of(uciMoves.begin(), uciMoves.end(), [](const std::string& s) {
        return s == "e1g1";
    });
    if (found) {
        std::cerr << "TestCastlingIfSquaresAttacked: got true, wanted false" << std::endl;
        assert(false);
    }
}

void testAllMoves() {
    std::string fen = "r3r1k1/pp3pbp/1qp1b1p1/2B5/2BP4/Q1n2N2/P4PPP/3R1K1R w - - 4 18";
    ChessBoard board;
    board.initializeFEN(fen);
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves)
        uciMoves.push_back(move.toUCI());
    std::vector<std::string> expectedMoves = {
        "d1c1", "d1b1", "d1a1", "d1e1", "d1d2", "d1d3", "h1g1",
        "f1e1", "f1g1", "g2g3", "g2g4", "h2h3", "h2h4", "a3b2",
        "a3c1", "a3b3", "a3c3", "a3a4", "a3a5", "a3a6", "a3a7",
        "a3b4", "f3e1", "f3g1", "f3d2", "f3h4", "f3e5", "f3g5",
        "c4b3", "c4d3", "c4e2", "c4b5", "c4d5", "c4a6", "c4e6",
        "d4d5", "c5b4", "c5b6", "c5d6", "c5e7", "c5f8"
    };
    if (!checkSameElements(expectedMoves, uciMoves)) {
        std::cerr << "TestAllMoves: got ";
        for (const auto& s : uciMoves) std::cerr << s << " ";
        std::cerr << ", wanted ";
        for (const auto& s : expectedMoves) std::cerr << s << " ";
        std::cerr << std::endl;
        assert(false);
    }
}

void testAllMovesHuge() {
    std::string fen = "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1";
    ChessBoard board;
    board.initializeFEN(fen);
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves)
        uciMoves.push_back(move.toUCI());
    int expectedCount = 218;
    if (static_cast<int>(uciMoves.size()) != expectedCount) {
        std::cerr << "TestAllMovesHuge: got " << uciMoves.size() << ", wanted " << expectedCount << std::endl;
        assert(false);
    }
}

void testPromotion() {
    ChessBoard board;
    board.initializeFEN("3k4/8/1K6/8/8/8/pppppppp/RRRRRRRR b - - 0 1");
    auto moves = board.generateLegalMoves();
    std::vector<std::string> uciMoves;
    for (const auto& move : moves)
        uciMoves.push_back(move.toUCI());
    std::vector<std::string> expectedMoves = {
        "a2b1q", "a2b1r", "a2b1n", "a2b1b",
        "b2a1q", "b2a1r", "b2a1n", "b2a1b",
        "b2c1q", "b2c1r", "b2c1n", "b2c1b",
        "c2b1q", "c2b1r", "c2b1n", "c2b1b",
        "c2d1q", "c2d1r", "c2d1n", "c2d1b",
        "e2d1q", "e2d1r", "e2d1n", "e2d1b",
        "e2f1q", "e2f1r", "e2f1n", "e2f1b",
        "f2e1q", "f2e1r", "f2e1n", "f2e1b",
        "f2g1q", "f2g1r", "f2g1n", "f2g1b",
        "g2f1q", "g2f1r", "g2f1n", "g2f1b",
        "g2h1q", "g2h1r", "g2h1n", "g2h1b",
        "h2g1q", "h2g1r", "h2g1n", "h2g1b",
        "d8c8", "d8d7", "d8e7", "d8e8"
    };
    if (!checkSameElements(expectedMoves, uciMoves)) {
        std::cerr << "TestPromotion: got ";
        for (const auto& s : uciMoves) std::cerr << s << " ";
        std::cerr << ", wanted ";
        for (const auto& s : expectedMoves) std::cerr << s << " ";
        std::cerr << std::endl;
        assert(false);
    }
}

void testMakeAndUndoMovePromotion() {
    ChessBoard board;
    board.initializeFEN("3k4/8/1K6/8/8/8/pppppppp/RRRRRRRR b - - 0 1");
    std::string orig = board.getStringFromBitBoards();
    board.makeMoveFromUCI("b2a1q");
    size_t historyLen = board.history.size();
    if (board.squareArray[a1] != bQ) {
        std::cerr << "TestMakeAndUndoMovePromotion (promotion->queen): got " << board.squareArray[a1].toString() 
                  << ", wanted " << bQ.toString() << std::endl;
        assert(false);
    }
    if (board.squareArray[b2] != EMPTY) {
        std::cerr << "TestMakeAndUndoMovePromotion (promotion->pawn): got " << board.squareArray[b2].toString() 
                  << ", wanted " << EMPTY.toString() << std::endl;
        assert(false);
    }
    board.undo();
    size_t newHistoryLen = board.history.size();
    if (board.squareArray[a1] != wR) {
        std::cerr << "TestMakeAndUndoMovePromotion (undo->rook): got " << board.squareArray[a1].toString() 
                  << ", wanted " << wR.toString() << std::endl;
        assert(false);
    }
    if (board.squareArray[b2] != bP) {
        std::cerr << "TestMakeAndUndoMovePromotion (undo->pawn): got " << board.squareArray[b2].toString() 
                  << ", wanted " << bP.toString() << std::endl;
        assert(false);
    }
    if (historyLen - newHistoryLen != 1) {
        std::cerr << "TestMakeAndUndoMovePromotion (history): got " << (historyLen - newHistoryLen) 
                  << ", wanted " << 1 << std::endl;
        assert(false);
    }
    std::string nowStr = board.getStringFromBitBoards();
    if (orig != nowStr) {
        std::cerr << "TestMakeAndUndoMovePromotion (bitboard): got " << nowStr 
                  << ", wanted " << orig << std::endl;
        assert(false);
    }
}

void testMakeAndUndoEnPassant() {
    std::string fen = "7k/8/8/8/pPp5/8/8/7K b - b3 0 1";
    ChessBoard board;
    board.initializeFEN(fen);
    std::string orig = board.getStringFromBitBoards();
    std::string stats = board.getStatsString(); // Assume board provides a method for stats
    board.makeMoveFromUCI("a4b3");
    if (board.squareArray[b3] != bP) {
        std::cerr << "TestMakeAndUndoEnPassant (e.p.->ourpawn): got " << board.squareArray[b3].toString() 
                  << ", wanted " << bP.toString() << std::endl;
        assert(false);
    }
    if (board.squareArray[b4] != EMPTY) {
        std::cerr << "TestMakeAndUndoEnPassant (e.p.->theirpawn): got " << board.squareArray[b4].toString() 
                  << ", wanted " << EMPTY.toString() << std::endl;
        assert(false);
    }
    board.undo();
    if (board.squareArray[b3] != EMPTY) {
        std::cerr << "TestMakeAndUndoEnPassant (after e.p.->ourpawn): got " << board.squareArray[b3].toString() 
                  << ", wanted " << EMPTY.toString() << std::endl;
        assert(false);
    }
    if (board.squareArray[b4] != wP) {
        std::cerr << "TestMakeAndUndoEnPassant (after e.p.->theirpawn): got " << board.squareArray[b4].toString() 
                  << ", wanted " << wP.toString() << std::endl;
        assert(false);
    }
    if (board.squareArray[a4] != bP) {
        std::cerr << "TestMakeAndUndoEnPassant (after e.p.->ourpawn back): got " << board.squareArray[a4].toString() 
                  << ", wanted " << bP.toString() << std::endl;
        assert(false);
    }
    std::string newStr = board.getStringFromBitBoards();
    std::string newStats = board.getStatsString();
    if (orig != newStr) {
        std::cerr << "TestMakeAndUndoEnPassant (bitboard): got " << newStr 
                  << ", wanted " << orig << std::endl;
        assert(false);
    }
    if (stats != newStats) {
        std::cerr << "TestMakeAndUndoEnPassant (stats): got " << newStats 
                  << ", wanted " << stats << std::endl;
        assert(false);
    }
}

void testUndoMoveCapture() {
    std::string fen = "r3r1k1/pp3pbp/1qp1b1p1/2B5/2BP4/Q1n2N2/P4PPP/3R1K1R w - - 4 18";
    ChessBoard board;
    board.initializeFEN(fen);
    std::string orig = board.getStringFromBitBoards();
    board.makeMoveFromUCI("c4e6");
    board.undo();
    if (board.squareArray[e6] != bB) {
        std::cerr << "TestUndoMoveCapture (theirbishop): got " << board.squareArray[e6].toString() 
                  << ", wanted " << bB.toString() << std::endl;
        assert(false);
    }
    if (board.squareArray[c4] != wB) {
        std::cerr << "TestUndoMoveCapture (ourbishop): got " << board.squareArray[c4].toString() 
                  << ", wanted " << wB.toString() << std::endl;
        assert(false);
    }
    std::string nowStr = board.getStringFromBitBoards();
    if (orig != nowStr) {
        std::cerr << "TestUndoMoveCapture (bitboard): got " << nowStr 
                  << ", wanted " << orig << std::endl;
        assert(false);
    }
}

void testAllMovesMakeUnmake() {
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ChessBoard board;
    board.initializeFEN(fen);
    std::string orig = board.getStringFromBitBoards();
    std::string stats = board.getStatsString();
    auto moves = board.generateLegalMoves();
    for (const auto& m : moves) {
        board.makeMove(m);
        // Assume Perft returns number of nodes; here we simply call it.
        int dummy = board.perft(4);
        board.undo();
        std::string newStats = board.getStatsString();
        std::string nowStr = board.getStringFromBitBoards();
        if (orig != nowStr) {
            std::cerr << "TestAllMovesMakeUnmake (" << m.toUCI() << "): got " << nowStr 
                      << ", wanted " << orig << std::endl;
            assert(false);
        }
        if (stats != newStats) {
            std::cerr << "TestAllMovesMakeUnmake (stats): got " << newStats 
                      << ", wanted " << stats << std::endl;
            assert(false);
        }
    }
}

void testThreeFoldRep() {
    ChessBoard board;
    board.initializeStartingPosition();
    board.makeMoveFromUCI("e2e4");
    board.makeMoveFromUCI("e7e5");
    board.makeMoveFromUCI("g1f3");
    board.makeMoveFromUCI("g8f6");
    board.makeMoveFromUCI("f3g1");
    board.makeMoveFromUCI("f6g8");
    board.makeMoveFromUCI("g1f3");
    board.makeMoveFromUCI("g8f6");
    board.makeMoveFromUCI("f3g1");
    board.makeMoveFromUCI("f6g8");
    bool rep = board.isThreeFoldRep();
    if (!rep) {
        std::cerr << "TestThreeFoldRep: got false, wanted true" << std::endl;
        assert(false);
    }
}

void testGenerateCaptures() {
    std::string fen = "r3r1k1/pp3pbp/1qp1b1p1/2B5/2BP4/Q1n2N2/P4PPP/3R1K1R w - - 4 18";
    ChessBoard board;
    board.initializeFEN(fen);
    auto moves = board.generateCaptures();
    std::vector<std::string> uciMoves;
    for (const auto& m : moves)
        uciMoves.push_back(m.toUCI());
    std::vector<std::string> expected = {"c4e6", "c5b6", "a3c3", "a3a7"};
    if (!checkSameElements(expected, uciMoves)) {
        std::cerr << "TestGenerateCaptures (not in check): got ";
        for (const auto& s : uciMoves) std::cerr << s << " ";
        std::cerr << ", wanted ";
        for (const auto& s : expected) std::cerr << s << " ";
        std::cerr << std::endl;
        assert(false);
    }
    fen = "r3r1k1/pp3pbp/1qp3p1/2B5/2bP4/Q1n2N1P/P4PP1/3R1K1R w - - 0 19";
    board.initializeFEN(fen);
    moves = board.generateCaptures();
    uciMoves.clear();
    for (const auto& m : moves)
        uciMoves.push_back(m.toUCI());
    expected.clear(); // expecting no captures
    if (!checkSameElements(expected, uciMoves)) {
        std::cerr << "TestGenerateCaptures (in check): got ";
        for (const auto& s : uciMoves) std::cerr << s << " ";
        std::cerr << ", wanted empty list" << std::endl;
        assert(false);
    }
}

int main() {
    testStartPos();
    testEnPassantPseudoPin();
    testTwoEnPassant();
    testTwoEnpassantOneLegal();
    testNoPawnMoves();
    testCastlingIfSquaresAttacked();
    testAllMoves();
    testAllMovesHuge();
    testPromotion();
    testMakeAndUndoMovePromotion();
    testMakeAndUndoEnPassant();
    testUndoMoveCapture();
    testAllMovesMakeUnmake();
    testThreeFoldRep();
    testGenerateCaptures();

    std::cout << "All tests passed successfully." << std::endl;
    return 0;
}
