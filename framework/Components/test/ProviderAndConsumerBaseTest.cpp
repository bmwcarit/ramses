//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceMock.h"
#include "ComponentMocks.h"
#include "Components/ManagedResource.h"
#include "Resource/ArrayResource.h"
#include "Components/ResourceComponent.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "MockTaskQueue.h"
#include "ForwardingCommunicationSystem.h"
#include "ProviderAndConsumerBaseTest.h"


namespace ramses_internal
{
    using namespace testing;

    ProviderAndConsumerBaseTest::ProviderAndConsumerBaseTest()
            : consumerGuid(78)
            , consumerCommunicationSystem(consumerGuid)
            , providerGuid(79)
            , providerCommunicationSystem(providerGuid)
        {
            consumerCommunicationSystem.setForwardingTarget(&providerCommunicationSystem);
            providerCommunicationSystem.setForwardingTarget(&consumerCommunicationSystem);
        }
}
