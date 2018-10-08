//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PROVIDERANDCONSUMERBASETEST_H
#define RAMSES_PROVIDERANDCONSUMERBASETEST_H

#include "MockConnectionStatusUpdateNotifier.h"
#include "MockTaskQueue.h"
#include "ForwardingCommunicationSystem.h"

namespace ramses_internal
{
    using namespace testing;

    class ProviderAndConsumerBaseTest : public testing::Test
    {
    public:
        ProviderAndConsumerBaseTest();

    protected:
        PlatformLock frameworkLock;
        NiceMock<MockConnectionStatusUpdateNotifier> consumerConnectionStatusUpdateNotifier;
        NiceMock<MockConnectionStatusUpdateNotifier> providerConnectionStatusUpdateNotifier;

        MockTaskQueue taskQueue;

        //consumer
        Guid consumerGuid;
        ForwardingCommunicationSystem consumerCommunicationSystem;

        //provider
        Guid providerGuid;
        ForwardingCommunicationSystem providerCommunicationSystem;

    };

}

#endif
