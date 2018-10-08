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

#ifndef RAMSES_CAPU_LIGHTWEIGHTMUTEX_H
#define RAMSES_CAPU_LIGHTWEIGHTMUTEX_H

#include "ramses-capu/Error.h"
#include <ramses-capu/os/PlatformInclude.h>

#include RAMSES_CAPU_PLATFORM_INCLUDE(LightweightMutex)

namespace ramses_capu
{
    /**
     * LightweightMutex which can be used for non-recursive locking
     * Depending on the platform it might be faster as the regular mutex
     * Hint: at windows platforms the LightweightMutex is equivalent to the regular mutex
     */
    class LightweightMutex: public ramses_capu::os::LightweightMutex
    {
    public:

        /**
         * Constructor
         */
        LightweightMutex();

        /**
         * Destructor
         */
        ~LightweightMutex();

        LightweightMutex(const LightweightMutex&) = delete;
        LightweightMutex& operator=(const LightweightMutex&) = delete;

        /**
         * used for locking if lock is not currently available, then wait until the lock is captured.
         * results in undefined behavior if already locked in the same thread.
         * @return CAPU_OK if the locking is successful
         *         CAPU_ERROR otherwise
         */
        status_t lock();

        /**
         * it will attempt to lock a mutex. However if the mutex is already locked, the routine will return false immediately
         * @return true if the mutex is successfully locked
         *         false if the mutex is already locked
         */
        bool trylock();

        /**
         *release the lock
         *@return CAPU_OK if the unlocking is successful
         *        CAPU_ERROR otherwise
         */
        status_t unlock();

    };

    inline
    LightweightMutex::LightweightMutex()
    {

    }

    inline
    LightweightMutex::~LightweightMutex()
    {

    }

    inline
    status_t LightweightMutex::lock()
    {
        return ramses_capu::os::LightweightMutex::lock();
    }

    inline
    bool   LightweightMutex::trylock()
    {
        return ramses_capu::os::LightweightMutex::trylock();
    }

    inline
    status_t LightweightMutex::unlock()
    {
        return ramses_capu::os::LightweightMutex::unlock();
    }
}

#endif //RAMSES_CAPU_LIGHTWEIGHTMUTEX_H
