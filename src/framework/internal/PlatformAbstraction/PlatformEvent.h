//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
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
    class PlatformEvent final
    {
    public:
        PlatformEvent() = default;

        PlatformEvent(const PlatformEvent&) = delete;
        PlatformEvent& operator=(const PlatformEvent&) = delete;

        void signal();
        bool wait(uint32_t millisec = 0);

    private:
        std::mutex m_mutex;
        std::condition_variable m_condVar;
        bool m_triggered = false;
    };

    inline void PlatformEvent::signal()
    {
        std::lock_guard<std::mutex> g(m_mutex);
        m_triggered = true;
        m_condVar.notify_all();
    }

    inline bool PlatformEvent::wait(uint32_t millisec)
    {
        std::unique_lock<std::mutex> l(m_mutex);
        bool result = true;
        if (millisec == 0)
        {
            m_condVar.wait(l, [&](){ return m_triggered; });
        }
        else
        {
            result = m_condVar.wait_for(l, std::chrono::milliseconds{millisec}, [&]() { return m_triggered; });
        }
        m_triggered = false;
        return result;
    }
}
