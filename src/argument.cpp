#include "argument.h"

#include "instruction.h"
#include "thread.h"

#define REGISTER(loc, reg, type)    \
    Location::loc:                  \
    loc_type = type;                \
    location.r = &thread.reg;

#define REGISTER_PTR(loc, reg)                          \
    Location::loc:                                      \
    loc_type = M;                                       \
    if(mode == AccessMode::RELATIVE)                    \
        location.m = thread.ip + (int16_t)thread.reg;   \
    else location.m = thread.reg;

Argument::Argument(Thread& thread, uint8_t* ram, uint8_t argn) : ram(ram), loc_type(NONE), read_only(false) {
    Location loc;
    AccessMode mode;
    if(argn == 1) {
        loc = Instruction::getArg1Loc(ram, thread.ip);
        mode = Instruction::getArg1Mode(ram, thread.ip);
    }
    else if(argn == 2) {
        loc = Instruction::getArg2Loc(ram, thread.ip);
        mode = Instruction::getArg2Mode(ram, thread.ip);
    }
    else throw std::out_of_range("Invalid argument number " + argn);

    switch(loc) {
        case REGISTER(AL, ax, R8L); break;
        case REGISTER(AH, ax, R8H); break;
        case REGISTER(BL, bx, R8L); break;
        case REGISTER(BH, bx, R8H); break;
        case REGISTER(CL, cx, R8L); break;
        case REGISTER(CH, cx, R8H); break;
        case REGISTER(AX, ax, R16); break;
        case REGISTER(BX, bx, R16); break;
        case REGISTER(CX, cx, R16); break;
        case REGISTER(IP, ip, R16); read_only = true; break;

        case REGISTER_PTR(PAX, ax); break;
        case REGISTER_PTR(PBX, bx); break;
        case REGISTER_PTR(PCX, cx); break;

        case Location::IMD:
            loc_type = M16;
            location.m = Instruction::getImdAddress(ram, thread.ip, argn);
            break;

        case Location::PIMD:
            loc_type = M;
            if(mode == AccessMode::RELATIVE) {
                //add to the ip the value stored in the memory location of the immediate value
                location.m = thread.ip + (uint16_t)ram[Instruction::getImdAddress(ram, thread.ip, argn)];
            } else {
                //take the value of immediate as the address
                location.m = ram[Instruction::getImdAddress(ram, thread.ip, argn)];
            }
            break;

        case Location::NONE: throw std::invalid_argument("Invalid location"); break;
    }
}

uint16_t Argument::read() const {
    uint16_t v = 0x0000;
    switch(loc_type) {
        case R16:
            v = *location.r;
            break;
        case R8L:
            v = (uint8_t)(*location.r);
            break;
        case R8H:
            v = *location.r >> 8;
            break;
        case M: case M16:
            v += ram[location.m];
            v <<= 8;
            v += ram[(uint16_t)(location.m + 1)];
            break;
        case NONE:
            throw std::runtime_error("Program attempted to read from invalid memory");
    }
    return v;
}

void Argument::write(uint16_t v, const bool memforce8) {
    if(read_only) throw std::runtime_error("Program attempted to write to read-only memory");

    switch(loc_type) {
        case R16:
            *location.r = v;
            break;
        case R8L:
            *location.r &= 0xFF00;
            *location.r += (uint8_t)v;
            break;
        case R8H:
            *location.r &= 0x00FF;
            *location.r += (v << 8);
            break;
        case M:
            if(memforce8) {
                ram[location.m] = (uint8_t) v;
                break;
            }
            // else follow M16 case
        case M16:
            ram[(uint16_t)(location.m + 1)] = (uint8_t)v;
            ram[location.m] = (uint8_t)(v >> 8);
            break;
        case NONE:
            throw std::runtime_error("Program attempted to write to invalid memory");
    }
}

void Argument::swp(Argument& other) {
    uint16_t t = other.read();
    other.write(*this);
    write(t, other.loc_type == M);
}