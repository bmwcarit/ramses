//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TIMEOUTRUNNABLE_H
#define RAMSES_TIMEOUTRUNNABLE_H

#include "PlatformAbstraction/PlatformThread.h"

class TimeoutRunnable : public ramses_internal::Runnable
{
public:
    TimeoutRunnable(std::atomic<bool>& rendererFinished)
        : m_rendererFinished(rendererFinished)
    {
    }

    void run() override
    {
        const ramses_internal::UInt32 timeoutTimeInMillis = 1000u;
        ramses_internal::PlatformThread::Sleep(timeoutTimeInMillis);
        if (!m_rendererFinished)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Test timeout. Will exit with code 1\n");
            exit(1);
        }
    }
private:
    std::atomic<bool>& m_rendererFinished;
};

#endif
