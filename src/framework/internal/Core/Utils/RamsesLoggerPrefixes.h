//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string_view>

namespace ramses::internal
{
    // This construct is a workaround to solve thread local storage across shared libraries for logging prefixes.
    // When these are called it will set the prefixes in both caller's side library (e.g. shared lib with renderer)
    // and also the core library (shared headless). This is needed to have the correct prefixes in logs for code compiled
    // into core library but invoked from renderer shared library (and vice versa if there was such case).
    // The reason is that each shared library has own copy of thread local storage so the actual value does not only depend
    // on thread but also which library is the log invoked from (this also seems to be the case on all platforms so far).
    class RamsesLoggerPrefixes
    {
    public:
        static void SetRamsesLoggerPrefixes(std::string_view instance, std::string_view thread, std::string_view additional = {});
        static void SetRamsesLoggerPrefixAdditional(std::string_view additional);
    };
}
