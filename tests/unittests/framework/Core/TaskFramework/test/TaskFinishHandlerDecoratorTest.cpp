//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TaskFinishHandlerDecoratorTest.h"
#include "gmock/gmock.h"
#include "internal/Core/TaskFramework/TaskFinishHandlerDecorator.h"
#include "MockTaskFinishHandler.h"
#include "MockITask.h"

using namespace testing;

namespace ramses::internal
{
    TEST_F(TaskFinishHandlerDecoratorTest, FinisHandlerCalledAfterExecute)
    {
        MockTaskFinishHandler finishHandler;
        MockITask Task;
        TaskFinishHandlerDecorator decorator(finishHandler, Task);

        InSequence seq;
        EXPECT_CALL(Task, execute());
        EXPECT_CALL(finishHandler, TaskFinished(Ref(Task)));

        decorator.execute();
        EXPECT_CALL(Task, destructor());
    }

    TEST_F(TaskFinishHandlerDecoratorTest, AddRefInConstructor)
    {
        MockTaskFinishHandler finishHandler;
        MockITask Task;
        TaskFinishHandlerDecorator decorator(finishHandler, Task);

        EXPECT_EQ(2U, Task.getReferenceCount());
        EXPECT_CALL(Task, destructor());
    }

    TEST_F(TaskFinishHandlerDecoratorTest, DecreaseRefInDestructor)
    {
        MockTaskFinishHandler finishHandler;
        MockITask Task;
        auto* decorator = new TaskFinishHandlerDecorator(finishHandler, Task);
        delete decorator;

        EXPECT_EQ(1U, Task.getReferenceCount());
        EXPECT_CALL(Task, destructor());
    }
}
