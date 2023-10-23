//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/TaskFramework/ITask.h"

namespace ramses::internal
{
    class ITaskFinishHandler;

    class TaskFinishHandlerDecorator : public ITask
    {
    public:
        TaskFinishHandlerDecorator(ITaskFinishHandler& finishHandler, ITask& TaskToWatch);
        ~TaskFinishHandlerDecorator() override;

        void execute() override;

    protected:

    private:
        ITaskFinishHandler& m_finisHandler;
        ITask& m_watchedTask;
    };
}
