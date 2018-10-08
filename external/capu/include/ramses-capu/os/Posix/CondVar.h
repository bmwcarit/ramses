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

#ifndef RAMSES_CAPU_UNIXBASED_CONDVAR_H
#define RAMSES_CAPU_UNIXBASED_CONDVAR_H

#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "ramses-capu/os/Time.h"

namespace ramses_capu
{
    namespace posix
    {

        class CondVar
        {
        public:
            CondVar()
            {
                pthread_condattr_t attr;
                pthread_condattr_init(&attr);
#ifdef OS_LINUX
                pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
#endif
                pthread_cond_init(&mCond, &attr);
                pthread_condattr_destroy(&attr);
            }

            ~CondVar()
            {
                pthread_cond_destroy(&mCond);
            }

            status_t signal()
            {
                if (pthread_cond_signal(&mCond) == 0)
                {
                    return CAPU_OK;
                }
                else
                {
                    return CAPU_ERROR;
                }
            }


            status_t broadcast()
            {
                if (pthread_cond_broadcast(&mCond) == 0)
                {
                    return CAPU_OK;
                }
                else
                {
                    return CAPU_ERROR;
                }
            }

            template<class Mutex>
            status_t wait(Mutex& mutex, uint32_t timeoutMillis)
            {
                if (timeoutMillis != 0)
                {
                    struct timespec timeout;
#ifdef OS_LINUX
                    clock_gettime(CLOCK_MONOTONIC, &timeout);
                    timeout.tv_sec += timeoutMillis / 1000;
                    timeout.tv_nsec += (timeoutMillis % 1000) * 1000000;
                    if (timeout.tv_nsec > 1000000000)
                    {
                        timeout.tv_sec += 1;
                        timeout.tv_nsec -= 1000000000;
                    }
#else
                    uint64_t endTime = Time::GetMillisecondsAbsolute() + timeoutMillis;
                    timeout.tv_sec = endTime / 1000;
                    timeout.tv_nsec = (endTime % 1000) * 1000000;
#endif

                    int32_t ret = pthread_cond_timedwait(&mCond, &mutex.mLock, &timeout);

                    switch (ret)
                    {
                    case 0:
                        return CAPU_OK;
                    case ETIMEDOUT:
                        return CAPU_ETIMEOUT;
                    default:
                        return CAPU_ERROR;
                    }
                }
                else
                {
                    if (pthread_cond_wait(&mCond, &mutex.mLock) == 0)
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
            pthread_cond_t mCond;
        };
    }
}
#endif // RAMSES_CAPU_UNIXBASED_CONDVAR_H
