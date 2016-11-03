#pragma once

#include "opcode.h"

struct Thread {
    uint16_t ax, bx, cx, ip;
    // overflow, sign, zero, carry
    bool o, s, z, c;

    Thread() {
        ax = bx = cx = ip = 0;
        o = s = z = c = false;
    }
};