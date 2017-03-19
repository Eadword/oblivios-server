#pragma once
#include <cstdint>

struct Player;
struct Thread;
enum class OPCode : uint8_t;

#include <json.hpp>
using Json = nlohmann::json;

/**
 * Abstract the game such that the server could handle more than one
 */
class Game {
    /// The memory warriors will run in
    uint8_t ram[0x10000];
    /// The pid of last modification
    uint8_t pid[0x10000];
    uint64_t cycle;

    //TODO: Create GameSettings Object to handle all of this crap
    const uint16_t cycles_per_turn;
    const int64_t max_cycles;
    const uint16_t ram_access_cycles;
    const uint16_t ram_double_access_penalty;
    const uint32_t score_for_killing_thread;
    const uint32_t score_for_killing_process;
    const float score_for_owning_ram;

    /// The number of players in a given game
    const uint8_t num_players;
    /// Array of players, index i is the player with pid i + 1
    Player* players;

    /**
     * Send the inital information about game state.
     * @param log The log stream
     */
    void sendInit(std::ostream& log);

    /**
     * Executes the next instruction.
     * @param thread The thread to run.
     * @param log The output stream for updates.
     * @return True if successful, false if not.
     */
    bool execIns(Thread& thread, const uint8_t pid, uint32_t& remaining_cycles, std::ostream& log);

    /**
     * Charges remaining cycles by proper amount given the thread state and opcode.
     * @param opcode OPCode of the current instruction.
     * @param arg1m Is arg1 a memory location?
     * @param arg2m Is arg2 a memory location?
     * @return True if the operation can be completed, false otherwise.
     *
     * @note Modifies thread.cycles to be new value if needed
     * @note Modifies remaining_cycles to be new value
     */
    bool remainingCycles(Thread& thread, uint32_t& remaining_cycles, const OPCode opcode,
                             const bool arg1m, const bool arg2m) const;

public:
    Game() = delete;
    Game(const Json& config);
    ~Game();

    void run(std::ostream& log);

    /// @warning This prints a lot of stuff
    friend std::ostream& operator<<(std::ostream& os, const Game& game);
};