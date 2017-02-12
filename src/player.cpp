#include <json.hpp>
#include "player.h"

Player::Player() : cycle_modifer(1), max_threads(32), owned_ram(0), killed_threads(0),
                   killed_processes(0), score(0) {}

Player::Player(const Json& j, uint8_t pid) : Player() {
    if(!j.is_object()) return;
    try { cycle_modifer = j.at("cycle_modifer"); } catch(std::out_of_range& e) {}
    try { max_threads   = j.at("max_threads");   } catch(std::out_of_range& e) {}

    try { name = j.at("name");}
    catch(std::out_of_range& e) { name = "Process_" + std::to_string(pid); }
}