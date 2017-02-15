#pragma once
#include <json.hpp>
using Json = nlohmann::json;


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

/// @note this performs a linear search
OPCode OPCodeFromString(const std::string&);

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


/**
 * Load
 * @param config
 */
void loadOPCodeCycles(const Json& config);

/**
 * Gets the number of cycles required by an opcode to execute.
 * @param op The opcode in question
 * @return The number of cycles needed
 *
 * @warning This requires a previous call to have been made to loadOPCodeCycles
 */
uint32_t getOPCodeCycles(OPCode op);