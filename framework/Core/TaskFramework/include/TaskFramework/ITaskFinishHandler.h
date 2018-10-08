//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ITASKFINISHHANDLER_H
#define RAMSES_ITASKFINISHHANDLER_H

namespace ramses_internal
{
    class ITask;

    class ITaskFinishHandler
    {
    public:
        virtual ~ITaskFinishHandler(){}

        virtual void TaskFinished(ITask& Task) = 0;
    };
}

#endif
