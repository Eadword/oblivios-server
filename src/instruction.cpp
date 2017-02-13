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

    addr += 2; //at least 2
    if(argn > 1) addr += 2; //4 total

    return addr;
}