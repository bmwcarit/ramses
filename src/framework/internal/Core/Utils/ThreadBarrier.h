//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <condition_variable>

namespace ramses::internal
{
    class ThreadBarrier
    {
    public:
        explicit ThreadBarrier(int num = 0);

        void init_wait_for_num(int num);

        void wait();

    private:
        std::mutex m_lock;
        std::condition_variable m_condVar;
        int m_requiredWaiters{0};
        int m_currentWaiters{0};
    };

    inline ThreadBarrier::ThreadBarrier(int num)
    {
        init_wait_for_num(num);
    }

    inline void ThreadBarrier::init_wait_for_num(int num)
    {
        std::lock_guard<std::mutex> g(m_lock);
        m_requiredWaiters = num;
        m_currentWaiters = 0;
    }

    inline void ThreadBarrier::wait()
    {
        std::unique_lock<std::mutex> l(m_lock);
        if (++m_currentWaiters == m_requiredWaiters)
        {
            m_condVar.notify_all();
        }
        else
        {
            m_condVar.wait(l, [&]() { return m_currentWaiters >= m_requiredWaiters; });
        }
    }
}
