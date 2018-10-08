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

#ifndef RAMSES_CAPU_CONDVAR_H
#define RAMSES_CAPU_CONDVAR_H

#include "ramses-capu/Config.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/os/Mutex.h"
#include "ramses-capu/os/LightweightMutex.h"
#include "ramses-capu/os/PlatformInclude.h"

#include <type_traits>

#include RAMSES_CAPU_PLATFORM_INCLUDE(CondVar)

namespace ramses_capu
{
    /**
     * Condition variables for concurrent usages.
     */
    class CondVar: private os::CondVar
    {
    public:

        /**
         * Wake up single thread that is waiting for this condition variable
         * @return CAPU_OK if the condition variable is correctly signaled
         *         CAPU_ERROR otherwise
         */
        status_t signal();

        /**
         * Wait for a condition variable
         * @param mutex The mutex to use for locked access
         * @param millisec Milliseconds to wait (default '0' is infinite)
         * @return CAPU_OK if the condition variable is correctly waited
         *         CAPU_ERROR otherwise
         */
        template<class Mutex>
        status_t wait(Mutex& mutex, uint32_t millisec = 0);

        /**
         * Wake up all threads that is waiting for this condition variable
         * @return CAPU_OK if the condition variable is correctly broadcasted
         *         CAPU_ERROR otherwise
         */
        status_t broadcast();

    };

    inline status_t CondVar::signal()
    {
        return os::CondVar::signal();
    }

    template<class Mutex>
    inline status_t CondVar::wait(Mutex& mutex, uint32_t millisec)
    {
        static_assert(std::is_same<Mutex, ramses_capu::Mutex>::value || std::is_same<Mutex, ramses_capu::LightweightMutex>::value, "Unsupported mutex type used in ramses_capu::Cond::wait()");
        return os::CondVar::wait(mutex, millisec);
    }

    inline status_t CondVar::broadcast()
    {
        return os::CondVar::broadcast();
    }
}
#endif //RAMSES_CAPU_CONDVAR_H
