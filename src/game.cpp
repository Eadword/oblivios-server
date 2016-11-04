#include <algorithm>
#include <iterator>

#include <base.hpp>
#include <json.hpp>

#include "game.h"
#include "player.h"

//Used only for the Game constructor
uint8_t getNumPlayers(const Json& j) {
    try {
        unsigned int t = j.at("num_players");
        if(t > 0xff) throw std::invalid_argument("Number of players is not valid");
        return (uint8_t)t;
    } catch(std::out_of_range& e) {
        throw std::invalid_argument("Number of players is not set");
    } catch(std::domain_error& e) {
        throw std::invalid_argument("Number of players is invalid");
    }
}


Game::Game(const Json& config) : num_players(getNumPlayers(config)) {
    std::fill_n(std::begin(last_pid), 0x10000, 0);

    players = new Player[num_players];

    //Read player configuration
    try {
        const Json& settings = config.at("player_settings");
        for (uint8_t x = 0; x < num_players && x < settings.size(); ++x)
            players[x] = Player(settings[x], (uint8_t)(x + 1));
    }
    catch(std::domain_error& e) { throw std::invalid_argument("Invalid player settings"); }
    catch(std::out_of_range& e) {}

    //Convert the quotable bytecode into a binary string
    //Create array of binary strings
    std::string* programs = new std::string[num_players];
    try {
        const Json& warriors = config.at("warriors");
        if(warriors.size() < num_players)
            throw std::invalid_argument("Config does not contain enough warrior programs");

        //Decode the strings
        for(uint8_t x = 0; x < num_players; ++x) {
            //Store the decoded string, and check if the size is zero, there was a problem 
            if(!( programs[x] = base64::decode(warriors[x].get<std::string>()) ).size())
                throw std::invalid_argument(players[x].name + " did not decode properly");
        }
    }
    catch(std::domain_error& e) { throw std::invalid_argument("Invalid warriors array"); }

    //Read the max player size from the configuration
    uint16_t max_size; //0 means no limit, as long as they all fit
    try { max_size = config.at("max_player_size"); }
    catch(std::out_of_range& e) { max_size = (uint8_t)(0x10000 / num_players); }
    catch(std::domain_error& e) { throw std::invalid_argument("Invalid max player size"); }

    //Count the space needed by the warriors and make sure that they are not over the max
    uint32_t total_size = 0;
    for(uint8_t x = 0; x < num_players; ++x) {
        const size_t size = programs[x].size();
        if(size > (max_size ? max_size : 0x10000)) //default maxsize to RAM size if 0
            throw std::invalid_argument(players[x].name + " is larger than the max size");
        total_size += size;
        if(total_size > 0x10000)
            throw std::invalid_argument("Total size of warriors exceeds RAM capacity");
    }

    //All the warriors are at least 1 byte and will fit

    delete[] programs;
}

Game::~Game() {
    delete[] players;
}