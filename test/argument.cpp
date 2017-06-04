#include <argument.h>
#include <instruction.h>
#include <thread.h>

#include "gtest/gtest.h"

#define CONSTRUCT_ARGS Argument arg1(thread, ram, 1); \
                       Argument arg2(thread, ram, 2);

class ArgumentTest : public ::testing::Test {
protected:
    uint8_t ram[0x10] = {
            0x00, 0x00, 0x00, 0x04,
            0x03, 0x11, 0x22, 0x07,
            0x12, 0x53, 0xC3, 0x32,
            0x8F, 0x06, 0x02, 0x02
    };
    Thread thread;

    ArgumentTest() {
        thread.ax = 5;
        thread.bx = 23;
        thread.cx = 623;
    }
};

TEST_F(ArgumentTest, ReadBasic) {
    Instruction::constructInstruction(ram, 0, OPCode::MOV, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::IMD);
    CONSTRUCT_ARGS;
    EXPECT_EQ(arg1.read(), 5);
    EXPECT_EQ(arg1.read(true), 5);
    EXPECT_EQ(arg2.read(), 0x0004);
    EXPECT_EQ(arg2.read(true), 0x04);
}

TEST_F(ArgumentTest, ReadPtr) {
    Instruction::constructInstruction(ram, 0, OPCode::SUB, AccessMode::DIRECT, AccessMode::RELATIVE,
                                      Location::PAX, Location::PIMD);
    {
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.read(), 0x1122);
        EXPECT_EQ(arg1.read(true), 0x11);
        EXPECT_EQ(arg2.read(), 0x1253);
        EXPECT_EQ(arg2.read(true), 0x12);
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::PIMD);
    {
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg2.read(), 0x0311);
        EXPECT_EQ(arg2.read(true), 0x03);
    }
}

TEST_F(ArgumentTest, ReadTwoImds) {
    Instruction::constructInstruction(ram, 0, OPCode::CMP, AccessMode::RELATIVE, AccessMode::DIRECT,
                                      Location::IMD, Location::IMD);

    CONSTRUCT_ARGS;
    EXPECT_EQ(arg1.read(), 0x0004);
    EXPECT_EQ(arg2.read(), 0x0311);
}

TEST_F(ArgumentTest, Read8) {
    Instruction::constructInstruction(ram, 0, OPCode::DIV, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CL, Location::CH);

    CONSTRUCT_ARGS;
    EXPECT_EQ(arg1.read(), 111);
    EXPECT_EQ(arg2.read(), 2);
    EXPECT_EQ(arg1.read(true), 111);
    EXPECT_EQ(arg2.read(true), 2);
}

TEST_F(ArgumentTest, ReadIncorrectMode) {
    Instruction::constructInstruction(ram, 0, OPCode::MUL, AccessMode::RELATIVE, AccessMode::RELATIVE,
                                      Location::AL, Location::BX);

    CONSTRUCT_ARGS;
    EXPECT_EQ(arg1.read(), 5);
    EXPECT_EQ(arg2.read(), 23);
}


TEST_F(ArgumentTest, WriteReg) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CL, Location::CH);

    const uint16_t bx = thread.bx;
    const uint16_t cx = thread.cx;

    { //Test 8bit registers
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2.read(), false), 2);
        EXPECT_EQ(thread.cx, 514);
        thread.cx = cx;
        EXPECT_EQ(arg1.write(arg2), 2);
        EXPECT_EQ(thread.cx, 514);
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CX, Location::BX);
    { //Test 16bit registers
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2), bx);
        EXPECT_EQ(thread.cx, bx);
        EXPECT_EQ(thread.bx, bx);
        thread.cx = cx;
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::CX, Location::CL);
    { //Test 8bit to 16bit
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2), 111);
        EXPECT_EQ(thread.cx, 111);
        thread.cx = cx;
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::CX);
    { //Test 16bit to 8bit
        thread.ax = 0x8005;
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2), 111);
        EXPECT_EQ(Thread::readLow(thread.ax), 111);
        EXPECT_EQ(Thread::readHigh(thread.ax), 0x80);
    }
}

