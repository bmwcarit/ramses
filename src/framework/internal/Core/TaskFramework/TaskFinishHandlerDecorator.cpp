//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/TaskFramework/TaskFinishHandlerDecorator.h"
#include "internal/Core/TaskFramework/ITaskFinishHandler.h"


namespace ramses::internal
{

    TaskFinishHandlerDecorator::TaskFinishHandlerDecorator(ITaskFinishHandler& TaskFinishHandler, ITask& TaskToWatch)
        : m_finisHandler(TaskFinishHandler)
        , m_watchedTask(TaskToWatch)
    {
        m_watchedTask.addRef();
    }

    TaskFinishHandlerDecorator::~TaskFinishHandlerDecorator()
    {
        m_watchedTask.release();
    }

    void TaskFinishHandlerDecorator::execute()
    {
        m_watchedTask.execute();
        m_finisHandler.TaskFinished(m_watchedTask);
    }
}
