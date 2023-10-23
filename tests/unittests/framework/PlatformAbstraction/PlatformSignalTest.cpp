//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/PlatformAbstraction/PlatformSignal.h"
#include "gtest/gtest.h"
#include <atomic>

namespace ramses::internal
{
    namespace
    {
        std::atomic<bool> handlerCalled {false};

        void handler(int32_t /*unused*/)
        {
            handlerCalled = true;
        }
    }

    TEST(PlatformSignal, signalAndRaise)
    {
        PlatformSignal::SetSignalHandler(ESignal::TERM, handler, false);
        ::raise(SIGTERM);
        EXPECT_TRUE(handlerCalled);
    }
}
