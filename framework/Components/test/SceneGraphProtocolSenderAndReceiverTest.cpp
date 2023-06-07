//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AbstractSenderAndReceiverTest.h"
#include "ServiceHandlerMocks.h"
#include "SceneAPI/SceneTypes.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "SceneUpdateSerializerTestHelper.h"

namespace ramses_internal
{
    using namespace testing;

    class ASceneGraphProtocolSenderAndReceiverTest : public AbstractSenderAndReceiverTest
    {
    public:
        ASceneGraphProtocolSenderAndReceiverTest()
            : AbstractSenderAndReceiverTest(EServiceType::Ramses)
        {
            receiver.setSceneRendererServiceHandler(&consumerHandler);
            receiver.setSceneProviderServiceHandler(&providerHandler);
        }

        StrictMock<SceneRendererServiceHandlerMock> consumerHandler;
        StrictMock<SceneProviderServiceHandlerMock> providerHandler;
    };

    INSTANTIATE_TEST_SUITE_P(TypedCommunicationTest, ASceneGraphProtocolSenderAndReceiverTest,
                            ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, broadcastNewScenesAvailable)
    {
        const SceneId sceneId(55u);
        const std::string name("sceneName");
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, name));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleNewScenesAvailable(newScenes, senderId, ramses::EFeatureLevel_Latest)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.broadcastNewScenesAvailable(newScenes, ramses::EFeatureLevel_Latest));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendInitializeScene)
    {
        const std::string name("test");
        const SceneId sceneId(1ull << 63);

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleInitializeScene(sceneId, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendInitializeScene(receiverId, sceneId));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, DISABLED_broadcastScenesBecameUnavailable)
    {
        const SceneId sceneId(1ull << 63);
        SceneInfoVector unavailableScenes;
        unavailableScenes.push_back(SceneInfo(sceneId));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleScenesBecameUnavailable(unavailableScenes, senderId)).WillRepeatedly(InvokeWithoutArgs([&]{ sendEvent(); }));
        }

        AssertionResult result{AssertionFailure()};
        for (int i = 0; i < 15; ++i)
        {
            EXPECT_TRUE(sender.broadcastScenesBecameUnavailable(unavailableScenes));
            result = waitForEvent(1, 200);
            if (result)
                break;
        }
        ASSERT_TRUE(result);
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendScenesAvailable)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(SceneId(55u), "sceneName"));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleNewScenesAvailable(newScenes, senderId, ramses::EFeatureLevel_Latest)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendScenesAvailable(receiverId, newScenes, ramses::EFeatureLevel_Latest));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendSubscribeScene)
    {
        SceneId sceneId;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleSubscribeScene(sceneId, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendSubscribeScene(receiverId, sceneId));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendUnsubscribeScene)
    {
        SceneId sceneId;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleUnsubscribeScene(sceneId, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendUnsubscribeScene(receiverId, sceneId));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendRendererEvent)
    {
        SceneId sceneId{432};
        std::vector<Byte> data{7, 1, 2, 3, 4, 5, 6};
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleRendererEvent(sceneId, data, senderId)).WillOnce(InvokeWithoutArgs([&] { sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendRendererEvent(receiverId, sceneId, data));
        ASSERT_TRUE(waitForEvent());

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleRendererEvent(sceneId, data, senderId)).WillOnce(InvokeWithoutArgs([&] { sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendRendererEvent(receiverId, sceneId, data));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendRendererEventFailsForTooLargeData)
    {
        std::vector<Byte> data(35000);
        EXPECT_FALSE(sender.sendRendererEvent(receiverId, SceneId(111), data));
        skipStatisticsTest();
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendSceneActions)
    {
        skipStatisticsTest();

        const SceneId sceneId{432};
        const std::vector<Byte> blob_1({1, 2, 3, 4, 5, 6});
        const std::vector<Byte> blob_2({255, 254, 11, 3});

        {
            PlatformGuard g(receiverExpectCallLock);
            InSequence    seq;
            EXPECT_CALL(consumerHandler, handleSceneUpdate(sceneId, _, senderId)).WillOnce([&](auto, const auto& data, auto) {
                EXPECT_EQ(blob_1, data);
                sendEvent();
            });
            EXPECT_CALL(consumerHandler, handleSceneUpdate(sceneId, _, senderId)).WillOnce([&](auto, const auto& data, auto) {
                EXPECT_EQ(blob_2, data);
                sendEvent();
            });
        }

        FakseSceneUpdateSerializer serializer({blob_1, blob_2}, 300000);
        EXPECT_TRUE(sender.sendSceneUpdate(receiverId, sceneId, serializer));
        ASSERT_TRUE(waitForEvent(2));
    }

}
