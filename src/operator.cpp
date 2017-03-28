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


void Operator::div(Thread& thread, const Argument& arg) {
    if(arg.is8Bit()) {
        const uint16_t n = thread.ax;
        const uint16_t d = arg.read(true);
        if(d == 0) throw std::runtime_error("Divide by zero error.");

        thread.ax = 0x0000;
        const uint16_t q = n / d;
        const uint16_t r = n % d;
        thread.o = q > 0xFF;
        thread.ax = (r << 8) | (uint8_t)q;
    }
    else { //16bit
        const uint32_t n = (thread.bx << 16) | thread.ax;
        const uint32_t d = arg.read();
        if(d == 0) throw std::runtime_error("Divide by zero error.");

        const uint32_t q = n / d;
        const uint32_t r = n % d;
        thread.o = q > 0xFFFF;
        thread.ax = (uint16_t)q;
        thread.bx = (uint16_t)r;
    }
}


void Operator::idiv(Thread& thread, const Argument& arg) {
    if(arg.is8Bit()) {
        const int16_t n = thread.ax;
        const int16_t d = (int8_t)arg.read(true);
        if(d == 0) throw std::runtime_error("Divide by zero error.");

        thread.ax = 0x0000;
        const int16_t q = n / d;
        const int16_t r = n % d;
        thread.o = (q > 0x7F || q < -0x80);
        thread.ax = (uint16_t)(r << 8) | (uint8_t)q;
    }
    else { //16bit
        const int32_t n = (thread.bx << 16) | thread.ax;
        const int32_t d = (int16_t)arg.read();
        if(d == 0) throw std::runtime_error("Divide by zero error.");

        const int32_t q = n / d;
        const int32_t r = n % d;
        thread.o = (q > 0x7FFF || q < -0x8000);
        thread.ax = (uint16_t)q;
        thread.bx = (uint16_t)r;
    }
}


void Operator::imul(Thread& thread, const Argument& arg) {
    if(arg.is8Bit()) {
        int16_t v = (int8_t)arg.read(true);
        v *= (int8_t)Thread::readLow(thread.ax);
        if(v < 0x7F && v > -0x80 ) { //Fits
            thread.o = false;
            thread.c = false;
            thread.ax &= 0xFF00;
            thread.ax |= (uint8_t)v;
        }
        else { //Overflow
            thread.o = true;
            thread.c = true;
            thread.ax = (uint16_t)v;
        }
    }
    else { //16bit
        int32_t v = (int16_t)arg.read();
        v *= (int16_t)thread.ax;
        if(v < 0x7FFF && v > -0x8000 ) { //Fits
            thread.o = false;
            thread.c = false;
            thread.ax = (uint16_t)v;
        }
        else { //Overflow
            thread.o = true;
            thread.c = true;
            thread.ax = (uint16_t)v;
            thread.bx = (uint16_t)(v >> 16);
        }
    }
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


void Operator::shl(Thread& thread, Argument& arg1, const Argument& arg2) {
    uint16_t v = arg1.read();
    const uint16_t n = arg2.read();

    //set carry to last bit which is cut off
    thread.c = ((v << std::max((int)n - 1, 0)) & (arg1.is8Bit() ? 0x80 : 0x8000)) != 0;

    v <<= n;
    arg1.write(v, arg2.is8Bit());

    thread.s = (v & (arg1.is8Bit() ? 0x80: 0x8000)) != 0;
    if(n == 1)
        //set overflow to true if the sign changed because of the shift
        thread.o =  thread.s != thread.c;

    thread.z = !v;
}


void Operator::shr(Thread& thread, Argument& arg1, const Argument& arg2) {
    uint16_t v = arg1.read();
    const uint16_t n = arg2.read();

    //set overflow flag to most significant bit of origional
    thread.o = (v & (arg1.is8Bit() ? 0x80 : 0x8000)) != 0;
    //set carry to last bit which is cut off
    thread.c = ((v >> std::max((int)n - 1, 0)) & (0x0001)) != 0;

    v >>= arg2.read();
    arg1.write(v, arg2.is8Bit());

    thread.z = !v;
    thread.s = (v & (arg1.is8Bit() ? 0x80 : 0x8000)) != 0;
}


void Operator::sub(Thread& thread, Argument& arg1, const Argument& arg2) {
    ::add(thread, arg1, arg2, true);
}


void Operator::swp(Argument& arg1, Argument& arg2) {
    arg1.swp(arg2);
}