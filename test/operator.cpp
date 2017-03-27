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