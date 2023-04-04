//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string>

namespace rlogic
{
    class LogicObject;

    /**
     * #EErrorType helps distinguish between different types of errors in #rlogic::ErrorData
     */
    enum class EErrorType : int
    {
        BinaryDataAccessError,  ///< Error during attempts to read or write files, non-existing paths, or data corruption (truncation, bit flips etc)
        BinaryVersionMismatch,  ///< The binary data was created with an incompatible version of the runtime(s) - either logic or ramses
        ContentStateError,      ///< The logic engine content is in an invalid state
        RuntimeError,           ///< There was an error during update(), e.g. a RamsesBinding failed to pass its values to Ramses, or Lua script's run() failed
        IllegalArgument,        ///< A call to the Ramses Logic API with missing arguments or incorrect values provided by user code
        LuaSyntaxError,         ///< Lua syntax error, e.g. when creating scripts from syntactically incorrect Lua source code
        Other,                  ///< Error does not fit in any of the above clusters
    };

    /**
     * Holds information about an error which occured during #rlogic::LogicEngine API calls
     */
    struct ErrorData
    {
        /**
         * Error description as human-readable text. For Lua errors, an extra stack
         * trace is contained in the error string with new-line separators.
        */
        std::string message;

        /**
         * Semantic type of the error
        */
        EErrorType type;

        /**
         * The #rlogic::LogicObject which caused the issue. Can be nullptr if the issue was not originating from a specific object.
         */
        const LogicObject* object;
    };
}
