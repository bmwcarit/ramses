//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMCONDITIONVARIABLE_H
#define RAMSES_PLATFORMCONDITIONVARIABLE_H

#include <condition_variable>
#include <cassert>
#include <type_traits>
#include <PlatformAbstraction/PlatformLock.h>
#include <PlatformAbstraction/PlatformError.h>

namespace ramses_internal
{
    class PlatformConditionVariable
    {
    public:
        void signal()
        {
            mCondVar.notify_one();
        }

        void broadcast()
        {
            mCondVar.notify_all();
        }

        template<class Mutex>
        EStatus wait(Mutex* mutex, uint32_t millisec = 0)
        {
            static_assert(std::is_same<Mutex, PlatformLock>::value ||
                          std::is_same<Mutex, PlatformLightweightLock>::value,
                          "Must use PlatformLightweightLock or PlatformLock");
            if (millisec == 0)
            {
                mCondVar.wait(mutex->m_mutex);
                return EStatus_RAMSES_OK;
            }
            else
            {
                switch (mCondVar.wait_for(mutex->m_mutex, std::chrono::milliseconds(millisec)))
                {
                case std::cv_status::no_timeout: return EStatus_RAMSES_OK;
                case std::cv_status::timeout: return EStatus_RAMSES_TIMEOUT;
                }
                assert(false && "invalid value in std::cv_status");
                return EStatus_RAMSES_ERROR;
            }
        }

    private:
        std::condition_variable_any mCondVar;
    };
}

#endif
