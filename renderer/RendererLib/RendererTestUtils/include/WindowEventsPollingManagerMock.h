//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOWEVENTSPOLLINGMANAGERMOCK_H
#define RAMSES_WINDOWEVENTSPOLLINGMANAGERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IWindowEventsPollingManager.h"


namespace ramses_internal
{
    class WindowEventsPollingManagerMock : public IWindowEventsPollingManager
    {
    public:
        MOCK_METHOD(void, pollWindowsTillAnyCanRender, (), (const, override));
    };

    class WindowEventsPollingManagerMockWithDestructor : public WindowEventsPollingManagerMock
    {
    public:
        ~WindowEventsPollingManagerMockWithDestructor()
        {
            Die();
        }

        MOCK_METHOD(void, Die, ());
    };
}

#endif
