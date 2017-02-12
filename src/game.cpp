#include <algorithm>
#include <cstdint>
#include <iterator>
#include <random>

#include <base.hpp>
#include <json.hpp>

#include "game.h"
#include "instruction.h"
#include "player.h"

//Used only for the Game constructor
template<typename T>
T readNum(const Json& j, std::string field, const int64_t min_value = INT64_MIN,
          const int64_t max_value = INT64_MAX) {
    try {
        int64_t t = j.at(field);
        if(t > max_value || t < min_value) throw std::invalid_argument(field + " is out of bounds");
        return (T)t;
    } catch(std::out_of_range& e) {
        throw std::invalid_argument(field + " is not set");
    } catch(std::domain_error& e) {
        throw std::invalid_argument(field + " is invalid");
    }
}


Game::Game(const Json& config) :
        num_players(readNum<uint8_t>(config, "num_players", 1, UINT8_MAX)),
        cycles_per_turn(readNum<uint16_t>(config, "cycles_per_turn", 1, UINT16_MAX)),
        max_cycles(readNum<int64_t>(config, "max_cycles", 1)){
    std::fill_n(pid, 0x10000, 0);

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
            if( !(programs[x] = base64::decode(warriors[x].get<std::string>())).size() )
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
        players[x].owned_ram = (uint32_t)size;
        total_size += size;
        if(total_size > 0x10000)
            throw std::invalid_argument("Total size of warriors exceeds RAM capacity");
    }

    //All the warriors are at least 1 byte and will fit
    //Now we put them into RAM
    //start by creating a non-repeating rand with random_shuffle which defines the order of programs
    uint8_t* order = new uint8_t[num_players];
    std::generate(order, order+num_players, [](){ static uint8_t x = 0; return x++; } );
    std::default_random_engine generator;
    generator.seed((unsigned long)time(nullptr));
    std::shuffle(order, order+num_players, generator);

    const uint16_t max_gap = (uint16_t)((0x10000 - total_size) / num_players);
    std::uniform_int_distribution<uint16_t> distribution(0, max_gap);
    uint16_t cur_offset = 0;
    for(uint8_t x = 0; x < num_players; ++x) {
        const uint8_t cur_pid = (uint8_t)(order[x] + 1);
        const std::string& cur_code = programs[cur_pid - 1];
        cur_offset += distribution(generator); //random offset that will not allow out of range indexing

        //copy binary data into ram at the location
        std::copy(cur_code.begin(), cur_code.end(), ram+cur_offset);
        //update who "owns" the RAM we just wrote
        std::fill_n(pid+cur_offset, cur_code.size(), cur_pid);

        //add thread to player
        players[cur_pid - 1].threads.push(new Thread(cur_offset));
        cur_offset += cur_code.size();
    }

    delete[] order;
    delete[] programs;
}

Game::~Game() {
    delete[] players;
}


void Game::run(std::ostream& log) {
    sendInit(log);

    uint8_t alive = num_players;

    //main loop, runs until only one AI continues running or max cycles is reached
    for(uint64_t cycle = 1; alive && cycle < max_cycles - alive * cycles_per_turn; ++cycle) {
        //run through player turns
        for(uint8_t pid = 1; pid <= num_players; ++pid) {
            Player& player = players[pid - 1];

            //get next thread to run
            if(player.threads.empty()) continue;
            Thread*const thread = player.threads.front();
            player.threads.pop();

            //process instruction
            //TODO: finish this up
            Instruction::getOPCode(ram, thread->ip);

            //add thread back to end of queue
            player.threads.push(thread);
        }
    }
}


void Game::sendInit(std::ostream& log) {
    //tell server where the warriors are
    Json update = {{"type", "init"},
                   {"cycle", 0}};

    //player has a region, server already knows what data to fill in if given starting position
    for (uint8_t x = 0; x < num_players; ++x) {
        const Player& player = players[x];

        const uint16_t start = player.threads.front()->ip;
        //const uint16_t end = (uint16_t) //find end of pid and then distance from start to it
        //        (std::find_if_not(pid+start, pid+0x10000, [x](int v){ return v == x + 1;}) - pid);

        //std::string data = base64::encode(std::string((char*)(ram+start), end - start));
        update["starts"].push_back(start);
    }

    log << update;
}



std::ostream& operator<<(std::ostream& os, const Game& game) {
    const size_t BUFFER_SIZE = 3;

    //os << std::hex << std::uppercase;

    char buffer[BUFFER_SIZE];

    for(uint16_t x = 0; ; ++x) {
        snprintf(buffer, BUFFER_SIZE, "%02X", game.ram[x]);
        if((x + 1) % 64 == 0) os << buffer << '\n';
        else os << buffer << ' ';
        if(x == 0xFFFF) break;
    }

    os.unsetf(std::ios::hex);
    os.unsetf(std::ios::uppercase);
    return os;
}