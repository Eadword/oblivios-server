#include <argument.h>
#include <instruction.h>
#include <operator.h>
#include <thread.h>

#include "gtest/gtest.h"

#define CONSTRUCT_ARGS Argument arg1(thread, ram, 1); \
                       Argument arg2(thread, ram, 2);

class OperatorTest : public ::testing::Test {
protected:
    uint8_t ram[0x10] = {
            0x00, 0x00, 0x00, 0x04,
            0x03, 0x11, 0x22, 0x07,
            0x12, 0x53, 0xC3, 0x32,
            0x8F, 0x06, 0x02, 0x02
    };
    Thread thread;

    OperatorTest() {
        thread.ax = 5;
        thread.bx = 23;
        thread.cx = 623;
    }
};

TEST_F(OperatorTest, Add16b) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::IMD);
    { //16bit add
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 9);
        EXPECT_FALSE(thread.s);
        EXPECT_FALSE(thread.o);
        EXPECT_FALSE(thread.c);
        EXPECT_FALSE(thread.z);
    }

    thread.ax = 0xFFFC; //MAX - 3 + 4 leads to a carry
    { //check carry
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 0x0000);
        EXPECT_FALSE(thread.s);
        EXPECT_FALSE(thread.o);
        EXPECT_TRUE(thread.c);
        EXPECT_TRUE(thread.z);
    }

    ram[2] = 0x80; ram[3] = 0x00;
    thread.ax = 0xFFFF; //-1
    { //check overflow and carry
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 0x7FFF);
        EXPECT_FALSE(thread.s);
        EXPECT_TRUE(thread.o);
        EXPECT_TRUE(thread.c);
        EXPECT_FALSE(thread.z);
    }
}

TEST_F(OperatorTest, Add8b) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::IMD);
    { //8bit add
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 9);
        EXPECT_FALSE(thread.s);
        EXPECT_FALSE(thread.o);
        EXPECT_FALSE(thread.c);
        EXPECT_FALSE(thread.z);
    }

    thread.ax = 0x00FC; //MAX - 3 + 4 leads to a carry
    { //check carry
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 0x0000);
        EXPECT_FALSE(thread.s);
        EXPECT_FALSE(thread.o);
        EXPECT_TRUE(thread.c);
        EXPECT_TRUE(thread.z);
    }

    ram[2] = 0x00; ram[3] = 0x80;
    thread.ax = 0x00FF; //-1
    { //check overflow and carry
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 0x007F);
        EXPECT_FALSE(thread.s);
        EXPECT_TRUE(thread.o);
        EXPECT_TRUE(thread.c);
        EXPECT_FALSE(thread.z);
    }
}

TEST_F(OperatorTest, AddRAM) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::PIMD);
    //IMD points to the value 0x0311
    { //16bit add
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 0x0316);
        EXPECT_FALSE(thread.s);
        EXPECT_FALSE(thread.o);
        EXPECT_FALSE(thread.c);
        EXPECT_FALSE(thread.z);
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CL, Location::PIMD);
    { //8bit add
        CONSTRUCT_ARGS;
        Operator::add(thread, arg1, arg2);
        EXPECT_EQ(arg1.read(), 0x0072);
        EXPECT_FALSE(thread.s);
        EXPECT_FALSE(thread.o);
        EXPECT_FALSE(thread.c);
        EXPECT_FALSE(thread.z);
    }
}

TEST_F(OperatorTest, Div8b) {
    Instruction::constructInstruction(ram, 0, OPCode::IDIV, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CL, Location::AX);
    thread.cx = 10;
    thread.ax = 30000;
    {
        CONSTRUCT_ARGS;
        Operator::div(thread, arg1);
        EXPECT_TRUE(thread.o);
    }

    thread.cx = 176;
    thread.ax = 7000;
    {
        CONSTRUCT_ARGS;
        Operator::div(thread, arg1);
        EXPECT_EQ(Thread::readLow(thread.ax), 39);
        EXPECT_EQ(Thread::readHigh(thread.ax), 136);
        EXPECT_FALSE(thread.o);
    }
    thread.cx = 0;
    {
        CONSTRUCT_ARGS;
        EXPECT_THROW(Operator::div(thread, arg1), std::runtime_error);
    }
}

