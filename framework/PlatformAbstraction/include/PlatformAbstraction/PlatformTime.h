//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMTIME_H
#define RAMSES_PLATFORMTIME_H

#include "PlatformTypes.h"
#include <chrono>

namespace ramses_internal
{
    class PlatformTime
    {
    public:
        static UInt64 GetMillisecondsAbsolute();
        static UInt64 GetMicrosecondsAbsolute();

        static UInt64 GetMillisecondsMonotonic();
        static UInt64 GetMicrosecondsMonotonic();
    };

    inline UInt64 PlatformTime::GetMillisecondsAbsolute()
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

    inline UInt64 PlatformTime::GetMicrosecondsAbsolute()
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    }

    inline UInt64 PlatformTime::GetMillisecondsMonotonic()
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

    inline UInt64 PlatformTime::GetMicrosecondsMonotonic()
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    }
}

#endif
