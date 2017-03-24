#pragma once
#include <cstdint>


struct Thread {
    uint16_t ax, bx, cx, ip;
    // overflow, sign, zero, carry
    bool o, s, z, c;

    /// in case it gets stopped mid-instruction, this is how many more cycles it needs
    uint32_t cycles;

    Thread(uint16_t ip = 0) : ax(0), bx(0), cx(0), ip(ip), o(false), s(false), z(false), c(false), cycles(0) {}

    /**
     * Read lower 8 bits of a register. E.g. AL = readLow(AX);
     * @param reg Register value to read from.
     * @return Lower 8 bits.
     */
    static uint8_t readLow(uint16_t reg);

    /**
     * Read higher 8 bits of a register. E.g. AH = readHigh(AX);
     * @param reg Register value to read from.
     * @return Higher 8 bits.
     */
    static uint8_t readHigh(uint16_t reg);
};


inline uint8_t Thread::readLow(uint16_t reg) {
    return reg & 0x00FF;
}

inline uint8_t Thread::readHigh(uint16_t reg) {
    return reg >> 8;
}