TEST_F(OperatorTest, Div16b) {
    Instruction::constructInstruction(ram, 0, OPCode::IDIV, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CX, Location::AX);
    thread.cx = 10;
    thread.bx = 0x1000; thread.ax = 0x0000; //268435456
    {
        CONSTRUCT_ARGS;
        Operator::div(thread, arg1);
        EXPECT_TRUE(thread.o);
    }

    thread.cx = 0xFFFF; //65535
    thread.bx = 0xF646; thread.ax = 0x0000; //4131782656
    {
        CONSTRUCT_ARGS;
        Operator::div(thread, arg1);
        EXPECT_EQ(thread.ax, 0xF646); //63046
        EXPECT_EQ(thread.bx, 0xF646); //63046
        EXPECT_FALSE(thread.o);
    }
    thread.cx = 0;
    {
        CONSTRUCT_ARGS;
        EXPECT_THROW(Operator::div(thread, arg1), std::runtime_error);
    }
}

TEST_F(OperatorTest, Idiv8b) {
    Instruction::constructInstruction(ram, 0, OPCode::IDIV, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CL, Location::AX);
    thread.cx = 10;
    thread.ax = 30000;
    {
        CONSTRUCT_ARGS;
        Operator::idiv(thread, arg1);
        EXPECT_TRUE(thread.o);
    }

    thread.cx = 0xFFB0; //-80
    thread.ax = 7000;
    {
        CONSTRUCT_ARGS;
        Operator::idiv(thread, arg1);
        EXPECT_EQ(Thread::readLow(thread.ax), 0xA9); //-87.5 -> -87
        EXPECT_EQ(Thread::readHigh(thread.ax), 40);
        EXPECT_FALSE(thread.o);
    }
    thread.cx = 0;
    {
        CONSTRUCT_ARGS;
        EXPECT_THROW(Operator::idiv(thread, arg1), std::runtime_error);
    }
}

TEST_F(OperatorTest, Idiv16b) {
    Instruction::constructInstruction(ram, 0, OPCode::IDIV, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CX, Location::AX);
    thread.cx = 10;
    thread.bx = 0x1000; thread.ax = 0x0000; //268435456
    {
        CONSTRUCT_ARGS;
        Operator::idiv(thread, arg1);
        EXPECT_TRUE(thread.o);
    }

    thread.cx = 0xA3A5; //-23643
    thread.bx = 0xF646; thread.ax = 0x0000; //-163184640
    {
        CONSTRUCT_ARGS;
        Operator::idiv(thread, arg1);
        EXPECT_EQ(thread.ax, 0x1AF6); //6902
        EXPECT_EQ(thread.bx, 0xFD72); //-654
        EXPECT_FALSE(thread.o);
    }
    thread.cx = 0;
    {
        CONSTRUCT_ARGS;
        EXPECT_THROW(Operator::idiv(thread, arg1), std::runtime_error);
    }
}

TEST_F(OperatorTest, Imul8b) {
    Instruction::constructInstruction(ram, 0, OPCode::IMUL, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AH, Location::AX);
    thread.ax |= 0xF200; //set AH to -14
    Argument arg(thread, ram);
    Operator::imul(thread, arg);

    EXPECT_EQ(Thread::readLow(thread.ax), 0xBA); //-70
    EXPECT_EQ(Thread::readHigh(thread.ax), 0xF2);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);

    Operator::imul(thread, arg);
    EXPECT_EQ(thread.ax, 0x03D4); //980
    EXPECT_TRUE(thread.o);
    EXPECT_TRUE(thread.c);
}

