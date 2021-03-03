//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYTHREADMOCK_H
#define RAMSES_DISPLAYTHREADMOCK_H

#include "gmock/gmock.h"
#include "RendererLib/DisplayThread.h"
#include "PlatformMock.h"

namespace ramses_internal
{
    class DisplayThreadMock : public IDisplayThread
    {
    public:
        DisplayThreadMock();
        virtual ~DisplayThreadMock() override;

        MOCK_METHOD(void, startUpdating, (), (override));
        MOCK_METHOD(void, stopUpdating, (), (override));
        MOCK_METHOD(void, setLoopMode, (ELoopMode), (override));
        MOCK_METHOD(void, setMinFrameDuration, (std::chrono::microseconds), (override));
    };
}
#endif
