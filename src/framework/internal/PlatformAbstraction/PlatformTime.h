//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "synchronized_clock.h"

#include <cstdint>
#include <chrono>

namespace ramses::internal
{
    namespace PlatformTime
    {
        uint64_t GetMillisecondsAbsolute();
        uint64_t GetMicrosecondsAbsolute();

        uint64_t GetMillisecondsMonotonic();
        uint64_t GetMicrosecondsMonotonic();

        uint64_t GetMillisecondsSynchronized();
        uint64_t GetMicrosecondsSynchronized();

        // Due to std::chrono not having overflow protection it is very prone to issues
        // whenever using min/max from chrono because these are likely to overflow when
        // cast to higher precision duration types. And type safe casting is typically why chrono is used
        // in first place.
        // Use this constant instead of using chrono max unless you know what you are doing.
        const constexpr std::chrono::hours InfiniteDuration{ 10u * 365u * 24u }; // 10 years duration

        static_assert(InfiniteDuration.count() == std::chrono::duration_cast<std::chrono::hours>(std::chrono::duration_cast<std::chrono::nanoseconds>(InfiniteDuration)).count(),
            "InfiniteDuration constant overflows if represented in std::chrono::nanoseconds");
    };

    inline uint64_t asMilliseconds(std::chrono::steady_clock::time_point tp)
    {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(tp).time_since_epoch().count();
    }

    inline uint64_t asMicroseconds(std::chrono::steady_clock::time_point tp)
    {
        return std::chrono::time_point_cast<std::chrono::microseconds>(tp).time_since_epoch().count();
    }

    inline uint64_t asMilliseconds(std::chrono::system_clock::time_point tp)
    {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(tp).time_since_epoch().count();
    }

    inline uint64_t asMicroseconds(std::chrono::system_clock::time_point tp)
    {
        return std::chrono::time_point_cast<std::chrono::microseconds>(tp).time_since_epoch().count();
    }

    inline uint64_t PlatformTime::GetMillisecondsAbsolute()
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

    inline uint64_t PlatformTime::GetMicrosecondsAbsolute()
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    }

    inline uint64_t PlatformTime::GetMillisecondsSynchronized()
    {
        auto now = synchronized_clock::now();
        return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

    inline uint64_t PlatformTime::GetMicrosecondsSynchronized()
    {
        auto now = synchronized_clock::now();
        return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    }

    inline uint64_t PlatformTime::GetMillisecondsMonotonic()
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

    inline uint64_t PlatformTime::GetMicrosecondsMonotonic()
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    }
}
