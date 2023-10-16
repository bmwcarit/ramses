//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/TaskFramework/ITask.h"

namespace ramses::internal
{
    class TaskHelper
    {
    public:
        bool enqueue(ITask& Task)
        {
            m_Task = &Task;
            m_Task->addRef();
            return true;
        }

        void execute()
        {
            assert(m_Task);
            m_Task->execute();
            m_Task->release();
        }
    private:
        ITask* m_Task{nullptr};
    };

    class MockITask : public ITask
    {
    public:
        MOCK_METHOD(void, execute, (), (override));
        MOCK_METHOD(void, destructor, ());
        ~MockITask() override
        {
            destructor();
        }
    };

}
