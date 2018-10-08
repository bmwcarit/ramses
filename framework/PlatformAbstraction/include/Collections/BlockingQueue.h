//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTAINER_BLOCKINGQUEUE_H
#define RAMSES_CONTAINER_BLOCKINGQUEUE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformError.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "PlatformAbstraction/PlatformConditionVariable.h"
#include "PlatformAbstraction/PlatformTime.h"
#include <deque>
#include <assert.h>

namespace ramses_internal
{
    template <typename T>
    class BlockingQueue
    {
    public:
        void push(const T& element);
        EStatus pop(T* pElement, UInt32 timeoutMillis = 0);
        Bool empty() const;

    private:
        std::deque<T> m_queue;
        mutable PlatformLock m_mutex;
        PlatformConditionVariable m_condvar;
    };

    template <typename T>
    inline void BlockingQueue<T>::push(const T& element)
    {
        PlatformGuard g(m_mutex);
        m_queue.push_back(element);
        m_condvar.broadcast();
    }

    template <typename T>
    inline EStatus BlockingQueue<T>::pop(T* pElement, UInt32 timeoutMillis)
    {
        assert(pElement);

        PlatformGuard g(m_mutex);
        UInt64 startTime = PlatformTime::GetMillisecondsMonotonic();
        UInt32 spentTime = 0u;
        while (m_queue.empty())
        {
            if (m_condvar.wait(&m_mutex, timeoutMillis - spentTime) == EStatus_RAMSES_TIMEOUT)
            {
                return EStatus_RAMSES_TIMEOUT;
            }

            if (timeoutMillis == 0)
            {
                continue;
            }

            spentTime = static_cast<UInt32>(PlatformTime::GetMillisecondsMonotonic() - startTime);
            if (timeoutMillis <= (spentTime + 1) && m_queue.empty()) // in case we are off by 1ms, we assume a rounding error and time out
            {
                return EStatus_RAMSES_TIMEOUT;
            }
        }

        *pElement = m_queue.front();
        m_queue.pop_front();
        return EStatus_RAMSES_OK;
    }

    template <typename T>
    inline bool BlockingQueue<T>::empty() const
    {
        PlatformGuard g(m_mutex);
        return m_queue.empty();
    }
}

#endif
