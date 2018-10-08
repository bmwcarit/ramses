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

#ifndef RAMSES_CAPU_TESTUTILS_H
#define RAMSES_CAPU_TESTUTILS_H

#include <gmock/gmock.h>
#include <ramses-capu/os/CondVar.h>
#include <ramses-capu/os/LightweightMutex.h>

namespace ramses_capu
{
    class ThreadSynchronizer
    {
    public:
        ThreadSynchronizer()
            : m_condition(false)
        {
        }

        void wait()
        {
            m_mutex.lock();
            while (!m_condition)
            {
                m_condVar.wait(m_mutex);
            }
            m_mutex.unlock();
        }

        void release()
        {
            m_mutex.lock();
            m_condition = true;
            m_condVar.signal();
            m_mutex.unlock();
        }

    private:
        LightweightMutex m_mutex;
        CondVar m_condVar;
        bool m_condition;
    };

    /**
    * Special GMock actionf or releasing condition variables on mock calls
    */
    ACTION_P(ReleaseSyncCall, syncer)
    {
        UNUSED(arg9);
        UNUSED(arg8);
        UNUSED(arg7);
        UNUSED(arg6);
        UNUSED(arg5);
        UNUSED(arg4);
        UNUSED(arg3);
        UNUSED(arg2);
        UNUSED(arg1);
        UNUSED(arg0);
        UNUSED(args);
        syncer->release();
    }
}

#endif // RAMSES_CAPU_TESTUTILS_H