TEST_F(ArgumentTest, WriteRAM) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::AL);
    {
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2), 5);
        EXPECT_EQ(ram[5], 5);
        EXPECT_EQ(ram[6], 0x22);
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::CX);
    {
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2), 623);
        EXPECT_EQ(ram[5], 2);
        EXPECT_EQ(ram[6], 111);
    }
}

TEST_F(ArgumentTest, WriteImd) {
    ram[3] = 0x01;
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::IMD, Location::BH);
    {
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2), 0);
        EXPECT_EQ(ram[2], 0);
        EXPECT_EQ(ram[3], 0);
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::IMD, Location::CX);
    {
        CONSTRUCT_ARGS;
        EXPECT_EQ(arg1.write(arg2), 623);
        EXPECT_EQ(ram[2], 2);
        EXPECT_EQ(ram[3], 111);
    }
}

TEST_F(ArgumentTest, Is8Bit) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::IMD, Location::CX);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(arg1.is8Bit());
        EXPECT_FALSE(arg2.is8Bit());
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::PBX);
    {
        CONSTRUCT_ARGS;
        EXPECT_TRUE(arg1.is8Bit());
        EXPECT_TRUE(arg2.is8Bit());
    }

}

TEST_F(ArgumentTest, Is8BitOp) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::IMD, Location::CX);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(Argument::is8BitOp(arg1, arg2));
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::CX);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(Argument::is8BitOp(arg1, arg2));
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::IMD, Location::PCX);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(Argument::is8BitOp(arg1, arg2));
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PBX, Location::IMD);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(Argument::is8BitOp(arg1, arg2));
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::AH);
    {
        CONSTRUCT_ARGS;
        EXPECT_TRUE(Argument::is8BitOp(arg1, arg2));
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::PBX);
    {
        CONSTRUCT_ARGS;
        EXPECT_TRUE(Argument::is8BitOp(arg1, arg2));
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::PBX);
    {
        CONSTRUCT_ARGS;
        EXPECT_TRUE(Argument::is8BitOp(arg1, arg2));
    }

}

TEST_F(ArgumentTest, IsMem) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::CX);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(arg1.isMem());
        EXPECT_FALSE(arg2.isMem());
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::IMD);
    {
        CONSTRUCT_ARGS;
        EXPECT_TRUE(arg1.isMem());
        EXPECT_TRUE(arg2.isMem());
    }
}

TEST_F(ArgumentTest, IsReg) {
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::CX);
    {
        CONSTRUCT_ARGS;
        EXPECT_TRUE(arg1.isReg());
        EXPECT_TRUE(arg2.isReg());
    }

    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::IMD);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(arg1.isReg());
        EXPECT_FALSE(arg2.isReg());
    }
}

TEST_F(ArgumentTest, Sign) {
    thread.ax = 10;
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::PAX, Location::IMD);
    {
        CONSTRUCT_ARGS;
        EXPECT_TRUE(arg1.sign()); //0xC332 + 0x0004 = 0xC336
        EXPECT_FALSE(arg2.sign());
    }

    thread.cx = 0x0087;
    Instruction::constructInstruction(ram, 0, OPCode::ADD, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::CL);
    {
        CONSTRUCT_ARGS;
        EXPECT_FALSE(arg1.sign());
        EXPECT_TRUE(arg2.sign());
    }
}

TEST_F(ArgumentTest, Swp) {
    Instruction::constructInstruction(ram, 0, OPCode::SUB, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AX, Location::CX);
    {
        CONSTRUCT_ARGS;
        arg1.swp(arg2);
        EXPECT_EQ(thread.ax, 623);
        EXPECT_EQ(thread.cx, 5);
    }

    thread.ax = 5;
    thread.cx = 623;
    Instruction::constructInstruction(ram, 0, OPCode::SUB, AccessMode::DIRECT, AccessMode::DIRECT,
                                      Location::AL, Location::CX);
    {
        CONSTRUCT_ARGS;
        arg1.swp(arg2);
        EXPECT_EQ(thread.ax, 111);
        EXPECT_EQ(thread.cx, 5);
    }
}