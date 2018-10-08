//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "ComponentMocks.h"
#include "Components/SceneGraphComponent.h"
#include "ProviderAndConsumerBaseTest.h"
#include "Collections/Vector.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "gmock/gmock.h"
#include "ServiceHandlerMocks.h"

namespace ramses_internal
{
    using namespace testing;

    class ASceneGraphProviderAndConsumerTest : public ProviderAndConsumerBaseTest
    {
    public:
        ASceneGraphProviderAndConsumerTest()
            : scenegraphConsumer(consumerGuid, consumerCommunicationSystem, consumerConnectionStatusUpdateNotifier, frameworkLock)
            , scenegraphProvider(providerGuid, providerCommunicationSystem, providerConnectionStatusUpdateNotifier, frameworkLock)
        {
            scenegraphConsumer.setSceneRendererServiceHandler(&consumer);
        }

    protected:
        SceneRendererServiceHandlerMock consumer;
        SceneGraphComponent scenegraphConsumer;
        SceneGraphComponent scenegraphProvider;
    };

    TEST_F(ASceneGraphProviderAndConsumerTest, sceneactionlistCounterStartsWith1)
    {
        SceneId sceneid(1234u);
        SceneInfo info(sceneid);
        scenegraphProvider.sendCreateScene(consumerGuid, info, EScenePublicationMode_LocalAndRemote);
        EXPECT_CALL(consumer, handleSceneActionList_rvr(_, _, 1u, _));
        scenegraphProvider.sendSceneActionList({ consumerGuid }, SceneActionCollection(), sceneid, EScenePublicationMode_LocalAndRemote);
    }

}
