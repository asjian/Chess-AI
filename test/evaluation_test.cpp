#include <iostream>
#include <cassert>
#include "Board.h"
#include "Evaluation.h"
#include "Constants.h"

using namespace Chess;

void testCheckmate() {
    {
        std::string fen = "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4";
        ChessBoard board;
        board.initializeFEN(fen);
        int res = evaluatePosition(&board);
        if (res != WIN_VALUE) {
            std::cerr << "TestCheckmate (white): got " << res << ", wanted " << WIN_VALUE << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3";
        ChessBoard board;
        board.initializeFEN(fen);
        int res = evaluatePosition(&board);
        if (res != -WIN_VALUE) {
            std::cerr << "TestCheckmate (black): got " << res << ", wanted " << -WIN_VALUE << std::endl;
            assert(false);
        }
    }
}

void testStalemate() {
    {
        std::string fen = "7K/5k1P/8/8/8/8/8/8 w - - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        int res = evaluatePosition(&board);
        if (res != 0) {
            std::cerr << "TestStalemate (white): got " << res << ", wanted " << 0 << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "8/8/8/8/8/8/p1K5/k7 b - - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        int res = evaluatePosition(&board);
        if (res != 0) {
            std::cerr << "TestStalemate (black): got " << res << ", wanted " << 0 << std::endl;
            assert(false);
        }
    }
}

void testMaterial() {
    {
        std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        auto [mat, total] = totalMaterialAndPieces(&board);
        if (mat != 0) {
            std::cerr << "TestMaterial (equal): got " << mat << ", wanted " << 0 << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2";
        ChessBoard board;
        board.initializeFEN(fen);
        auto [mat, total] = totalMaterialAndPieces(&board);
        if (mat != PAWN_VALUE) {
            std::cerr << "TestMaterial (white pawn up): got " << mat << ", wanted " << PAWN_VALUE << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "rnb1kb1r/ppp1pppp/5n2/3N4/8/8/PPPP1PPP/R1BQKBNR b KQkq - 0 4";
        ChessBoard board;
        board.initializeFEN(fen);
        auto [mat, total] = totalMaterialAndPieces(&board);
        if (mat != QUEEN_VALUE) {
            std::cerr << "TestMaterial (white queen up): got " << mat << ", wanted " << QUEEN_VALUE << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "rnb1kbnr/ppp1pppp/8/7q/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 4";
        ChessBoard board;
        board.initializeFEN(fen);
        auto [mat, total] = totalMaterialAndPieces(&board);
        if (mat != -QUEEN_VALUE) {
            std::cerr << "TestMaterial (black queen up): got " << mat << ", wanted " << -QUEEN_VALUE << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "2r5/1p1k1pp1/p2p4/P7/2R4p/1P1b1B1P/2r2PP1/3KR3 w - - 0 31";
        ChessBoard board;
        board.initializeFEN(fen);
        auto [mat, total] = totalMaterialAndPieces(&board);
        if (mat != -PAWN_VALUE) {
            std::cerr << "TestMaterial (black pawn up): got " << mat << ", wanted " << -PAWN_VALUE << std::endl;
            assert(false);
        }
    }
}

void testInsufficientMaterial() {
    {
        std::string fen = "8/4k3/8/2n5/8/8/8/3K4 w - - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        if (!board.isInsufficientMaterial()) {
            std::cerr << "TestInsufficientMaterial (b+k vs K): got false, wanted true" << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "8/4k3/2b5/3n4/8/8/8/3K4 w - - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        if (board.isInsufficientMaterial()) {
            std::cerr << "TestInsufficientMaterial (b+n+k vs K): got true, wanted false" << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "8/4k3/2n5/3n4/8/8/8/3K4 w - - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        if (!board.isInsufficientMaterial()) {
            std::cerr << "TestInsufficientMaterial (n+n+k vs K): got false, wanted true" << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "8/4k3/2n5/3n4/8/8/4P3/3K4 w - - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        if (board.isInsufficientMaterial()) {
            std::cerr << "TestInsufficientMaterial (n+n+k vs K+P): got true, wanted false" << std::endl;
            assert(false);
        }
    }
    {
        std::string fen = "8/4k3/8/8/8/8/4P3/3K4 w - - 0 1";
        ChessBoard board;
        board.initializeFEN(fen);
        if (board.isInsufficientMaterial()) {
            std::cerr << "TestInsufficientMaterial (k vs K+P): got true, wanted false" << std::endl;
            assert(false);
        }
    }
}

int main() {
    testCheckmate();
    testStalemate();
    testMaterial();
    testInsufficientMaterial();
    std::cout << "All tests passed successfully." << std::endl;
    return 0;
}