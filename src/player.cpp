#include <json.hpp>
#include "player.h"

Player::Player() : cycle_modifer(1), max_threads(32), owned_ram(0), killed_processes(0), score(0) {}

Player::Player(const Json& j) : Player() {
    if(!j.is_object()) return;
    if(j["cycle_modifer"].is_number()) cycle_modifer = j["cycle_modifer"];
    if(j["max_threads"].is_number()) max_threads = j["max_threads"];
}