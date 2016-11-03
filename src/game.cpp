#include <algorithm>
#include <iterator>
#include <json.hpp>

#include "game.h"
#include "player.h"

//Used only for the Game constructor
uint8_t getNumPlayers(Json j) {
    const Json num_players = j["num_players"];
    if(!num_players.is_number() || num_players > 0xff)
        throw new std::invalid_argument("Number of players is not valid");
    return num_players;
}


Game::Game(Json j) : num_players(getNumPlayers(j)) {
    std::fill_n(std::begin(last_pid), 0x10000, 0);
    players = new Player[num_players];
}

Game::~Game() {
    delete[] players;
}