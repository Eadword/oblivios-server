#pragma once

#include <map>
#include <vector>

enum class Location : uint8_t {
#define X(name, str, dstval, srcval) name
#include "locations"
#undef X
};

const std::vector<std::string> Location_Strings {
#define X(name, str, dstval, srcval) str
#include "locations"
#undef X
};

const std::map<int8_t, Location> Location_By_Arg1 {
#define X(name, str, dstval, srcval) {dstval, Location::name}
#include "locations"
#undef X
};

const std::map<int8_t, Location> Location_By_Arg2 {
#define X(name, str, dstval, srcval) {srcval, Location::name}
#include "locations"
#undef X
};


/**
 * Converts a location enum value to a string representation.
 * @param l The enum value
 * @return A string representing the value
 */
inline std::string LocationToString(Location l) {
    return Location_Strings[(uint8_t)l];
}

inline std::ostream& operator<<(std::ostream& out, Location l) {
    return out << LocationToString(l);
}

/**
 * Gets a location enum value from an integer value.
 * @param loc The integer to be converted
 * @param argn The number of the argument, starting at 1 (currently only 1 and 2 are valid)
 * @return The Location enum value
 */
inline Location LocationFromInt(uint8_t loc, const uint8_t argn = 1) {
    try {
        if(argn == 1) return Location_By_Arg2.at(loc);
        else if(argn == 2) return Location_By_Arg1.at(loc);
        else throw std::out_of_range("Invalid argument number " + argn);
    } catch(std::invalid_argument& e) {
        return Location::NONE;
    }
}