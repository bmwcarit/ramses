//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Core/TaskFramework/ITask.h"
#include "internal/Core/TaskFramework/ITaskQueue.h"

namespace ramses::internal
{
    class MockTaskQueue : public ITaskQueue
    {
    public:
        ~MockTaskQueue() override = default;
        MOCK_METHOD(bool, enqueue, (ITask& Task), (override));
        MOCK_METHOD(void, disableAcceptingTasksAfterExecutingCurrentQueue, (), (override));
    };
}
