#ifndef RUN_H
#define RUN_H

#include <string>

namespace Chess {

void initialize();
void Run(const std::string& command, const std::string& position, int depth);
void RunSearch(const std::string& position, int depth);
void RunSelfPlay(const std::string& position, int depth);
void RunPlay(const std::string& position, int depth, int player);

}

#endif
