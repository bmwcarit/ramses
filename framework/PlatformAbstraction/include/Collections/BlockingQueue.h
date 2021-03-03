//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTAINER_BLOCKINGQUEUE_H
#define RAMSES_CONTAINER_BLOCKINGQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <chrono>

namespace ramses_internal
{
    template <typename T>
    class BlockingQueue final
    {
    public:
        void push(const T& element);
        bool pop(T* pElement, std::chrono::milliseconds timeout = std::chrono::milliseconds{0});
        bool empty() const;

    private:
        std::deque<T> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_condvar;
    };

    template <typename T>
    inline void BlockingQueue<T>::push(const T& element)
    {
        std::lock_guard<std::mutex> g(m_mutex);
        m_queue.push_back(element);
        m_condvar.notify_all();
    }

    template <typename T>
    inline bool BlockingQueue<T>::pop(T* pElement, std::chrono::milliseconds timeout)
    {
        assert(pElement);
        std::unique_lock<std::mutex> l(m_mutex);
        if (timeout == std::chrono::milliseconds{0})
            m_condvar.wait(l, [&](){ return !m_queue.empty(); });
        else
        {
            if (!m_condvar.wait_for(l, timeout, [&](){ return !m_queue.empty(); }))
                return false;
        }
        *pElement = m_queue.front();
        m_queue.pop_front();
        return true;
    }

    template <typename T>
    inline bool BlockingQueue<T>::empty() const
    {
        std::lock_guard<std::mutex> g(m_mutex);
        return m_queue.empty();
    }
}

#endif
