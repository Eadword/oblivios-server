#include "operator.h"

#include "argument.h"
#include "thread.h"

static inline uint16_t neg(uint16_t v) {
    return (uint16_t)(~v + 1);
}

static void add(Thread& thread, Argument& arg1, const Argument& arg2, bool negate = false) {
    const bool ebit = arg1.is8Bit();

    const uint16_t arg1v = arg1.read();
    const uint16_t arg2v = negate ? ::neg(arg2.read(ebit)) : arg2.read(ebit);

    const bool arg1s = arg1.sign();
    const bool arg2s = ((ebit ? 0x0080 : 0x8000) & arg2v) != 0;

    const uint16_t sum = arg1.write(arg1v + arg2v, arg2.is8Bit());

    thread.c = (sum < arg1v || sum < arg2v);
    thread.s = arg1.sign();
    thread.z = sum == 0;

    //overflow if both args have same sign and the arg has a sign which is the opposite
    thread.o = (arg1s == arg2s && arg1.sign() != arg1s);
}



void Operator::add(Thread& thread, Argument& arg1, const Argument& arg2) {
    ::add(thread, arg1, arg2);
}


void Operator::int_(Thread& thread, Argument& arg1, Argument& arg2) {
    //TODO: handle interrupts
}


void Operator::mov(Argument& arg1, const Argument& arg2) {
    arg1.write(arg2);
}


void Operator::mul(Thread& thread, const Argument& arg) {
    if(arg.is8Bit()) {
        uint16_t v = arg.read(true);
        v *= Thread::readLow(thread.ax);
        if(v & 0xFF00) { //Overflow
            thread.o = true;
            thread.c = true;
            thread.ax = v;
        }
        else { //Fits
            thread.o = false;
            thread.c = false;
            thread.ax &= 0xFF00;
            thread.ax |= v;
        }
    }
    else { //16bit
        uint32_t v = arg.read();
        v *= thread.ax;
        if(v & 0xFFFF0000) { //Overflow
            thread.o = true;
            thread.c = true;
            thread.ax = (uint16_t)v;
            thread.bx = (uint16_t)(v >> 16);
        }
        else { //Fits
            thread.o = false;
            thread.c = false;
            thread.ax = (uint16_t)v;
        }
    }
}


void Operator::neg(Thread& thread, Argument& arg) {
    const uint16_t v = arg.read();
    thread.z = v == 0;
    thread.c = v != 0;
    thread.o = v == 0x8000;

    arg.write( ::neg(v) );

    thread.s = arg.sign();
}


void Operator::sub(Thread& thread, Argument& arg1, const Argument& arg2) {
    ::add(thread, arg1, arg2, true);
}


void Operator::swp(Argument& arg1, Argument& arg2) {
    arg1.swp(arg2);
}