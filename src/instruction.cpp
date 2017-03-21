#include "instruction.h"

uint8_t Instruction::numImds(const uint8_t* ram, uint16_t addr) {
    uint8_t count = 0;
    Location t = getArg1Loc(ram, addr);
    if(t == Location::IMD || t == Location::PIMD) ++count;
    t = getArg2Loc(ram, addr);
    if(t == Location::IMD || t == Location::PIMD) ++count;
    return count;
}

uint16_t Instruction::getImdAddress(const uint8_t* ram, uint16_t addr, uint8_t argn) {
    if(!argn) return 0;
    uint8_t num_imds = Instruction::numImds(ram, addr);
    if(!num_imds) return 0;

    return addr += std::min(argn, num_imds) * 2;
}

void Instruction::constructInstruction(uint8_t* ram, uint16_t index, OPCode opcode,
                                           AccessMode arg1mode, AccessMode arg2mode,
                                           Location arg1loc, Location arg2loc) {

    ram[index] = (uint8_t)(OPCodeToInt(OPCode::MOV) << 2) |
                 (uint8_t)(((uint8_t)arg1mode & 0x01) << 1) |
                 (uint8_t)(((uint8_t)arg2mode & 0x01));

    ram[++index] =  RouteToInt(arg1loc, arg2loc);
}