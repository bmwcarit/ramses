//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2011 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMTHREAD_H
#define RAMSES_PLATFORMTHREAD_H

#include "PlatformAbstraction/Runnable.h"

#include <cassert>
#include <string>
#include <string_view>
#include <exception>

#ifdef _WIN32
#include "PlatformAbstraction/internal/Thread_std.h"
#else
#include "PlatformAbstraction/internal/Thread_Posix.h"
#endif

namespace ramses_internal
{
    class PlatformThread final
    {
    public:
        explicit PlatformThread(std::string_view threadName);
        ~PlatformThread();

        PlatformThread(const PlatformThread&) = delete;
        PlatformThread(PlatformThread&&) = delete;
        PlatformThread& operator=(const PlatformThread&) = delete;
        PlatformThread& operator=(PlatformThread&&) = delete;

        void start(Runnable& runnable);
        void join();

        void cancel();
        [[nodiscard]] bool isRunning() const;
        [[nodiscard]] bool joinable() const;

        static void Sleep(uint32_t msec);

    private:
        std::string m_name;
        internal::Thread  m_thread;
        std::atomic<bool> m_isRunning {false};
        Runnable* m_runnable = nullptr;
    };

    inline
    PlatformThread::PlatformThread(std::string_view threadName)
        : m_name(threadName)
    {
        assert(m_name.find(' ') == std::string::npos && "PlatformThread name may not contain spaces");
        assert(m_name.size() < 16u);
    }

    inline PlatformThread::~PlatformThread()
    {
        if (m_thread.joinable())
            join();
    }

    inline
    void PlatformThread::cancel()
    {
        if (m_runnable)
            m_runnable->cancel();
    }

    inline
    bool PlatformThread::isRunning() const
    {
        return m_isRunning;
    }

    inline
    bool PlatformThread::joinable() const
    {
        return m_thread.joinable();
    }

    inline
    void PlatformThread::start(Runnable& runnable)
    {
        if (m_thread.joinable())
            std::terminate();

        m_runnable = &runnable;
        m_isRunning = true;
        m_thread = internal::Thread(m_name, [&]()
                                            {
                                                m_runnable->run();
                                                m_isRunning = false;
                                            });
    }

    inline void PlatformThread::join()
    {
        if (m_thread.joinable())
            m_thread.join();
        m_runnable = nullptr;
    }

    inline
    void PlatformThread::Sleep(uint32_t msec)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{msec});
    }
}

#endif
