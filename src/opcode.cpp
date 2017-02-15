#include "opcode.h"

#include <fstream>


std::map<OPCode, uint32_t> OPCode_NumCycles;

OPCode OPCodeFromString(const std::string& s) {
    const auto begin = OPCode_Strings.begin();
    const auto end = OPCode_Strings.end();

    const auto loc = find(begin, end, s);
    if(loc == end) throw std::invalid_argument(s + " is not a valid opcode.");
    return OPCodeFromInt((uint8_t)(loc - begin));
}

void loadOPCodeCycles(const Json& config) {
    const Json& cycles = config.at("cycles");
    if(!cycles.is_object()) return;
    for(auto&& itr = cycles.begin(); itr != cycles.end(); ++itr) {
        const OPCode op = OPCodeFromString(itr.key());
        OPCode_NumCycles[op] = *itr;
    }
}

uint32_t getOPCodeCycles(OPCode op) {
    try {
        return OPCode_NumCycles.at(op);
    } catch(std::out_of_range& e) {
        return 1;
    }
}