TEST_F(OperatorTest, Imul16b) {
    Instruction::constructInstruction(ram, 0, OPCode::IMUL, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);

    thread.bx = 0xEFED; //-4115
    Argument arg(thread, ram);
    Operator::imul(thread, arg);

    EXPECT_EQ(thread.ax, 0xAFA1); //-20575
    EXPECT_EQ(thread.bx, 0xEFED);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);

    Operator::imul(thread, arg);
    EXPECT_EQ(thread.ax, 0xE70D);
    EXPECT_EQ(thread.bx, 0x050B);
    EXPECT_TRUE(thread.o);
    EXPECT_TRUE(thread.c);
}

TEST_F(OperatorTest, Mul8b) {
    Instruction::constructInstruction(ram, 0, OPCode::MUL, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PIMD, Location::AX);
    ram[3] = 11; //0x32
    Argument arg(thread, ram);
    Operator::mul(thread, arg);

    EXPECT_EQ(Thread::readLow(thread.ax), 0xFA);
    EXPECT_EQ(Thread::readHigh(thread.ax), 0);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);

    Operator::mul(thread, arg);
    EXPECT_EQ(thread.ax, 0x30D4);
    EXPECT_TRUE(thread.o);
    EXPECT_TRUE(thread.c);
}

TEST_F(OperatorTest, Mul16b) {
    Instruction::constructInstruction(ram, 0, OPCode::MUL, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::IMD, Location::AX);
    ram[2] = 0x31; //0x3104
    Argument arg(thread, ram);
    Operator::mul(thread, arg);

    EXPECT_EQ(thread.ax, 0xF514);
    EXPECT_EQ(thread.bx, 23);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);

    Operator::mul(thread, arg);
    EXPECT_EQ(thread.ax, 0xA850);
    EXPECT_EQ(thread.bx, 0x2EEC);
    EXPECT_TRUE(thread.o);
    EXPECT_TRUE(thread.c);
}

TEST_F(OperatorTest, Shl) {
    Instruction::constructInstruction(ram, 0, OPCode::SHL, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::shl(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 736);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    Operator::shl(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 23552);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    Operator::shl(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 32768);
    EXPECT_FALSE(thread.o);
    EXPECT_TRUE(thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_TRUE(thread.s);

    thread.ax = 1;
    Operator::shl(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 0);
    EXPECT_TRUE(thread.o);
    EXPECT_TRUE(thread.c);
    EXPECT_TRUE(thread.z);
    EXPECT_FALSE(thread.s);
}

TEST_F(OperatorTest, Shr) {
    Instruction::constructInstruction(ram, 0, OPCode::SHR, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);
    thread.bx = 47104;
    thread.ax = 1;
    CONSTRUCT_ARGS;
    Operator::shr(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 23552);
    EXPECT_TRUE(thread.o);
    EXPECT_FALSE(thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    thread.ax = 5;
    Operator::shr(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 736);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    thread.ax = 6;
    Operator::shr(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 11);
    EXPECT_FALSE(thread.o);
    EXPECT_TRUE(thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    thread.ax = 4;
    Operator::shr(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 0);
    EXPECT_FALSE(thread.o);
    EXPECT_TRUE(thread.c);
    EXPECT_TRUE(thread.z);
    EXPECT_FALSE(thread.s);
}

TEST_F(OperatorTest, Sub) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::BX);
    CONSTRUCT_ARGS;
    Operator::sub(thread, arg1, arg2);
    EXPECT_EQ(arg1.read(), 0xFFEE);
    EXPECT_TRUE(thread.s);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.c);
    EXPECT_FALSE(thread.z);
}

TEST_F(OperatorTest, Neg8b) {
    Instruction::constructInstruction(ram, 0, OPCode::NEG, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CL, Location::AX);
    CONSTRUCT_ARGS;
    Operator::neg(thread, arg1);
    EXPECT_EQ(Thread::readLow(thread.cx), (uint8_t)(-111));
    EXPECT_EQ(thread.cx, 0x0291); //check that it only negates the specified 8 bits
    EXPECT_TRUE(thread.c);
    EXPECT_FALSE(thread.o);
    EXPECT_TRUE(thread.s);
    EXPECT_FALSE(thread.z);


    Operator::neg(thread, arg1);
    EXPECT_EQ(Thread::readLow(thread.cx), (uint8_t)(111));
    EXPECT_EQ(thread.cx, 623);
    EXPECT_FALSE(thread.s);
}

TEST_F(OperatorTest, Neg16b) {
    Instruction::constructInstruction(ram, 0, OPCode::NEG, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::neg(thread, arg1);
    EXPECT_EQ(thread.ax, (uint16_t)(-5));
    EXPECT_TRUE(thread.s);
    EXPECT_TRUE(thread.c);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.z);

    Operator::neg(thread, arg1);
    EXPECT_EQ(thread.ax, (uint16_t)(5));
    EXPECT_FALSE(thread.s);
}

TEST_F(OperatorTest, NegRAM) {
    Instruction::constructInstruction(ram, 0, OPCode::NEG, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::neg(thread, arg1);
    EXPECT_EQ(ram[5], (uint8_t)(-0x11));
    EXPECT_EQ(ram[4], 0x03);
    EXPECT_EQ(ram[6], 0x22);
    EXPECT_TRUE(thread.s);
    EXPECT_TRUE(thread.c);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.z);
}

TEST_F(OperatorTest, NegEdge) {
    Instruction::constructInstruction(ram, 0, OPCode::NEG, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);
    CONSTRUCT_ARGS;
    thread.bx = 0;
    Operator::neg(thread, arg1);
    EXPECT_EQ(thread.bx, 0);
    EXPECT_FALSE(thread.c);
    EXPECT_TRUE(thread.z);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.s);

    thread.bx = 0x8000;
    Operator::neg(thread, arg1);
    EXPECT_TRUE(thread.o);
    EXPECT_TRUE(thread.s);
    EXPECT_TRUE(thread.c);
    EXPECT_FALSE(thread.z);
}

