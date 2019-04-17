/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_THREAD_H
#define RAMSES_CAPU_THREAD_H

#include "ramses-capu/os/PlatformInclude.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/util/Runnable.h"
#include "ramses-capu/os/ThreadState.h"

#include RAMSES_CAPU_PLATFORM_INCLUDE(Thread)

namespace ramses_capu
{
    /**
     * Class representing a thread.
     */
    class Thread: private ramses_capu::os::Thread
    {
    public:
        /**
         * Create a thread object with an optional name
         *
         * Depending on OS, the name might appear in debuggers, task lists etc
         */
        Thread(const std::string& name = "");

        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

        /**
         * Starts the thread.
         *
         * If the thread was already started it has to be joined to be
         * able to start another thread.
         * @param runnable the runnable which should be executed by the
         *                 new thread.
         * @return CAPU_OK if thread has been started successfully
         *         CAPU_ERROR otherwise
         */
        status_t start(Runnable& runnable);

        /**
         * Waits the thread completeness
         * @return CAPU_OK if thread is currently waiting for completeness or has terminated
         *         CAPU_ERROR otherwise
         */
        status_t join();

        /**
        * Returns the previously set name of the thread
        * @return the previously set name of the thread
        */
        const char* getName();

        /**
         * Sets the cancel flag of the runnable
         */
        void cancel();

        /**
         * Resets the cancel flag of the runnable, it could be run again or continue to run
         */
        void resetCancel();

        /**
         * Return the current thread state
         * @return state of the thread
         */
        ThreadState getState() const;

        /**
         * Suspend the current thread for specific amount of time
         * @return CAPU_OK if thread is currently suspended
         *         CAPU_ERROR otherwise
         */
        static status_t Sleep(uint32_t millis);

        /**
         * Gets the id of the current thread.
         * @return The id of the current thread.
         */
        static uint_t CurrentThreadId();
    };

    inline
    Thread::Thread(const std::string& name)
        : ramses_capu::os::Thread(name)
    {
    }

    inline
    status_t
    Thread::start(Runnable& runnable)
    {
        return ramses_capu::os::Thread::start(runnable);
    }

    inline
    status_t
    Thread::join()
    {
        return ramses_capu::os::Thread::join();
    }

    inline
    const char* Thread::getName()
    {
        return ramses_capu::os::Thread::getName();
    }

    inline
    void
    Thread::cancel()
    {
        ramses_capu::os::Thread::cancel();
    }

    inline
    void
    Thread::resetCancel()
    {
        ramses_capu::os::Thread::resetCancel();
    }

    inline
    ThreadState
    Thread::getState() const
    {
        return ramses_capu::os::Thread::getState();
    }

    inline
    status_t
    Thread::Sleep(uint32_t millis)
    {
        return ramses_capu::os::Thread::Sleep(millis);
    }

    inline
    uint_t
    Thread::CurrentThreadId()
    {
        return ramses_capu::os::Thread::CurrentThreadId();
    }


}

#endif /* RAMSES_CAPU_THREAD_H */
