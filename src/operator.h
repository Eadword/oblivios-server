#pragma once

class Thread;
class Argument;

#include <json.hpp>
using Json = nlohmann::json;

namespace Operator {
    /**
     * Perform an ADD operation.
     * @param thread Current thread with flag values.
     * @param arg1 The argument which will store the result and be added.
     * @param arg2 The argument which is being added to the first.
     * @param json Running Json obj to write updates to.
     */
    void add(Thread& thread, Argument& arg1, const Argument& arg2, Json& json);

    /**
     * Perform an SUB operation.
     * @param thread Current thread with flag values.
     * @param arg1 The argument which will store the result and be subtracted from.
     * @param arg2 The argument which is subtracted from the first.
     * @param json Running Json obj to write updates to.
     */
    void sub(Thread& thread, Argument& arg1, const Argument& arg2, Json& json);
}