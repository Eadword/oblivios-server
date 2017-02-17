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
     * Reads up to 32 bits. If the data stores less, it will pad the most significant bits with 0s.
     * @throws std::runtime_error if the memory is invalid and cannot be read.
     * @return The value read.
     */
    uint32_t read() const;

    /**
     * Writes to this argument with data from src
     * @param src Place to copy from
     * @tparam B the number of bits to copy (8, 16, 32)
     * @throws std::runtime_exception if this argument is read-only
     *
     * @note Uses a Bigendian interpretation of RAM
     *
     * @note
     *  Does not write to most significant digits if this argument is larger than the number
     *  of bits and truncates most significant when it is smaller.
     */
    template<uint8_t B> void write(const Argument& src);
};