#pragma once

class Thread;
class Argument;

#include <json.hpp>
using Json = nlohmann::json;


/**
 * Set of functions useful for working with arguments to apply operations.
 *
 * @see Argument for more information.
 */
namespace Operator {
    /**
     * Perform an ADD operation.
     * @param thread Current thread with registers and flag values.
     * @param arg1 Argument which will store the result and be added.
     * @param arg2 Argument which is being added to the first.
     */
    void add(Thread& thread, Argument& arg1, const Argument& arg2);

    /**
     * Sends an interrupt which is interpreted by the "OS".
     * @param thread Current thread with registers and flag values.
     * @param arg1 Argument storing interrupt code.
     * @param arg2 Possibly read/written depending on interrupt.
     */
    void int_(Thread& thread, Argument& arg1, Argument& arg2);

    /**
     * Moves/copies arg2 into arg1.
     * @note Uses truncation or zero extension if necessary.
     * @param arg1 Argument which will store the value of arg2.
     * @param arg2 Argument which is read for its value.
     */
    void mov(Argument& arg1, const Argument& arg2);

    /**
     * Unsigned multiplication of AX or AL by the argument. If 8bit, it will multiply
     * AL by arg, and save in AH:AL if overflow (overwriting the value in AH).
     * If 16bit, it will multiply AX by arg, and save in BX:AX if overflow,
     * overwriting the value in BX.
     * @note Bitage of arg determines if it is 8bit or 16bit.
     * @param thread Current thread with registers and flag values.
     * @param arg Argument used to multiply by.
     */
    void mul(Thread& thread, const Argument& arg);

    /**
     * Signed multiplication of AX or AL by the argument. If 8bit, it will multiply
     * AL by arg, and save in AH:AL if overflow (overwriting the value in AH).
     * If 16bit, it will multiply AX by arg, and save in BX:AX if overflow,
     * overwriting the value in BX.
     * @note Bitage of arg determines if it is 8bit or 16bit.
     * @param thread Current thread with registers and flag values.
     * @param arg Argument used to multiply by.
     */
    void imul(Thread& thread, const Argument& arg);

    /**
     * Perform two's complement negation.
     * @param thread Current thread with registers and flag values.
     * @param arg Argument which is to be negated.
     */
    void neg(Thread& thread, Argument& arg);

    /**
     * Perform an SUB operation.
     * @param thread Current thread with registers and flag values.
     * @param arg1 Argument which will store the result and be subtracted from.
     * @param arg2 Argument which is subtracted from the first.
     */
    void sub(Thread& thread, Argument& arg1, const Argument& arg2);

    /**
     * Swaps values of the two registers.
     * @param arg1 First argument to be swapped.
     * @param arg2 Second argument to be swapped.
     */
    void swp(Argument& arg1, Argument& arg2);
}