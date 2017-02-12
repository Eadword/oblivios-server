#pragma once
#include <cstdint>

struct Player;

namespace nolhmann {class json;}
using Json = nlohmann::json;

/**
 * Abstract the game such that the server could handle more than one
 */
class Game {
    /// The memory warriors will run in
    uint8_t ram[0x10000];
    /// The pid of last modification
    uint8_t pid[0x10000];

    const uint16_t cycles_per_turn;
    const int64_t max_cycles;

    /// The number of players in a given game
    const uint8_t num_players;
    /// Array of players, index i is the player with pid i + 1
    Player* players;

    void sendInit(std::ostream& log);

public:
    Game() = delete;
    Game(const Json& config);
    ~Game();

    void run(std::ostream& log);

    /// @warning This prints a lot of stuff
    friend std::ostream& operator<<(std::ostream& os, const Game& game);
};