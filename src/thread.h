#pragma once
#include <cstdint>


struct Thread {
    uint16_t ax, bx, cx, ip;
    // overflow, sign, zero, carry
    bool o, s, z, c;

    Thread(uint16_t ip = 0) : ax(0), bx(0), cx(0), ip(ip), o(false), s(false), z(false), c(false) {}
};