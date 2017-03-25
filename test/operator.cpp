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