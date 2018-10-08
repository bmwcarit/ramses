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

#ifndef RAMSES_CAPU_WINDOWS_CONDVAR_H
#define RAMSES_CAPU_WINDOWS_CONDVAR_H

#include "ramses-capu/os/Time.h"
#include "ramses-capu/Config.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/os/Mutex.h"

namespace ramses_capu
{
    namespace os
    {
        class CondVar
        {
        public:
            CondVar()
            {
                InitializeConditionVariable(&mCond);
            }

            ~CondVar()
            {
            }

            status_t signal()
            {
                WakeConditionVariable(&mCond);
                return CAPU_OK;
            }

            status_t broadcast()
            {
                WakeAllConditionVariable(&mCond);
                return CAPU_OK;
            }

            template<class Mutex>
            status_t wait(Mutex& mutex, uint32_t timeoutMillis)
            {
                if (timeoutMillis != 0)
                {
                    BOOL ret = SleepConditionVariableCS(&mCond, &mutex.mLock, timeoutMillis);

                    if (ret != 0)
                    {
                        return CAPU_OK;
                    }

                    uint32_t errorCode = GetLastError();

                    if (errorCode == ERROR_TIMEOUT)
                    {
                        return CAPU_ETIMEOUT;
                    }

                    return CAPU_ERROR;
                }
                else
                {
                    BOOL ret = SleepConditionVariableCS(&mCond, &mutex.mLock, INFINITE);

                    if (ret != 0)
                    {
                        return CAPU_OK;
                    }
                    else
                    {
                        return CAPU_ERROR;
                    }
                }
            }
        private:
            CONDITION_VARIABLE mCond;
        };
    }
}
#endif // RAMSES_CAPU_WINDOWS_CONDVAR_H
