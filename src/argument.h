#pragma once
struct Thread;

#include <cstdint>


/**
 * An abstraction to allow handling all arguments no matter where they are in the same way.
 * The thread must be valid when using this class.
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
     * @throws std::runtime_error if the memory is invalid and cannot be read.
     * @return The value read.
     */
    uint16_t read() const;

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
     */
    void write(uint16_t data, const bool memforce8 = true);

    /**
     * Writes to this argument with data from src
     * @see write(uint32_t, uint8_t)
     */
    inline void write(const Argument& src) { write(src.read(), src.isMem()); }

    /**
     * @return True iff the data is stored in RAM, excluding immediate values.
     */
    inline bool isMem() const { return loc_type == M; }

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