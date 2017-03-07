#include "opcode.h"
#include "instruction.h"
#include "argument.h"
#include "game.h"

#include <iostream>
#include <fstream>


int main() {
    std::ifstream file("data/default.json");
    if(!file) throw std::runtime_error("Could not load config file.");
    Json json;
    file >> json;
    file.close();

    Game game(json);
    game.run(std::cout);

    return 0;
}