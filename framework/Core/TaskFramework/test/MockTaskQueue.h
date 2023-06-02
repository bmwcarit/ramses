//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MOCKTASKQUEUE_H
#define RAMSES_MOCKTASKQUEUE_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "TaskFramework/ITask.h"
#include "TaskFramework/ITaskQueue.h"

namespace ramses_internal
{
    class MockTaskQueue : public ITaskQueue
    {
    public:
        ~MockTaskQueue() override = default;
        MOCK_METHOD(bool, enqueue, (ITask& Task), (override));
        MOCK_METHOD(void, disableAcceptingTasksAfterExecutingCurrentQueue, (), (override));
    };
}

#endif
