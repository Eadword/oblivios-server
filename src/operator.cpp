#include "operator.h"

#include "argument.h"
#include "thread.h"


//Used for AND, OR, and XOR because they are all the same save for a single operator
#define DUAL_LOGIC_OPERATION(op)                                        \
    const bool ebit = Argument::is8BitOp(arg1, arg2);                   \
    const uint16_t v = arg1.read(ebit) op arg2.read(ebit);              \
    thread.o = thread.c = 0;                                            \
    thread.z = v == 0;                                                  \
    thread.s = ((ebit ? 0x80 : 0x8000) & v) != 0;                       \
    arg1.write(v, ebit);


//Sign mask
#define SMASK (ebit ? 0x0080 : 0x8000)

//Check if the var is signed
#define IS_SIGNED(var) (!!(var & SMASK))

//Bit mask
#define BMASK (ebit ? 0x00FF : 0xFFFF)

//Negate the value and apply a mask for the bit size
#define NEGATE(var) ((uint16_t)((~var + 1) & BMASK))


//static void add(Thread& thread, Argument& arg1, const Argument& arg2, bool negate = false) {
//    const bool ebit = Argument::is8BitOp(arg1, arg2);
//
//    const uint16_t arg1v = arg1.read(ebit);
//    const uint16_t arg2v = negate ? ::neg(arg2.read(ebit)) : arg2.read(ebit);
//
//    const bool arg1s = arg1.sign();
//    const bool arg2s = ((ebit ? 0x0080 : 0x8000) & arg2v) != 0;
//
//    const uint16_t sum = arg1.write(arg1v + arg2v, ebit);
//
//    thread.c = (sum < arg1v || sum < arg2v);
//    thread.s = arg1.sign();
//    thread.z = sum == 0;
//
//    //overflow if both args have same sign and the arg has a sign which is the opposite
//    thread.o = (arg1s == arg2s && arg1.sign() != arg1s);
//}



void Operator::add(Thread& thread, Argument& arg1, const Argument& arg2) {
    const bool ebit = Argument::is8BitOp(arg1, arg2);

    const uint16_t arg1v = arg1.read(ebit);
    const uint16_t arg2v = arg2.read(ebit);

    const bool arg1s = IS_SIGNED(arg1v);
    const bool arg2s = IS_SIGNED(arg2v);

    const uint16_t sum = arg1.write(arg1v + arg2v, ebit);
    const bool sum_sign = IS_SIGNED(sum);

    thread.c = (sum < arg1v || sum < arg2v);
    thread.s = sum_sign;
    thread.z = !sum;

    //overflow if both args have same sign and the result has a sign which is the opposite
    thread.o = (arg1s == arg2s && sum_sign != arg1s);
}


void Operator::_and(Thread& thread, Argument& arg1, const Argument& arg2) {
    DUAL_LOGIC_OPERATION(&)
}


void Operator::dec(Thread& thread, Argument& arg) {
    const bool ebit = arg.is8Bit();
    if(arg.is8Bit()) {
        uint8_t v = (uint8_t)(arg.read(true));
        thread.o = v == 0 || v == 0x80;
        v--;
        thread.s = (v & 0x80) != 0;
        thread.z = v == 0;
        arg.write(v, true);
    } else {
        uint16_t v = arg.read();
        thread.o = v == 0 || v == 0x8000;
        v--;
        thread.s = (v & 0x8000) != 0;
        thread.z = v == 0;
        arg.write(v);
    }
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


void Operator::inc(Thread& thread, Argument& arg) {
    const bool ebit = arg.is8Bit();
    if(arg.is8Bit()) {
        uint8_t v = (uint8_t)(arg.read(true));
        v++;
        thread.s = (v & 0x80) != 0;
        thread.o = v == 0 || v == 0x80;
        thread.z = v == 0;
        arg.write(v, true);
    } else {
        uint16_t v = arg.read();
        v++;
        thread.s = (v & 0x8000) != 0;
        thread.o = v == 0 || v == 0x8000;
        thread.z = v == 0;
        arg.write(v);
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
    const bool ebit = arg.is8Bit();
    const uint16_t v = arg.read(ebit); //8bit if in ram
    thread.z = v == 0;
    thread.c = v != 0;
    thread.o = v == (ebit ? 0x80 : 0x8000);

    arg.write(NEGATE(v), true); //always force ram to be 8bit

    thread.s = arg.sign();
}


void Operator::_not(Argument& arg) {
    const uint16_t v = arg.read(arg.is8Bit());
    arg.write(~v, true); //always force ram to be 8bit
}


void Operator::_or(Thread& thread, Argument& arg1, const Argument& arg2) {
    DUAL_LOGIC_OPERATION(|)
}


void Operator::shl(Thread& thread, Argument& arg1, const Argument& arg2) {
    const bool ebit = Argument::is8BitOp(arg1, arg2);
    uint16_t v = arg1.read(ebit);
    const uint16_t n = arg2.read();

    //set carry to last bit which is cut off
    thread.c = ((v << std::max((int)n - 1, 0)) & (ebit ? 0x80 : 0x8000)) != 0;

    v <<= n;
    arg1.write(v, ebit);

    thread.s = (v & (ebit ? 0x80: 0x8000)) != 0;
    if(n == 1)
        //set overflow to true if the sign changed because of the shift
        thread.o =  thread.s != thread.c;

    thread.z = !v;
}


void Operator::shr(Thread& thread, Argument& arg1, const Argument& arg2) {
    const bool ebit = Argument::is8BitOp(arg1, arg2);
    uint16_t v = arg1.read(ebit);
    const uint16_t n = arg2.read();

    //set overflow flag to most significant bit of original
    thread.o = (v & (ebit ? 0x80 : 0x8000)) != 0;
    //set carry to last bit which is cut off
    thread.c = ((v >> std::max((int)n - 1, 0)) & (0x0001)) != 0;

    v >>= n;
    arg1.write(v, ebit);

    thread.z = !v;
    thread.s = (v & (ebit ? 0x80 : 0x8000)) != 0;
}


void Operator::sub(Thread& thread, Argument& arg1, const Argument& arg2) {
    const bool ebit = Argument::is8BitOp(arg1, arg2);

    const uint16_t arg1v = arg1.read(ebit);
    const uint16_t arg2v = arg2.read(ebit);

    const bool arg1s = IS_SIGNED(arg1v);
    const bool arg2s = !IS_SIGNED(arg2v); //because of the implicit subtraction

    const uint16_t sum = arg1.write(arg1v + NEGATE(arg2v), ebit);
    const bool sum_sign = IS_SIGNED(sum);

    thread.c = arg1v < arg2v;
    thread.s = sum_sign;
    thread.z = !sum;

    //overflow if both args have same sign and the result has a sign which is the opposite
    thread.o = (arg1s == arg2s && sum_sign != arg1s);
}


void Operator::swp(Argument& arg1, Argument& arg2) {
    arg1.swp(arg2);
}


void Operator::_xor(Thread& thread, Argument& arg1, const Argument& arg2) {
    DUAL_LOGIC_OPERATION(^)
}