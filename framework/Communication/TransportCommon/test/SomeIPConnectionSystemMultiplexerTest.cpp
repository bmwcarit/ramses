//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SomeIPConnectionSystemMultiplexer.h"
#include "Collections/StringOutputStream.h"
#include "TransportCommon/DcsmConnectionSystem.h"
#include "TransportCommon/RamsesConnectionSystem.h"
#include "Utils/StatisticCollection.h"
#include "SomeIPStackMocks.h"
#include "ServiceHandlerMocks.h"
#include "SceneUpdateSerializerTestHelper.h"
#include "ScopedLogContextLevel.h"

namespace ramses_internal
{
    using namespace testing;

    namespace
    {
        class Callbacks
        {
        public:
            MOCK_METHOD(bool, doPrepareConnect, (), ());
            MOCK_METHOD(void, doFinalizeConnect, (), ());
            MOCK_METHOD(void, doFinalizeDisconnect, (), ());
        };

        class TestSomeIPConnectionSystemMultiplexer : public SomeIPConnectionSystemMultiplexer
        {
        public:
            TestSomeIPConnectionSystemMultiplexer(uint32_t someipCommunicationUserID,
                                                  const ParticipantIdentifier& participantIdentifier,
                                                  PlatformLock& frameworkLock,
                                                  std::unique_ptr<DcsmConnectionSystem> dcsmConnectionSystem,
                                                  std::unique_ptr<RamsesConnectionSystem> ramsesConnectionSystem,
                                                  Callbacks& callbacks_)
                : SomeIPConnectionSystemMultiplexer(someipCommunicationUserID, participantIdentifier, frameworkLock,
                                                    std::move(dcsmConnectionSystem), std::move(ramsesConnectionSystem))
                , callbacks(callbacks_)
            {
                setDcsmProviderServiceHandler(&provider);
                setDcsmConsumerServiceHandler(&consumer);
                setSceneProviderServiceHandler(&sceneProvider);
                setSceneRendererServiceHandler(&sceneRenderer);
            }

            virtual bool doPrepareConnect() override
            {
                return callbacks.doPrepareConnect();
            }

            virtual void doFinalizeConnect() override
            {
                callbacks.doFinalizeConnect();
            }

            virtual void doFinalizeDisconnect() override
            {
                callbacks.doFinalizeDisconnect();
            }

            ISomeIPDcsmStackCallbacks& dcsmStackCallbacks()
            {
                assert(m_dcsmConnectionSystem);
                return *m_dcsmConnectionSystem;
            }

            DcsmConnectionSystem& getDcsm()
            {
                assert(m_dcsmConnectionSystem);
                return *m_dcsmConnectionSystem;
            }

            ISomeIPRamsesStackCallbacks& ramsesStackCallbacks()
            {
                assert(m_ramsesConnectionSystem);
                return *m_ramsesConnectionSystem;
            }

            RamsesConnectionSystem& getRamses()
            {
                assert(m_ramsesConnectionSystem);
                return *m_ramsesConnectionSystem;
            }

            Callbacks& callbacks;
            StrictMock<SceneProviderServiceHandlerMock> sceneProvider;
            StrictMock<SceneRendererServiceHandlerMock> sceneRenderer;
            StrictMock<DcsmProviderServiceHandlerMock> provider;
            StrictMock<DcsmConsumerServiceHandlerMock> consumer;
        };
    }

    class ASomeIPConnectionSystemMultiplexer : public Test
    {
    public:
        auto Make(std::unique_ptr<DcsmConnectionSystem> dcsm, std::unique_ptr<RamsesConnectionSystem> ramses)
        {
            return std::make_unique<TestSomeIPConnectionSystemMultiplexer>(commUser, namedPid, lock, std::move(dcsm), std::move(ramses), callbacks);
        }

        auto MakeDcsm()
        {
            EXPECT_CALL(*dcsmStack, getServiceInstanceId()).WillRepeatedly(Return(DcsmInstanceId(2)));
            return DcsmConnectionSystem::Construct(dcsmStack, commUser, namedPid, 99, lock, stats,
                                                   std::chrono::milliseconds(0), std::chrono::milliseconds(0),
                                                   clock);
        }

