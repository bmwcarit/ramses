/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_TIME_H
#define RAMSES_CAPU_TIME_H

#include "ramses-capu/Config.h"
#include <chrono>

namespace ramses_capu
{
    /**
     * Operating system time functions.
     */
    class Time
    {
    public:
        /**
         * Get monotonic time in milliseconds.
         *
         * Monotonic time is guaranteed to never jump forward or backward. Always use for measuring
         * time differences or when absolute time does not add any value.
         **/
        static uint64_t GetMillisecondsMonotonic();

        /**
         * Get monotonic time in microseconds.
         *
         * Monotonic time is guaranteed to never jump forward or backward. Always use for measuring
         * time differences or when absolute time does not add any value.
         **/
        static uint64_t GetMicrosecondsMonotonic();

        /**
         * Get absolute system time in milliseconds.
         *
         * This time represents the system time since beginning of unix epoch. It is bound to the
         * platforms system clock and can jump forward and backward any time.
         *
         * Only use when time must be compared with reference outside platform, e.g. in logs.
         * Do NOT use when jumping timestamps might cause problems.
         **/
        static uint64_t GetMillisecondsAbsolute();

        /**
         * Get absolute system time in microseconds.
         *
         * This time represents the system time since beginning of unix epoch. It is bound to the
         * platforms system clock and can jump forward and backward any time.
         *
         * Only use when time must be compared with reference outside platform, e.g. in logs.
         * Do NOT use when jumping timestamps might cause problems.
         **/
        static uint64_t GetMicrosecondsAbsolute();
    };

    inline uint64_t Time::GetMillisecondsAbsolute()
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

    inline uint64_t Time::GetMicrosecondsAbsolute()
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    }

    inline uint64_t Time::GetMillisecondsMonotonic()
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

    inline uint64_t Time::GetMicrosecondsMonotonic()
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
    }
}
#endif //RAMSES_CAPU_TIME_H
