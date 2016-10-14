#pragma once

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
 * @param arg False if for arg1, True if for arg2
 * @return The Location enum value
 */
inline Location LocationFromInt(uint8_t loc, bool arg = false) {
    try {
        if (arg) return Location_By_Arg2.at(loc);
        else return Location_By_Arg1.at(loc);
    } catch(std::invalid_argument& e) {
        return Location::NONE;
    }
}