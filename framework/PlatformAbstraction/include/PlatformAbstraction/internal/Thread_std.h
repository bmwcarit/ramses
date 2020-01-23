//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_THREAD_STD_H
#define RAMSES_PLATFORMABSTRACTION_THREAD_STD_H

#include "PlatformAbstraction/Macros.h"
#include <thread>
#include <functional>

namespace ramses_internal
{
namespace internal
{
    class Thread
    {
    public:
        using Fun_t = std::function<void()>;

        Thread() = default;
        explicit Thread(std::string name, Fun_t fun);

        Thread(Thread&& other) RNOEXCEPT = default;
        Thread& operator=(Thread&& other) RNOEXCEPT = default;

        bool joinable() const;
        void join();

        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

    private:
        std::thread m_thread;
    };

    static_assert(std::is_nothrow_move_constructible<Thread>::value, "Thread must be movable");
    static_assert(std::is_nothrow_move_assignable<Thread>::value, "Thread must be movable");

    inline Thread::Thread(std::string /*name*/, Fun_t fun)
        : m_thread(fun)
    {
    }

    inline bool Thread::joinable() const
    {
        return m_thread.joinable();
    }

    inline void Thread::join()
    {
        m_thread.join();
    }
}
}

#endif
