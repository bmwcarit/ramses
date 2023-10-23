//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace ramses::internal
{
    class ITask;
    class ITaskExecutionObserver;

    class ITaskQueue
    {
    public:
        virtual ~ITaskQueue() = default;

        virtual bool enqueue(ITask& Task) = 0;
        virtual void disableAcceptingTasksAfterExecutingCurrentQueue() = 0;
    };
}