        auto MakeRamses()
        {
            EXPECT_CALL(*ramsesStack, getServiceInstanceId()).WillRepeatedly(Return(RamsesInstanceId(2)));
            EXPECT_CALL(*ramsesStack, getSendDataSizes()).WillRepeatedly(Return(RamsesStackSendDataSizes{1000, 1000, 10, 10}));
            return RamsesConnectionSystem::Construct(ramsesStack, commUser, namedPid, 99, lock, stats,
                                                     std::chrono::milliseconds(0), std::chrono::milliseconds(0),
                                                     clock);
        }

        uint32_t commUser = 2;
        ParticipantIdentifier namedPid{Guid(2), "Foo"};
        PlatformLock lock;
        StrictMock<Callbacks> callbacks;
        StatisticCollectionFramework stats;
        std::function<std::chrono::steady_clock::time_point(void)> clock {[](){ return std::chrono::steady_clock::time_point{}; }};
        std::shared_ptr<StrictMock<SomeIPDcsmStackMock>> dcsmStack{std::make_shared<StrictMock<SomeIPDcsmStackMock>>()};
        std::shared_ptr<StrictMock<SomeIPRamsesStackMock>> ramsesStack{std::make_shared<StrictMock<SomeIPRamsesStackMock>>()};

        DcsmMetadata metadata;
        ResourceContentHashVector resourceHashes{{1, 2}, {999, 0xabcdef0123}, {0, 0}, {5, 6}};
        SceneInfoVector sceneInfos{SceneInfo{SceneId(99), "asd"}, SceneInfo{SceneId(1), ""}, SceneInfo{SceneId(0), "boo"}};
        std::vector<SceneAvailabilityUpdate> sceneUpdateAvailable{{SceneId(99), "asd", true}, {SceneId(1), "", true}, {SceneId(0), "boo", true}};
        std::vector<SceneAvailabilityUpdate> sceneUpdateUnavailable{{SceneId(99), "asd", false}, {SceneId(1), "", false}, {SceneId(0), "boo", false}};
        std::vector<Byte> dataBlob{200, 127, 255, 0, 1, 80};
    };

    TEST_F(ASomeIPConnectionSystemMultiplexer, connectFailsWithoutConnectionSystem)
    {
        auto csm = Make(nullptr, nullptr);
        EXPECT_FALSE(csm->connectServices());
        EXPECT_FALSE(csm->disconnectServices());
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, canConnectWithDcsmOnly)
    {
        auto csm = Make(MakeDcsm(), nullptr);
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*dcsmStack, connect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeConnect());
        EXPECT_TRUE(csm->connectServices());
        EXPECT_FALSE(csm->connectServices());

        EXPECT_EQ(&csm->getDcsm().getConnectionStatusUpdateNotifier(), &csm->getDcsmConnectionStatusUpdateNotifier());
        csm->getRamsesConnectionStatusUpdateNotifier();  // can get fake notifier;

