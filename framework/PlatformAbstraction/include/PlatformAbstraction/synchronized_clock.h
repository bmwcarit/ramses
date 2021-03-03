//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYNCHRONIZED_CLOCK_H
#define RAMSES_SYNCHRONIZED_CLOCK_H

#include <chrono>

namespace ramses_internal
{
    enum class synchronized_clock_type
    {
        SystemTime = 0, // for development, or if system time is synchronized (hypervisor, container etc)
        PTP = 1, // precision time protocol - network wide synchronized time
    };

    struct synchronized_clock final
    {
        using duration =  std::chrono::nanoseconds;
        using rep = duration::rep;
        using period = duration::period;
        using time_point = std::chrono::time_point<synchronized_clock, duration>;
        static const bool is_steady = false;

        static time_point now();
        static const char* source();
        static synchronized_clock_type getClockType();
    };

    inline uint64_t asMilliseconds(synchronized_clock::time_point tp)
    {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(tp).time_since_epoch().count();
    }

    inline uint64_t asMicroseconds(synchronized_clock::time_point tp)
    {
        return std::chrono::time_point_cast<std::chrono::microseconds>(tp).time_since_epoch().count();
    }
}

// platform specific implementations
#if defined(__INTEGRITY)
namespace ramses_internal
{
    inline const char* synchronized_clock::source()
    {
        return "gptp";
    }

    inline synchronized_clock_type synchronized_clock::getClockType()
    {
        return synchronized_clock_type::PTP;
    }

}

#elif defined(RAMSES_LINUX_USE_DEV_PTP)
#include <ctime>

namespace ramses_internal
{
    namespace synchronized_clock_impl
    {
        clockid_t GetClockId();
    }

    inline const char* synchronized_clock::source()
    {
        return "/dev/ptp0";
    }

    inline synchronized_clock::time_point synchronized_clock::now()
    {
        static clockid_t clockId = synchronized_clock_impl::GetClockId();
        if (clockId == 0)
        {
            // ptp not supported, warnings already printed by GetClockId
            return time_point(std::chrono::nanoseconds(0));
        }
        struct timespec ts;
        clock_gettime(clockId, &ts);
        return time_point(std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec));
    }

    inline synchronized_clock_type synchronized_clock::getClockType()
    {
        return synchronized_clock_type::PTP;
    }
}

#else
namespace ramses_internal
{
    inline const char* synchronized_clock::source()
    {
        return "std::chrono::system_clock::now()";
    }

    inline synchronized_clock::time_point synchronized_clock::now()
    {
        return time_point(std::chrono::system_clock::now().time_since_epoch());
    }

    inline synchronized_clock_type synchronized_clock::getClockType()
    {
        return synchronized_clock_type::SystemTime;
    }
}
#endif

#endif
