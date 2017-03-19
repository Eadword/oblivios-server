#include "operator.h"

#include "argument.h"
#include "thread.h"

void Operator::int_(Thread& thread, Argument& arg1, Argument& arg2) {
    //TODO: handle interrupts
}

void Operator::mov(Argument& arg1, const Argument& arg2) {
    arg1.write(arg2);
}

void Operator::swp(Argument& arg1, Argument& arg2) {
    arg1.swp(arg2);
}

void Operator::add(Thread& thread, Argument& arg1, const Argument& arg2) {
    //TODO: record actions to json
    const uint16_t arg1v = arg1.read();
    const uint16_t arg2v = arg2.read();
    const bool arg1s = arg1.sign();
    const bool arg2s = arg2.sign();

    const uint16_t sum = arg1.write(arg1v + arg2v, arg2.is8Bit());

    thread.c = (sum < arg1v || sum < arg2v);
    thread.s = arg1.sign();
    thread.z = !sum;

    //overflow if both args have same sign and the arg has a sign which is the opposite
    thread.o = (arg1s == arg2s && arg1.sign() != arg1s);
}

void Operator::sub(Thread& thread, Argument& arg1, const Argument& arg2) {
    //TODO: record actions to json
    const uint16_t arg1v = arg1.read();
    const uint16_t arg2v = arg2.read();
    const bool arg1s = arg1.sign();
    const bool arg2s = arg2.sign();

    const uint16_t dif = arg1.write(arg1v - arg2v, arg2.is8Bit());

    thread.c = (dif > arg1v);
    thread.s = arg1.sign();
    thread.z = !dif;

    //overflow if both args have same sign and the result has a sign which is the opposite
    thread.o = (arg1s == arg2s && arg1.sign() != arg1s);
}