        EXPECT_CALL(*dcsmStack, disconnect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeDisconnect());
        EXPECT_TRUE(csm->disconnectServices());
        EXPECT_FALSE(csm->disconnectServices());
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, connectFailsWhenDcsmStackConnectFails)
    {
        auto csm = Make(MakeDcsm(), nullptr);
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*dcsmStack, connect()).WillOnce(Return(false));
        EXPECT_FALSE(csm->connectServices());
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, dcsmStackDisconnectFailMakesDisconnectFinishAndFail)
    {
        auto csm = Make(MakeDcsm(), nullptr);
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*dcsmStack, connect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeConnect());
        EXPECT_TRUE(csm->connectServices());

        EXPECT_CALL(*dcsmStack, disconnect()).WillOnce(Return(false));
        EXPECT_CALL(callbacks, doFinalizeDisconnect());
        EXPECT_FALSE(csm->disconnectServices());

        // connection system tries to disconnect again in dtor
        EXPECT_CALL(*dcsmStack, disconnect()).WillOnce(Return(false));
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, canConnectWithRamsesOnly)
    {
        auto csm = Make(nullptr, MakeRamses());
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*ramsesStack, connect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeConnect());
        EXPECT_TRUE(csm->connectServices());

        EXPECT_EQ(&csm->getRamses().getConnectionStatusUpdateNotifier(), &csm->getRamsesConnectionStatusUpdateNotifier());
        csm->getDcsmConnectionStatusUpdateNotifier();  // can get fake notifier;

        EXPECT_CALL(*ramsesStack, disconnect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeDisconnect());
        EXPECT_TRUE(csm->disconnectServices());
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, connectFailsWhenRamsesStackConnectFails)
    {
        auto csm = Make(nullptr, MakeRamses());
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*ramsesStack, connect()).WillOnce(Return(false));
        EXPECT_FALSE(csm->connectServices());
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, ramsesStackDisconnectFailMakesDisconnectFinishAndFail)
    {
        auto csm = Make(nullptr, MakeRamses());
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*ramsesStack, connect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeConnect());
        EXPECT_TRUE(csm->connectServices());

        EXPECT_CALL(*ramsesStack, disconnect()).WillOnce(Return(false));
        EXPECT_CALL(callbacks, doFinalizeDisconnect());
        EXPECT_FALSE(csm->disconnectServices());

        // connection system tries to disconnect again in dtor
        EXPECT_CALL(*ramsesStack, disconnect()).WillOnce(Return(false));
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, forwardsDcsmCorrectly)
    {
        // connect with dcsm
        auto csm = Make(MakeDcsm(), nullptr);
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*dcsmStack, connect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeConnect());
        EXPECT_TRUE(csm->connectServices());

        EXPECT_CALL(*dcsmStack, sendParticipantInfo(_, _, _, _, _, _, _)).WillOnce(InvokeWithoutArgs([&](){
            csm->dcsmStackCallbacks().handleParticipantInfo(SomeIPMsgHeader{1, 1, 1}, 99, DcsmInstanceId(1), 2, 0, 0);
            return true;
        }));
        csm->dcsmStackCallbacks().handleServiceAvailable(DcsmInstanceId(1));


        // now connected, dcsm send calls must succeed
        EXPECT_CALL(*dcsmStack, sendOfferContent(DcsmInstanceId(1), _, ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "mycontent", 0)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmOfferContent(Guid(1), ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "mycontent"));

        EXPECT_CALL(*dcsmStack, sendCanvasSizeChange(DcsmInstanceId(1), _, ContentID(33), CategoryInfo{10, 11}, 0, AnimationInformation{100, 200})).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmCanvasSizeChange(Guid(1), ContentID(33), CategoryInfo{10, 11}, AnimationInformation{100, 200}));

        EXPECT_CALL(*dcsmStack, sendContentReady(DcsmInstanceId(1), _, ContentID(2))).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmContentReady(Guid(1), ContentID(2)));

        EXPECT_CALL(*dcsmStack, sendContentDescription(DcsmInstanceId(1), _, ContentID(2), TechnicalContentDescriptor(10))).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmContentDescription(Guid(1), ContentID(2), TechnicalContentDescriptor(10)));

        EXPECT_CALL(*dcsmStack, sendContentStateChange(DcsmInstanceId(1), _, ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211})).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmContentStateChange(Guid(1), ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211}));

        EXPECT_CALL(*dcsmStack, sendContentEnableFocusRequest(DcsmInstanceId(1), _, ContentID(42434), 32)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmContentEnableFocusRequest(Guid(1), ContentID(42434), 32));

        EXPECT_CALL(*dcsmStack, sendContentDisableFocusRequest(DcsmInstanceId(1), _, ContentID(42434), 32)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmContentDisableFocusRequest(Guid(1), ContentID(42434), 32));

        EXPECT_CALL(*dcsmStack, sendOfferContent(DcsmInstanceId(1), _, ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "mycontent", 0)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmBroadcastOfferContent(ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "mycontent"));

        EXPECT_CALL(*dcsmStack, sendRequestStopOfferContent(DcsmInstanceId(1), _, ContentID(9), false)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmBroadcastRequestStopOfferContent(ContentID(9)));

        EXPECT_CALL(*dcsmStack, sendRequestStopOfferContent(DcsmInstanceId(1), _, ContentID(9), true)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmBroadcastForceStopOfferContent(ContentID(9)));

        EXPECT_CALL(*dcsmStack, sendUpdateContentMetadata(DcsmInstanceId(1), _, ContentID(56), metadata.toBinary())).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendDcsmUpdateContentMetadata(Guid(1), ContentID(56), metadata));


        // dcsm messages from stack must be passed to handler
        EXPECT_CALL(csm->consumer, handleForceStopOfferContent(ContentID(123), Guid(1)));
        csm->dcsmStackCallbacks().handleRequestStopOfferContent(SomeIPMsgHeader{1, 1, 2}, ContentID(123), true);
        EXPECT_CALL(csm->provider, handleCanvasSizeChange(ContentID(33), CategoryInfo{10, 11}, AnimationInformation{100, 200}, Guid(1)));
        csm->dcsmStackCallbacks().handleCanvasSizeChange(SomeIPMsgHeader{1, 1, 3}, ContentID(33), CategoryInfo{10, 11}, 0, AnimationInformation{100, 200});


        // ramses calls must fail
        EXPECT_FALSE(csm->sendScenesAvailable(Guid(1), sceneInfos));
        EXPECT_FALSE(csm->sendSubscribeScene(Guid(1), SceneId(321)));
        EXPECT_FALSE(csm->sendUnsubscribeScene(Guid(1), SceneId(678)));
        EXPECT_FALSE(csm->sendInitializeScene(Guid(1), SceneId(65)));
        EXPECT_FALSE(csm->sendRendererEvent(Guid(1), SceneId(2), dataBlob));
        EXPECT_FALSE(csm->broadcastNewScenesAvailable(sceneInfos));
        EXPECT_FALSE(csm->broadcastScenesBecameUnavailable(sceneInfos));
        EXPECT_FALSE(csm->sendSceneUpdate(Guid(1), SceneId(434), FakseSceneUpdateSerializer({dataBlob}, 1000)));


        // disconnect
        EXPECT_CALL(*dcsmStack, disconnect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeDisconnect());
        EXPECT_TRUE(csm->disconnectServices());
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, forwardsRamsesCorrectly)
    {
        // connect with ramses
        auto csm = Make(nullptr, MakeRamses());
        InSequence seq;
        EXPECT_CALL(callbacks, doPrepareConnect()).WillOnce(Return(true));
        EXPECT_CALL(*ramsesStack, connect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeConnect());
        EXPECT_TRUE(csm->connectServices());

        EXPECT_CALL(*ramsesStack, sendParticipantInfo(_, _, _, _, _, _, _)).WillOnce(InvokeWithoutArgs([&](){
            csm->ramsesStackCallbacks().handleParticipantInfo(SomeIPMsgHeader{1, 1, 1}, 99, RamsesInstanceId(1), 2, 0, 0);
            return true;
        }));
        csm->ramsesStackCallbacks().handleServiceAvailable(RamsesInstanceId(1));


        // now connected, ramses send calls must succeed
        EXPECT_CALL(*ramsesStack, sendSceneAvailabilityChange(RamsesInstanceId(1), _, sceneUpdateAvailable)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendScenesAvailable(Guid(1), sceneInfos));

        EXPECT_CALL(*ramsesStack, sendSceneSubscriptionChange(RamsesInstanceId(1), _, std::vector<SceneSubscriptionUpdate>({{SceneId(321), true}}))).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendSubscribeScene(Guid(1), SceneId(321)));

        EXPECT_CALL(*ramsesStack, sendSceneSubscriptionChange(RamsesInstanceId(1), _, std::vector<SceneSubscriptionUpdate>({{SceneId(678), false}}))).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendUnsubscribeScene(Guid(1), SceneId(678)));

        EXPECT_CALL(*ramsesStack, sendInitializeScene(RamsesInstanceId(1), _, SceneId(65))).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendInitializeScene(Guid(1), SceneId(65)));

        EXPECT_CALL(*ramsesStack, sendRendererEvent(RamsesInstanceId(1), _, SceneId(2), dataBlob)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendRendererEvent(Guid(1), SceneId(2), dataBlob));

        EXPECT_CALL(*ramsesStack, sendSceneAvailabilityChange(RamsesInstanceId(1), _, sceneUpdateAvailable)).WillOnce(Return(true));
        EXPECT_TRUE(csm->broadcastNewScenesAvailable(sceneInfos));

        EXPECT_CALL(*ramsesStack, sendSceneAvailabilityChange(RamsesInstanceId(1), _, sceneUpdateUnavailable)).WillOnce(Return(true));
        EXPECT_TRUE(csm->broadcastScenesBecameUnavailable(sceneInfos));

        EXPECT_CALL(*ramsesStack, sendSceneUpdate(RamsesInstanceId(1), _, SceneId(999), dataBlob)).WillOnce(Return(true));
        EXPECT_TRUE(csm->sendSceneUpdate(Guid(1), SceneId(999), FakseSceneUpdateSerializer({dataBlob}, 1000)));


        // ramses messages from stack must be passed to handler
        EXPECT_CALL(csm->sceneProvider, handleRendererEvent(SceneId(69), dataBlob, Guid(1)));
        csm->ramsesStackCallbacks().handleRendererEvent(SomeIPMsgHeader{1, 1, 2}, SceneId(69), dataBlob);
        EXPECT_CALL(csm->sceneRenderer, handleInitializeScene(SceneId(9999954), Guid(1)));
        csm->ramsesStackCallbacks().handleInitializeScene(SomeIPMsgHeader{1, 1, 3}, SceneId(9999954));


        // dcsm calls must fail
        EXPECT_FALSE(csm->sendDcsmOfferContent(Guid(1), ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "mycontent"));
        EXPECT_FALSE(csm->sendDcsmCanvasSizeChange(Guid(1), ContentID(33), CategoryInfo{10, 11}, AnimationInformation{100, 200}));
        EXPECT_FALSE(csm->sendDcsmContentReady(Guid(1), ContentID(2)));
        EXPECT_FALSE(csm->sendDcsmContentDescription(Guid(1), ContentID(2), TechnicalContentDescriptor(10)));
        EXPECT_FALSE(csm->sendDcsmContentStateChange(Guid(1), ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211}));
        EXPECT_FALSE(csm->sendDcsmContentEnableFocusRequest(Guid(1), ContentID(42434), 32));
        EXPECT_FALSE(csm->sendDcsmContentDisableFocusRequest(Guid(1), ContentID(42434), 32));
        EXPECT_FALSE(csm->sendDcsmBroadcastOfferContent(ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "mycontent"));
        EXPECT_FALSE(csm->sendDcsmBroadcastRequestStopOfferContent(ContentID(9)));
        EXPECT_FALSE(csm->sendDcsmBroadcastForceStopOfferContent(ContentID(9)));
        EXPECT_FALSE(csm->sendDcsmUpdateContentMetadata(Guid(1), ContentID(56), metadata));


        // disconnect
        EXPECT_CALL(*ramsesStack, disconnect()).WillOnce(Return(true));
        EXPECT_CALL(callbacks, doFinalizeDisconnect());
        EXPECT_TRUE(csm->disconnectServices());
    }

    TEST_F(ASomeIPConnectionSystemMultiplexer, canTriggerLoggingFunctions)
    {
        auto csm = Make(MakeDcsm(), MakeRamses());

        // ensure logging code is triggered
        ScopedLogContextLevel scopedLogLevelComm(CONTEXT_COMMUNICATION, ELogLevel::Info);
        ScopedLogContextLevel scopedLogLevelDcsm(CONTEXT_DCSM, ELogLevel::Info);

        EXPECT_CALL(*dcsmStack, logConnectionState(_));
        EXPECT_CALL(*ramsesStack, logConnectionState(_));
        csm->logConnectionInfo();

        csm->triggerLogMessageForPeriodicLog();

        StringOutputStream sos;
        EXPECT_CALL(*dcsmStack, logConnectionState(_));
        EXPECT_CALL(*ramsesStack, logConnectionState(_));
        csm->writeStateForLog(sos);
        EXPECT_FALSE(sos.release().empty());
    }
}
