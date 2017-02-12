#pragma once

#include <cstdint>

struct Thread;

/**
 * An abstraction to allow handling all arguments no matter where they are in the same way.
 * The thread must be valid when using this class.
 */
class Argument {
    /// m just stores the index into ram, r is a pointer to the register in the thread
    union { uint16_t  m; uint16_t* r; } location;

    /// M refers to scalable memory/RAM, M16 refers to memory which is only 16bits,
    /// R to registers, and NONE only happens if route value is invalid
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
};