//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_THREADINGSYSTEM_H
#define RAMSES_THREADINGSYSTEM_H

#include "TaskFramework/ThreadedTaskExecutor.h"
#include "ThreadWatchdogConfig.h"

namespace ramses_internal
{
    class ThreadingSystem
    {
    public:
        explicit ThreadingSystem(UInt16 numberOfThreads, const ThreadWatchdogConfig& watchdogConfig = ThreadWatchdogConfig())
            : e(numberOfThreads, watchdogConfig)
        {
            e.start();
        }
        ThreadedTaskExecutor e;

        ~ThreadingSystem()
        {
            e.disableAcceptingTasksAfterExecutingCurrentQueue();
            e.stop();
        }
    private:
    };
}

#endif
