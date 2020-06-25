//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MOCKTASKFINISHHANDLER_H
#define RAMSES_MOCKTASKFINISHHANDLER_H

#include "framework_common_gmock_header.h"
#include "TaskFramework/ITaskFinishHandler.h"

namespace ramses_internal
{
    class MockTaskFinishHandler : public ITaskFinishHandler
    {
    public:

        MOCK_METHOD(void, TaskFinished, (ITask& Task), (override));
    };
}

#endif
