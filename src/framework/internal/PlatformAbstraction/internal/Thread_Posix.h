//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Macros.h"
#include "internal/Core/Utils/AssertMovable.h"
#include <thread>
#include <functional>
#include <cassert>
#include <pthread.h>

namespace ramses::internal
{
namespace internal
{
    class Thread
    {
    public:
        using Fun_t = std::function<void()>;

        Thread() = default;
        explicit Thread(const std::string& name, Fun_t fun);

        Thread(Thread&& other) noexcept;
        Thread& operator=(Thread&& other) noexcept;

        ~Thread();

        [[nodiscard]] bool joinable() const;
        void join();

        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

    private:
        static void* Run(void* arg);

        bool m_running = false;
        pthread_t m_threadId{};
        std::unique_ptr<Fun_t> m_fun;
    };

    ASSERT_MOVABLE(Thread)

    inline Thread::Thread(Thread&& other) noexcept
        : m_running(other.m_running)
        , m_threadId(other.m_threadId)
        , m_fun(std::move(other.m_fun))
    {
        other.m_running = false;
    }

    inline Thread::Thread(const std::string& name, Fun_t fun)
        : m_fun(new Fun_t([name = name, userfun = std::move(fun)]()
                          {
#if defined(__APPLE__)
                              pthread_setname_np(name.c_str());
#else
                              pthread_setname_np(pthread_self(), name.c_str());
#endif
                              userfun();
                          }))
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        if (pthread_create(&m_threadId, &attr, Thread::Run, m_fun.get()) == 0)
        {
            m_running = true;
        }
        else
        {
            m_fun = nullptr;
        }
        pthread_attr_destroy(&attr);
    }

    inline void* Thread::Run(void* arg)
    {
        assert(arg);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) must restore original type after going through C
        auto* fun = reinterpret_cast<Fun_t*>(arg);
        (*fun)();
        return nullptr;
    }

    inline Thread::~Thread()
    {
        if (m_running)
            std::terminate();
    }

    inline Thread& Thread::operator=(Thread&& other) noexcept
    {
        if (m_running)
            std::terminate();
        m_running = other.m_running;
        m_threadId = other.m_threadId;
        m_fun = std::move(other.m_fun);
        other.m_running = false;
        return *this;
    }

    inline bool Thread::joinable() const
    {
        return m_running;
    }

    inline void Thread::join()
    {
        assert(m_running);
        if (m_running)
            pthread_join(m_threadId, nullptr);
        m_running = false;
    }
}
}
