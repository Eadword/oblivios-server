#pragma once
struct Thread;

#include <cstdint>


/**
 * An abstraction to allow handling all arguments no matter where they are in the same way.
 * The thread must be valid when using this class.
 *
 * @see operator namespace which performs many operations on one or more arguments.
 *
 * @note
 *  An odd result of how this works, is that an 8bit RAM value truncates the lower-order bits, while
 *  registers will truncate the higher-order bits. Immediate types are treated as 16bit registers in
 *  this regard, as their size is garunteed, so we truncate the higher-order bits as is expected.
 */
class Argument {
    /// stores reference to game's ram to reduce number of params
    uint8_t* ram;

    /// m just stores the index into ram, r is a pointer to the register in the thread
    union { uint16_t m; uint16_t* r; } location;

    /**
     * M refers to scalable memory/RAM, M16 refers to memory which is only 16bits,
     * R to registers, and NONE only happens if route value is invalid
     */
    enum { M, M16, R8L, R8H, R16, NONE } loc_type;

    /// true if it should not be written to
    bool read_only;

public:

    /**
     * Check whether an operation on the two arguments will be 8bit; if false it is 16bit.
     * @param arg1 First argument, the destination.
     * @param arg2 Second argument, the source.
     * @return True if the operation is on 8bits, false if it is on 16bits.
     */
    static inline bool is8BitOp(const Argument& arg1, const Argument& arg2);

    //Do not allow default construction
    Argument() = delete;

    /**
     * Constructs an argument using values in thread and RAM. This will store a pointer/reference to
     * the value it wants to keep, but not take ownership of it.
     * @param thread The current thread, includes the IP
     * @param ram A pointer to the beginning of RAM
     * @param argn The number of the argument, starting at 1 (currently only 1 and 2 are valid)
     */
    Argument(Thread& thread, uint8_t* ram, uint8_t argn = 1);

    /**
     * Reads up to 16 bits. If the data stores less, it will pad the most significant bits with 0s.
     * @param force8 Will force reading the value as 8bits rather than 16bits.
     * @throws std::runtime_error if the memory is invalid and cannot be read.
     * @return The value read.
     */
    uint16_t read(bool force8 = false) const;

    /**
     * Writes to this argument the value of data
     * @param src Place to copy from
     * @param memforce8 Forces a write to memory to use only 8 bits instead of 16 bits
     * @throws std::runtime_exception if this argument is read-only
     * @throws std::invalid_argument if number of bits is invalid
     *
     * @note Uses a Bigendian interpretation of RAM
     *
     * @note
     *  Does not write to most significant digits if this argument is larger than the number
     *  of bits and truncates most significant when it is smaller.
     *
     * @return The value written.
     */
    uint16_t write(uint16_t data, bool memforce8 = true);

    /**
     * Writes to this argument with data from src
     * @see write(uint32_t, uint8_t)
     * @return The value written
     */
    inline uint16_t write(const Argument& src);

    /**
     * @return True if the data stored is 8bits and not 16.
     */
    bool is8Bit() const;

    /**
     * @return True if the data is in memory (RAM).
     */
    bool isMem() const;

    /**
     * @return True if the data is in a register.
     */
    bool isReg() const;

    /**
     * Checks the sign of the value.
     * @param force8 Will force reading the value as 8bits rather than 16bits.
     * @return True if negative, else false.
     */
    bool sign(bool force8 = false) const;

    /**
     * Swaps the data stored at the location. This is not a true object swap such as std::swap,
     * rather it is functionality for the SWPx opcodes.
     * @param other The place to swap with
     * @param bits The number of bits to read/write (8, 16, 32)
     * @throws std::runtime_exception if either Argument is read-only or invalid
     * @throws std::invalid_argument if the number of bits is invalid
     */
    void swp(Argument& other);
};



inline uint16_t Argument::write(const Argument& src) {
    return write(src.read(), src.is8Bit());
}


inline bool Argument::is8BitOp(const Argument& arg1, const Argument& arg2) {
    //Will be 16 bit if either
    // 1. Arg1 is 16bit
    // 2. Arg1 is RAM and Arg2 is 16bit
    const bool _16b = (!arg1.is8Bit()) || (arg1.isMem() && !arg2.is8Bit());

    return !_16b;
}
