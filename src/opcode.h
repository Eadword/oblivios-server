#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class OPCode : uint8_t {
#define X(val, b) val
#include "opcodes"
#undef X
};

const std::vector<std::string> OPCode_Strings {
#define X(val, b) #val
#include "opcodes"
#undef X
};

const std::vector<uint8_t> OPCode_NumParams {
#define X(b, val) val
#include "opcodes"
#undef X
};

inline std::string OPCodeToString(OPCode op) {
    return OPCode_Strings[(uint8_t)op];
}

inline std::ostream& operator<<(std::ostream& out, OPCode op) {
    return out << OPCodeToString(op);
}

/**
 * Gets the opcode of the integer opcode value.
 * @param op The integer opcode value
 * @return The opcode, or NOP if not valid
 */
inline OPCode OPCodeFromInt(uint8_t op) {
    if(op < (uint8_t)OPCode::NONE) return (OPCode)op;
    else return OPCode::NOP;
}

/**
 * Gets the number of parameters expected for a given opcode.
 * @param op The opcode
 * @return The number of parameters expected
 */
inline uint8_t getOPCodeParams(OPCode op) { return OPCode_NumParams[(uint8_t)op]; }