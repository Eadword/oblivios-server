#include <iostream>
#include <fstream>
#include <json.hpp>

#include "opcode.h"
#include "instruction.h"
#include "argument.h"
#include "game.h"

int main() {
    std::ifstream file("data/test.json");
    if(!file) return -1;
    Json json;
    file >> json;
    file.close();

    Game game(json);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}