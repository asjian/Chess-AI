cmake_minimum_required(VERSION 3.10)
project(ChessAI)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/engine)

add_library(engine
    engine/Bitboard.cpp
    engine/Board.cpp
    engine/Constants.cpp
    engine/Evaluation.cpp
    engine/Move.cpp
    engine/MoveGen.cpp
    engine/Search.cpp
    engine/TTable.cpp
    engine/UCI.cpp
)

add_executable(main_exe
    main/main.cpp
)

target_link_libraries(main_exe engine)
