//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IBLOCKINGTASKQUEUE_H
#define RAMSES_IBLOCKINGTASKQUEUE_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    // class forward declarations
    class ITask;
    class ITaskExecutionObserver;

    class IBlockingTaskQueue
    {

    public:
        virtual ~IBlockingTaskQueue();

        virtual void addTask(ITask* taskToAdd) = 0;

        virtual ITask* popTask(UInt32 timeoutMillis = 0) = 0;

        virtual Bool isEmpty() /*const*/ = 0;
    };

    inline IBlockingTaskQueue::~IBlockingTaskQueue()
    {
    }
}

#endif
