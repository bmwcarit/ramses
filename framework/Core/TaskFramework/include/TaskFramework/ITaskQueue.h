//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ITASKQUEUE_H
#define RAMSES_ITASKQUEUE_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    class ITask;
    class ITaskExecutionObserver;

    class ITaskQueue
    {
    public:
        virtual ~ITaskQueue(){};

        virtual Bool enqueue(ITask& Task) = 0;
        virtual void enableAcceptingTasks() = 0;
        virtual void disableAcceptingTasksAfterExecutingCurrentQueue() = 0;
    };
}

#endif
