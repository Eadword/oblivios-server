#pragma once
#include <queue>

#include "thread.h"

namespace nolhmann {class json;}
using Json = nlohmann::json;


struct Player {
    // pid is defined by its index in the game array + 1
    float cycle_modifer;
    uint8_t max_threads;

    uint32_t owned_ram;
    uint32_t killed_threads;
    uint32_t killed_processes;
    uint32_t score;

    std::queue<Thread> threads;

    Player();

    /**
     * Pass a json object containing the player
     * configuration only.
     */
    Player(const Json& j);
};