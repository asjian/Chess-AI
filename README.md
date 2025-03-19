# Chess-AI

C++ chess engine built from scratch. I still work on this project from time to time, and recently pushed my latest working version to this github respository. Many of the algorithms and optimizations I used can be attributed to the [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page).

## Features
- Fast bitboard move generation
- Opening Book
- Transposition Table
- Principal Variation Search (PVS)
- Move-ordering using priority queues, with ordering based on known standard techniques (Last PV-move, hash move, killer moves, captures)
- Evaluation function using both hand-crafting and trained neural networks
- Quiescence search (extending depth of evaluation function at unstable nodes)

In the future I will try to integrate an endgame tablebase, ensuring perfect play once there are 7 or less pieces remaining.
