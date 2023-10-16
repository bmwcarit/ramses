//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/RendererLib/DisplayThread.h"
#include "PlatformMock.h"

namespace ramses::internal
{
    class DisplayThreadMock : public IDisplayThread
    {
    public:
        DisplayThreadMock();
        ~DisplayThreadMock() override;

        MOCK_METHOD(void, startUpdating, (), (override));
        MOCK_METHOD(void, stopUpdating, (), (override));
        MOCK_METHOD(void, setLoopMode, (ELoopMode), (override));
        MOCK_METHOD(void, setMinFrameDuration, (std::chrono::microseconds), (override));
        MOCK_METHOD(uint32_t, getFrameCounter, (), (const, override));
    };
}

