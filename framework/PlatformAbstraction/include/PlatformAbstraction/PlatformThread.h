//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2011 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMTHREAD_H
#define RAMSES_PLATFORMTHREAD_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformError.h>

#include <ramses-capu/os/Thread.h>
#include "Utils/LogMacros.h"
#include "Collections/String.h"


namespace ramses_internal
{
    /**
     * Type definition for a Runnable;
     */
    typedef ramses_capu::Runnable Runnable;

    //typedef long(*ThreadFunc)(void*);

    /**
     * A thread class which performs the thread execution within a supplied runnable.
     * Todo:    Something like thread CPU affinity would be useful here.
     */
    class PlatformThread
    {
    public:
        /**
         * Default constructor. A further call createThread is required in order that the thread knows the runnable.
         */
        PlatformThread(const String& threadName);

        /**
         * Destructor.
         */
        ~PlatformThread();

        /**
         * Start the thread and call the specified runnable instance.
         */
        void start(Runnable& runnable);

        /**
         * Joins the thread. Returns when the thread ends.
         * Todo:    a join function with a timeout value would be useful.
         */
        void join();

        void cancel();

        Bool isRunning() const;

        /**
         * Sleep for the specified amount of milli seconds.
         * @param   msec    The time in milli seconds to sleep.
         * @return  RAMSES_OK if the sleep was successful or an according error code otherwise.
         */
        static EStatus Sleep(UInt32 msec);

    private:
        /**
         * Underlaying CAPU thread.
         */
        ramses_capu::Thread  m_thread;
    };

    inline
    PlatformThread::PlatformThread(const String& threadName)
        : m_thread(threadName.stdRef())
    {
        assert(threadName.indexOf(' ') == -1 && "PlatformThread name may not contain spaces");
        assert(threadName.getLength() < 16u);
    }

    inline
    PlatformThread::~PlatformThread()
    {
    }

    inline
    void PlatformThread::cancel()
    {
        m_thread.cancel();
    }

    inline
    Bool PlatformThread::isRunning() const
    {
        return m_thread.getState() == ramses_capu::TS_RUNNING;
    }

    inline
    void PlatformThread::start(Runnable& runnable)
    {
        const ramses_capu::status_t status = m_thread.start(runnable);
        if (status != ramses_capu::CAPU_OK)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "Could not start platform thread. Error code: " << status << " " << ramses_capu::StatusConversion::GetStatusText(status));
        }
    }

    inline void PlatformThread::join()
    {
        const ramses_capu::status_t status = m_thread.join();
        if (status != ramses_capu::CAPU_OK)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "Could not join platform thread. Error code: " << status << " " << ramses_capu::StatusConversion::GetStatusText(status));
        }
    }

    inline
    EStatus PlatformThread::Sleep(UInt32 msec)
    {
        return static_cast<EStatus>(ramses_capu::Thread::Sleep(msec));
    }
}

#endif
