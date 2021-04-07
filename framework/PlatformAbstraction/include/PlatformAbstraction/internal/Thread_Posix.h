//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_THREAD_POSIX_H
#define RAMSES_PLATFORMABSTRACTION_THREAD_POSIX_H

#include "PlatformAbstraction/Macros.h"
#include <thread>
#include <functional>
#include <cassert>
#include <pthread.h>

#define RAMSES_INTEGRITY_DEFAULT_THREAD_STACK_SIZE 0x7D000    // stack size for all integrity threads in bytes (512k)

#if defined(__ghs__)
#include "Utils/LogMacros.h"
#include "absl/strings/string_view.h"

inline void setThreadPriorityIntegrity(int priority, absl::string_view threadname)
{
    const Error ret = SetPriorityAndWeight(CurrentTask(), priority, 1, true);
    if (ret != Success)
        LOG_ERROR(ramses_internal::CONTEXT_FRAMEWORK, "setThreadPriorityIntegrity: " << threadname.data() << " setting thread priority failed:" << ret);
    else
        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "setThreadPriorityIntegrity: " << threadname.data() << " set thread priority to:" << priority);
}

inline void setThreadCoreBindingIntegrity(int subset, absl::string_view threadname)
{
    // bind the thread to the passed processor (core) subset
    const Error ret = SetTaskProcessorSubsetBinding(CurrentTask(), true, subset);
    if (ret != Success)
        LOG_ERROR(ramses_internal::CONTEXT_FRAMEWORK, "setThreadCoreBindingIntegrity: " << threadname.data() << " setting thread core binding failed:" << ret);
    else
        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "setThreadCoreBindingIntegrity: " << threadname.data() << " bind thread to processor subset:" << subset);
}
#endif

namespace ramses_internal
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

        bool joinable() const;
        void join();

        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

    private:
        static void* Run(void*);

        bool m_running = false;
        pthread_t m_threadId{};
        std::unique_ptr<Fun_t> m_fun;
    };

    static_assert(std::is_nothrow_move_constructible<Thread>::value, "Thread must be movable");
    static_assert(std::is_nothrow_move_assignable<Thread>::value, "Thread must be movable");

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
#ifdef __INTEGRITY
                              (void)name;
#else
                              pthread_setname_np(pthread_self(), name.c_str());
#endif
                              userfun();
                          }))
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
#ifdef __INTEGRITY
        if (pthread_attr_setstacksize(&attr, RAMSES_INTEGRITY_DEFAULT_THREAD_STACK_SIZE) == -1)
            printf("Error while setting stack size for Integrity thread\n");

        pthread_attr_setthreadname(&attr, name.c_str());

        // TODO: understand this
        pthread_t currentThreadId = 0;
        PosixEnableCurrentTask(&currentThreadId);
#endif
        if (pthread_create(&m_threadId, &attr, Thread::Run, m_fun.get()) == 0)
            m_running = true;
        else
            m_fun = nullptr;
        pthread_attr_destroy(&attr);
    }

    inline void* Thread::Run(void* arg)
    {
        assert(arg);
        Fun_t* fun = reinterpret_cast<Fun_t*>(arg);
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

#endif
