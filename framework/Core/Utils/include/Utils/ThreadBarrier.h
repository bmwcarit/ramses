//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_THREADBARRIER_H
#define RAMSES_THREADBARRIER_H

#include "PlatformAbstraction/PlatformConditionVariable.h"
#include "PlatformAbstraction/PlatformGuard.h"

namespace ramses_internal
{
    class ThreadBarrier
    {
    public:
        ThreadBarrier(int num = 0);

        void init_wait_for_num(int num);

        void wait();

    private:
        PlatformLightweightLock m_lock;
        PlatformConditionVariable m_condVar;
        int m_requiredWaiters;
        int m_currentWaiters;
    };

    inline ThreadBarrier::ThreadBarrier(int num)
        : m_requiredWaiters(0)
        , m_currentWaiters(0)
    {
        init_wait_for_num(num);
    }

    inline void ThreadBarrier::init_wait_for_num(int num)
    {
        PlatformLightweightGuard g(m_lock);
        m_requiredWaiters = num;
        m_currentWaiters = 0;
    }

    inline void ThreadBarrier::wait()
    {
        PlatformLightweightGuard g(m_lock);
        if (++m_currentWaiters == m_requiredWaiters)
        {
            m_condVar.broadcast();
        }
        else
        {
            while (m_currentWaiters < m_requiredWaiters)
            {
                m_condVar.wait(&m_lock);
            }
        }
    }
}

#endif
