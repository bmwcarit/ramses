//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Watchdog/IThreadAliveNotifier.h"

namespace ramses::internal
{
    class ThreadAliveNotifierMock : public IThreadAliveNotifier
    {
    public:
        constexpr static uint64_t dummyThreadId{ 68 };

        ThreadAliveNotifierMock()
        {
            ON_CALL(*this, registerThread()).WillByDefault(testing::Return(dummyThreadId));
            ON_CALL(*this, calculateTimeout()).WillByDefault(testing::Return(std::chrono::milliseconds{ 10 }));
        }

        MOCK_METHOD(uint64_t, registerThread, (), (override));
        MOCK_METHOD(void, unregisterThread, (uint64_t identifier), (override));
        MOCK_METHOD(void, notifyAlive, (uint64_t identifier), (override));
        MOCK_METHOD(std::chrono::milliseconds, calculateTimeout, (), (const, override));
    };
}
