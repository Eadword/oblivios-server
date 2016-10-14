#pragma once

#include "location.h"

enum class AccessMode : uint8_t { DIRECT, RELATIVE };

/**
 * This namespace provides functions useful for decoding instructions residing in the RAM.
 * @note This assumes that the RAM is addressed for all valid uint16_t values, i.e. 0 to 2^16 -1.
 * @param ram Pointer to the beginning of RAM
 * @param addr Address of the start of the instruction in RAM
 */
namespace Instruction {
    inline OPCode getOPCode(const uint8_t* ram, uint16_t addr) {
        return OPCodeFromInt(ram[addr] >> 2);
    }

    inline AccessMode getArg1Mode(const uint8_t* ram, uint16_t addr) {
        //if the value is 1, return relative, otherwise direct
        return ((ram[addr] & 0x02) >> 1) ? AccessMode::RELATIVE : AccessMode::DIRECT;
    }

    inline AccessMode getArg2Mode(const uint8_t* ram, uint16_t addr) {
        //if the value is 1, return relative, otherwise direct
        return (ram[addr] & 0x01) ? AccessMode::RELATIVE : AccessMode::DIRECT;
    }

    inline Location getArg1Loc(const uint8_t* ram, uint16_t addr) {
        return LocationFromInt(ram[addr + 1] & 0x0F, false);
    }

    inline Location getArg2Loc(const uint8_t* ram, uint16_t addr) {
        return LocationFromInt((ram[addr + 1] & 0xF0) >> 4, true);
    }
};