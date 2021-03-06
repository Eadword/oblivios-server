#pragma once
#include <cstdint>
#include "opcode.h"
#include "location.h"

enum class OPCode : uint8_t;
enum class Location : uint8_t;


enum class AccessMode : uint8_t { DIRECT, RELATIVE };

/**
 * This namespace provides functions useful for decoding instructions residing in the RAM.
 * @note This assumes that the RAM is addressed for all valid uint16_t values, i.e. 0 to 2^16 -1.
 * @param ram Pointer to the beginning of RAM
 * @param addr Address of the start of the instruction in RAM
 */
namespace Instruction {
    OPCode getOPCode(const uint8_t*const ram, uint16_t addr);

    AccessMode getArg1Mode(const uint8_t* ram, uint16_t addr);

    AccessMode getArg2Mode(const uint8_t* ram, uint16_t addr);

    Location getArg1Loc(const uint8_t* ram, uint16_t addr);

    Location getArg2Loc(const uint8_t* ram, uint16_t addr);

    uint8_t numImds(const uint8_t* ram, uint16_t addr);

    /**
     * @return Size of this argument, will be 2 + 2*(number of imds)
     */
    uint8_t getSize(const uint8_t* ram, uint16_t addr);

    /**
     * Gets the address of the immediate for an argument.
     * @todo support more than 2 args
     * @param ram Pointer to the beginning of RAM
     * @param addr Address of the start of the instruction in RAM
     * @param argn The argument number, with 1 being the first argument and 2 being the second
     * @return addr+0 if no immediates exist, addr+2, or addr+4 depending on number of immedaites and position.
     */
    uint16_t getImdAddress(const uint8_t* ram, uint16_t addr, uint8_t argn);

    /**
     * Constructs a new instruction in ram. Mostly for testing purposes.
     * @param ram RAM to insert new instruction into.
     * @param index Beginning address for new instruction (will use 16bits).
     * @param opcode Operation code for the instruction.
     * @param arg1mode AccessMode of first argument.
     * @param arg2mode AccessMode of second argument.
     * @param arg1loc Location where first argument is stored.
     * @param arg2loc Location where second argument is stored.
     *
     * @note Stores straight into RAM to avoid issues of endianness.
     */
    void constructInstruction(uint8_t* ram, uint16_t index, OPCode opcode, AccessMode arg1mode, AccessMode arg2mode, Location arg1loc, Location arg2loc);
};


#include "opcode.h"
#include "location.h"

inline OPCode Instruction::getOPCode(const uint8_t*const ram, uint16_t addr) {
    return OPCodeFromInt(ram[addr] >> 2);
}

inline AccessMode Instruction::getArg1Mode(const uint8_t* ram, uint16_t addr) {
    //if the value is 1, return relative, otherwise direct
    return ((ram[addr] & 0x02) >> 1) ? AccessMode::RELATIVE : AccessMode::DIRECT;
}

inline AccessMode Instruction::getArg2Mode(const uint8_t* ram, uint16_t addr) {
    //if the value is 1, return relative, otherwise direct
    return (ram[addr] & 0x01) ? AccessMode::RELATIVE : AccessMode::DIRECT;
}

inline Location Instruction::getArg1Loc(const uint8_t* ram, uint16_t addr) {
    return LocationFromInt(ram[addr + 1] & 0x0F, 1);
}

inline Location Instruction::getArg2Loc(const uint8_t* ram, uint16_t addr) {
    return LocationFromInt(ram[addr + 1] >> 4, 2);
}

inline uint8_t Instruction::getSize(const uint8_t* ram, uint16_t addr) {
    return (uint8_t)(2 + 2*Instruction::numImds(ram, addr));
}