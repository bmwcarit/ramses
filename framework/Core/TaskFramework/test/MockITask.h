//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MOCKITASK_H
#define RAMSES_MOCKITASK_H

#include "framework_common_gmock_header.h"
#include "TaskFramework/ITask.h"

namespace ramses_internal
{
    class TaskHelper
    {
    public:
        TaskHelper()
            : m_Task(nullptr)
        {
        }

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
        ITask* m_Task;
    };

    class MockITask : public ITask
    {
    public:
        MOCK_METHOD0(execute, void());
        MOCK_METHOD0(destructor, void());
        virtual ~MockITask()
        {
            destructor();
        }
    };

}

#endif
