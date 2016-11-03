#include <iostream>
#include <json.hpp>

#include "opcode.h"
#include "instruction.h"
#include "argument.h"
#include "game.h"

int main() {
    Json json = {
            {"num_players", 255}
    };

    Game game(json);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}