TEST_F(OperatorTest, Not8b) {
    Instruction::constructInstruction(ram, 0, OPCode::NOT, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CH, Location::AX);
    CONSTRUCT_ARGS;
    Operator::_not(arg1);
    EXPECT_EQ(Thread::readHigh(thread.cx), 0xFD);
    EXPECT_EQ(Thread::readLow(thread.cx), 0x6F);
}

TEST_F(OperatorTest, Not16b) {
    Instruction::constructInstruction(ram, 0, OPCode::NOT, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::_not(arg1);
    EXPECT_EQ(thread.bx, 0xFFE8);
}

TEST_F(OperatorTest, NotRAM) {
    Instruction::constructInstruction(ram, 0, OPCode::NOT, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::_not(arg1);
    EXPECT_EQ(ram[5], 0xEE);
    EXPECT_EQ(ram[4], 0x03);
    EXPECT_EQ(ram[6], 0x22);
}

TEST_F(OperatorTest, And8b) {
    {
        Instruction::constructInstruction(ram, 0, OPCode::AND, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::CL, Location::CH);

        CONSTRUCT_ARGS;
        Operator::_and(thread, arg1, arg2);
        EXPECT_EQ(thread.cx, 0x0202);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_FALSE(thread.z);
        EXPECT_FALSE(thread.s);
    }
    {
        Instruction::constructInstruction(ram, 0, OPCode::AND, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::CL, Location::AL);

        CONSTRUCT_ARGS;
        thread.cx = 0x0082;
        thread.ax = 0x008F;
        Operator::_and(thread, arg1, arg2);
        EXPECT_EQ(thread.cx, 0x0082);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_TRUE(thread.s);
        EXPECT_FALSE(thread.z);
    }
}

TEST_F(OperatorTest, And16b) {
    Instruction::constructInstruction(ram, 0, OPCode::AND, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);

    CONSTRUCT_ARGS;
    Operator::_and(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 5);
    EXPECT_FALSE(thread.o | thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    thread.bx = 0xFFFF;
    thread.ax = 0xA782;
    Operator::_and(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 0xA782);
    EXPECT_FALSE(thread.o | thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_TRUE(thread.s);
}

TEST_F(OperatorTest, AndRAM) {
    {
        Instruction::constructInstruction(ram, 0, OPCode::AND, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::PAX, Location::BL);

        CONSTRUCT_ARGS;
        thread.bx = 0;
        Operator::_and(thread, arg1, arg2);
        EXPECT_EQ(ram[5], 0x00);
        EXPECT_EQ(ram[4], 0x03);
        EXPECT_EQ(ram[6], 0x22);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_TRUE(thread.z);
        EXPECT_FALSE(thread.s);
    }
    {
        Instruction::constructInstruction(ram, 0, OPCode::AND, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::PAX, Location::BX);

        CONSTRUCT_ARGS;
        thread.bx = 0;
        Operator::_and(thread, arg1, arg2);
        EXPECT_EQ(ram[5], 0x00);
        EXPECT_EQ(ram[6], 0x00);
        EXPECT_EQ(ram[4], 0x03);
        EXPECT_EQ(ram[7], 0x07);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_TRUE(thread.z);
        EXPECT_FALSE(thread.s);
    }
}

TEST_F(OperatorTest, Or8b) {
    {
        Instruction::constructInstruction(ram, 0, OPCode::OR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::CL, Location::CH);

        CONSTRUCT_ARGS;
        Operator::_or(thread, arg1, arg2);
        EXPECT_EQ(thread.cx, 0x026F);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_FALSE(thread.z);
        EXPECT_FALSE(thread.s);
    }
    {
        Instruction::constructInstruction(ram, 0, OPCode::OR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::CL, Location::AL);

        CONSTRUCT_ARGS;
        thread.cx = 0x0090;
        thread.ax = 0x008F;
        Operator::_or(thread, arg1, arg2);
        EXPECT_EQ(thread.cx, 0x009F);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_TRUE(thread.s);
        EXPECT_FALSE(thread.z);
    }
}

TEST_F(OperatorTest, Or16b) {
    Instruction::constructInstruction(ram, 0, OPCode::OR, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);

    thread.bx = 24;
    CONSTRUCT_ARGS;
    Operator::_or(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 29);
    EXPECT_FALSE(thread.o | thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    thread.bx = 0xFFFF;
    thread.ax = 0xA782;
    Operator::_or(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 0xFFFF);
    EXPECT_FALSE(thread.o | thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_TRUE(thread.s);
}

TEST_F(OperatorTest, OrRAM) {
    {
        Instruction::constructInstruction(ram, 0, OPCode::OR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::PAX, Location::BL);

        CONSTRUCT_ARGS;
        Operator::_or(thread, arg1, arg2);
        EXPECT_EQ(ram[5], 0x17);
        EXPECT_EQ(ram[4], 0x03);
        EXPECT_EQ(ram[6], 0x22);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_FALSE(thread.z);
        EXPECT_FALSE(thread.s);
    }
    {
        Instruction::constructInstruction(ram, 0, OPCode::OR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::PAX, Location::BX);

        CONSTRUCT_ARGS;
        thread.bx = 0xFFFF;
        Operator::_or(thread, arg1, arg2);
        EXPECT_EQ(ram[5], 0xFF);
        EXPECT_EQ(ram[6], 0xFF);
        EXPECT_EQ(ram[4], 0x03);
        EXPECT_EQ(ram[7], 0x07);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_FALSE(thread.z);
        EXPECT_TRUE(thread.s);
    }
}

TEST_F(OperatorTest, Xor8b) {
    {
        Instruction::constructInstruction(ram, 0, OPCode::XOR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::CL, Location::CH);

        CONSTRUCT_ARGS;
        Operator::_xor(thread, arg1, arg2);
        EXPECT_EQ(thread.cx, 0x026D);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_FALSE(thread.z);
        EXPECT_FALSE(thread.s);
    }
    {
        Instruction::constructInstruction(ram, 0, OPCode::XOR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::CL, Location::AL);

        CONSTRUCT_ARGS;
        thread.cx = 0x0070;
        thread.ax = 0x008F;
        Operator::_xor(thread, arg1, arg2);
        EXPECT_EQ(thread.cx, 0x00ff);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_TRUE(thread.s);
        EXPECT_FALSE(thread.z);
    }
}

TEST_F(OperatorTest, Xor16b) {
    Instruction::constructInstruction(ram, 0, OPCode::XOR, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::_xor(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 18);
    EXPECT_FALSE(thread.o | thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);

    thread.bx = 0x0FFF;
    thread.ax = 0xA782;
    Operator::_xor(thread, arg1, arg2);
    EXPECT_EQ(thread.bx, 0xA87D);
    EXPECT_FALSE(thread.o | thread.c);
    EXPECT_FALSE(thread.z);
    EXPECT_TRUE(thread.s);
}

TEST_F(OperatorTest, XorRAM) {
    {
        Instruction::constructInstruction(ram, 0, OPCode::XOR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::PAX, Location::BL);

        thread.bx = 0x11;
        CONSTRUCT_ARGS;
        Operator::_xor(thread, arg1, arg2);
        EXPECT_EQ(ram[5], 0x00);
        EXPECT_EQ(ram[4], 0x03);
        EXPECT_EQ(ram[6], 0x22);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_TRUE(thread.z);
        EXPECT_FALSE(thread.s);
    }
    {
        Instruction::constructInstruction(ram, 0, OPCode::XOR, AccessMode::DIRECT, AccessMode::DIRECT,
                                          Location::PAX, Location::BX);

        CONSTRUCT_ARGS;
        thread.bx = 0xFFFF;
        Operator::_xor(thread, arg1, arg2);
        EXPECT_EQ(ram[5], 0xFF);
        EXPECT_EQ(ram[6], 0xDD);
        EXPECT_EQ(ram[4], 0x03);
        EXPECT_EQ(ram[7], 0x07);
        EXPECT_FALSE(thread.o | thread.c);
        EXPECT_FALSE(thread.z);
        EXPECT_TRUE(thread.s);
    }
}

TEST_F(OperatorTest, Inc8b) {
    Instruction::constructInstruction(ram, 0, OPCode::INC, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CH, Location::AX);
    CONSTRUCT_ARGS;
    thread.cx = 0xFFFF;
    Operator::inc(thread, arg1);
    EXPECT_EQ(thread.cx, 0x00FF);
    EXPECT_TRUE(thread.o);
    EXPECT_FALSE(thread.s);
    EXPECT_TRUE(thread.z);
}

TEST_F(OperatorTest, Inc16b) {
    Instruction::constructInstruction(ram, 0, OPCode::INC, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::inc(thread, arg1);
    EXPECT_EQ(thread.bx, 24);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);
}

TEST_F(OperatorTest, IncRAM) {
    Instruction::constructInstruction(ram, 0, OPCode::INC, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::inc(thread, arg1);
    EXPECT_EQ(ram[5], 0x12);
    EXPECT_EQ(ram[4], 0x03);
    EXPECT_EQ(ram[6], 0x22);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);
}

TEST_F(OperatorTest, Dec8b) {
    Instruction::constructInstruction(ram, 0, OPCode::DEC, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CH, Location::AX);
    CONSTRUCT_ARGS;
    thread.cx = 0x0000;
    Operator::dec(thread, arg1);
    EXPECT_EQ(thread.cx, 0xFF00);
    EXPECT_TRUE(thread.o);
    EXPECT_TRUE(thread.s);
    EXPECT_FALSE(thread.z);
}

TEST_F(OperatorTest, Dec16b) {
    Instruction::constructInstruction(ram, 0, OPCode::DEC, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::BX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::dec(thread, arg1);
    EXPECT_EQ(thread.bx, 22);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);
}

TEST_F(OperatorTest, DecRAM) {
    Instruction::constructInstruction(ram, 0, OPCode::DEC, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::AX);
    CONSTRUCT_ARGS;
    Operator::dec(thread, arg1);
    EXPECT_EQ(ram[5], 0x10);
    EXPECT_EQ(ram[4], 0x03);
    EXPECT_EQ(ram[6], 0x22);
    EXPECT_FALSE(thread.o);
    EXPECT_FALSE(thread.z);
    EXPECT_FALSE(thread.s);
}