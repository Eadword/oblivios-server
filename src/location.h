#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>


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
        if(argn == 1) return Location_By_Arg1.at(loc);
        else if(argn == 2) return Location_By_Arg2.at(loc);
        else throw std::out_of_range("Invalid argument number " + argn);
    } catch(std::invalid_argument& e) {
        return Location::NONE;
    }
}

/**
 * Finds the location in the map and then returns the value that represents it.
 * @param loc Location to find the key for.
 * @param argn Which argument are we searching for, e.g. 1, or 2.
 * @return Value representing that location.
 */
inline uint8_t LocationToInt(Location loc, uint8_t argn = 1) {
    auto foundloc = [&loc](std::pair<uint8_t, Location> pair) -> bool {
        return pair.second == loc;
    };

    if(argn == 1)
        return std::find_if(Location_By_Arg1.begin(), Location_By_Arg1.end(), foundloc)->first;
    else
        return std::find_if(Location_By_Arg2.begin(), Location_By_Arg2.end(), foundloc)->first;
}

inline uint8_t RouteToInt(Location arg1, Location arg2) {
    return LocationToInt(arg1, 1) | (LocationToInt(arg2, 2) << 4);
}