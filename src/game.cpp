#include <algorithm>
#include <iterator>
#include <json.hpp>
#include <malloc.h>

#include "game.h"
#include "player.h"

//Used only for the Game constructor
uint8_t getNumPlayers(const Json& j) {
    Json num_players = j["num_players"];
    if(!num_players.is_number() || num_players > 0xff)
        throw new std::invalid_argument("Number of players is not valid");
    return num_players;
}


Game::Game(const Json& j) : num_players(getNumPlayers(j)) {
    std::fill_n(std::begin(last_pid), 0x10000, 0);

    players = new Player[num_players];
    for(unsigned int x = 0; x < num_players; ++x) {
        const Json& t = j["player_settings"];
        if(!t.is_array()) continue;
        players[x] = Player(t[x]);
    }
}

Game::~Game() {
    delete[] players;
}