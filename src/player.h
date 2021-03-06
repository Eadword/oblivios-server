#pragma once
struct Thread;

#include <json.hpp>
using Json = nlohmann::json;

#include <cstdint>
#include <string>
#include <queue>


struct Player {
    // pid is defined by its index in the game array + 1
    float cycle_modifer;
    uint8_t max_threads;
    std::string name;

    uint32_t owned_ram;
    uint32_t killed_threads;
    uint32_t killed_processes;
    uint32_t score;

    std::queue<Thread*> threads;

    Player();

    /**
     * Pass a json object containing the player's
     * configuration only.
     */
    Player(const Json& j, uint8_t pid);
    ~Player();
};