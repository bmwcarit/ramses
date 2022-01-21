//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/DcsmConnectionSystem.h"
#include "SomeIPStackMocks.h"
#include "MockConnectionStatusListener.h"
#include "ServiceHandlerMocks.h"
#include "gmock/gmock.h"
#include "DcsmGmockPrinter.h"
#include "Utils/LogMacros.h"
#include "ConnectionSystemTestCommon.h"
#include "ConcreteConnectionSystemCommonTest.h"

namespace ramses_internal
{
    class ADcsmConnectionSystem : public TestWithParam<ConcreteConnectionSystemCommonTest::Config>,
                                  public ConcreteConnectionSystemCommonTest
    {
    public:
        ADcsmConnectionSystem()
            : stackPtr(std::make_shared<StrictMock<SomeIPDcsmStackMock>>())
            , stack(*stackPtr)
            , connsys(construct(stackPtr))
            , fromStack(*connsys)
        {
            metadata.setCarModel(123);
            metadata.setPreviewDescription(U"foobar!");
        }

        std::unique_ptr<DcsmConnectionSystem> construct(const std::shared_ptr<StrictMock<SomeIPDcsmStackMock>>& stackMock)
        {
            EXPECT_CALL(stack, getServiceInstanceId()).WillRepeatedly(Return(DcsmInstanceId(GetParam().serviceIid)));
            auto ptr = DcsmConnectionSystem::Construct(stackMock, 3, ParticipantIdentifier(Guid(pid), "foobar"), 99, lock,
                                                       std::chrono::milliseconds(0), std::chrono::milliseconds(0),
                                                       CountingConnectionSystemClock());
            assert(ptr);

            ptr->setDcsmProviderServiceHandler(&provider);
            ptr->setDcsmConsumerServiceHandler(&consumer);
            ptr->getConnectionStatusUpdateNotifier().registerForConnectionUpdates(&connections);

            return ptr;
        }

        uint64_t connectRemote(DcsmInstanceId remoteIidArg, const Guid& remotePid)
        {
            return connectRemoteHelper(GetParam(), stack, fromStack, remoteIidArg, remotePid);
        }

        std::shared_ptr<StrictMock<SomeIPDcsmStackMock>> stackPtr;
        StrictMock<SomeIPDcsmStackMock>& stack;
        StrictMock<DcsmProviderServiceHandlerMock> provider;
        StrictMock<DcsmConsumerServiceHandlerMock> consumer;
        PlatformLock lock;
        std::unique_ptr<DcsmConnectionSystem> connsys;
        ISomeIPDcsmStackCallbacks& fromStack;
        DcsmInstanceId remoteIid{GetParam().remoteIid};
        DcsmMetadata metadata;
    };

    INSTANTIATE_TEST_SUITE_P(ADcsmConnectionSystemP,
                             ADcsmConnectionSystem,
                             testing::ValuesIn(ConcreteConnectionSystemCommonTest::ConfigValues()),
                             ConcreteConnectionSystemCommonTest::ConfigPrinter());

    class ADcsmConnectionSystemConnected : public ADcsmConnectionSystem
    {
    public:
        void SetUp() override
        {
            lock.lock();
            EXPECT_CALL(stack, connect()).WillOnce(Return(true));
            ASSERT_TRUE(connsys->connect());
        }

        void TearDown() override
        {
            EXPECT_CALL(stack, disconnect()).WillOnce(Return(true));
            ASSERT_TRUE(connsys->disconnect());
            lock.unlock();
        }
    };

    INSTANTIATE_TEST_SUITE_P(ADcsmConnectionSystemConnectedP,
                             ADcsmConnectionSystemConnected,
                             testing::ValuesIn(ConcreteConnectionSystemCommonTest::ConfigValues()),
                             ConcreteConnectionSystemCommonTest::ConfigPrinter());

    TEST_P(ADcsmConnectionSystem, sendMethodsFailWhenNotConnected)
    {
        std::lock_guard<std::recursive_mutex> g(lock);
        EXPECT_FALSE(connsys->sendOfferContent(Guid(10), ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "fooname", 2));
        EXPECT_FALSE(connsys->sendCanvasSizeChange(Guid(10), ContentID(33), CategoryInfo{10, 11}, 300, AnimationInformation{100, 200}));
        EXPECT_FALSE(connsys->sendContentReady(Guid(10), ContentID(2)));
        EXPECT_FALSE(connsys->sendContentDescription(Guid(10), ContentID(2), TechnicalContentDescriptor(10)));
        EXPECT_FALSE(connsys->sendContentStateChange(Guid(10), ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211}));
        EXPECT_FALSE(connsys->sendContentEnableFocusRequest(Guid(10), ContentID(42434), 32));
        EXPECT_FALSE(connsys->sendContentDisableFocusRequest(Guid(10), ContentID(42434), 32));
        EXPECT_FALSE(connsys->sendBroadcastOfferContent(ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "bar", 3));
        EXPECT_FALSE(connsys->sendBroadcastRequestStopOfferContent(ContentID(9), true));
        EXPECT_FALSE(connsys->sendBroadcastUpdateContentMetadata(ContentID(55), metadata));
        EXPECT_FALSE(connsys->sendUpdateContentMetadata(Guid(10), ContentID(56), metadata));
        EXPECT_FALSE(connsys->sendContentStatus(Guid(10), ContentID(88), 23u, std::vector<Byte>(2)));
    }

    TEST_P(ADcsmConnectionSystemConnected, canSendUnicastMessagesToConnectedParticipant)
    {
        connectRemote(DcsmInstanceId(1), Guid(2));
        connectRemote(remoteIid, Guid(10));

        SomeIPMsgHeader firstHdr;
        EXPECT_CALL(stack, sendOfferContent(remoteIid, ValidHdr(pid, 2u), ContentID(44), Category(55), static_cast<ETechnicalContentType>(9), "fooname", 2)).WillOnce(DoAll(SaveArg<1>(&firstHdr), Return(true)));
        EXPECT_TRUE(connsys->sendOfferContent(Guid(10), ContentID(44), Category(55), static_cast<ETechnicalContentType>(9), "fooname", 2));

        EXPECT_CALL(stack, sendCanvasSizeChange(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 3u}, ContentID(33), CategoryInfo{10, 11}, 300, AnimationInformation{100, 200})).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendCanvasSizeChange(Guid(10), ContentID(33), CategoryInfo{10, 11}, 300, AnimationInformation{100, 200}));

        EXPECT_CALL(stack, sendContentReady(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 4u}, ContentID(2))).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendContentReady(Guid(10), ContentID(2)));

        EXPECT_CALL(stack, sendContentStateChange(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 5u}, ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211})).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendContentStateChange(Guid(10), ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211}));

        EXPECT_CALL(stack, sendContentEnableFocusRequest(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 6u}, ContentID(42434), 32)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendContentEnableFocusRequest(Guid(10), ContentID(42434), 32));

        EXPECT_CALL(stack, sendContentDisableFocusRequest(remoteIid, SomeIPMsgHeader{ pid, firstHdr.sessionId, 7u }, ContentID(42434), 32)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendContentDisableFocusRequest(Guid(10), ContentID(42434), 32));

        EXPECT_CALL(stack, sendUpdateContentMetadata(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 8u}, ContentID(65464), metadata.toBinary())).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendUpdateContentMetadata(Guid(10), ContentID(65464), metadata));

        EXPECT_CALL(stack, sendContentDescription(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 9u}, ContentID(88), TechnicalContentDescriptor(21))).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendContentDescription(Guid(10), ContentID(88), TechnicalContentDescriptor(21)));

        std::vector<Byte> message{ 1, 2 ,3, 4 };
        EXPECT_CALL(stack, sendContentStatus(remoteIid, SomeIPMsgHeader{ pid, firstHdr.sessionId, 10u }, ContentID(88), 23u, message)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendContentStatus(Guid(10), ContentID(88), 23u, message));

        expectRemoteDisconnects(stack, {2, 10});
    }

    TEST_P(ADcsmConnectionSystemConnected, canSendBroadcastMessagesToConnectedParticipants)
    {
        connectRemote(DcsmInstanceId(1), Guid(2));
        connectRemote(remoteIid, Guid(10));

        SomeIPMsgHeader firstHdrIid_1;
        SomeIPMsgHeader firstHdrIid_3;
        EXPECT_CALL(stack, sendOfferContent(DcsmInstanceId(1), ValidHdr(pid, 2u), ContentID(44), Category(55), static_cast<ETechnicalContentType>(9), "bar", 3)).WillOnce(DoAll(SaveArg<1>(&firstHdrIid_1), Return(true)));
        EXPECT_CALL(stack, sendOfferContent(remoteIid, ValidHdr(pid, 2u), ContentID(44), Category(55), static_cast<ETechnicalContentType>(9), "bar", 3)).WillOnce(DoAll(SaveArg<1>(&firstHdrIid_3), Return(true)));
        EXPECT_TRUE(connsys->sendBroadcastOfferContent(ContentID(44), Category(55), static_cast<ETechnicalContentType>(9), "bar", 3));

        EXPECT_CALL(stack, sendRequestStopOfferContent(DcsmInstanceId(1), SomeIPMsgHeader{pid, firstHdrIid_1.sessionId, 3u}, ContentID(9), true)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendRequestStopOfferContent(remoteIid, SomeIPMsgHeader{pid, firstHdrIid_3.sessionId, 3u}, ContentID(9), true)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendBroadcastRequestStopOfferContent(ContentID(9), true));

        EXPECT_CALL(stack, sendUpdateContentMetadata(DcsmInstanceId(1), SomeIPMsgHeader{pid, firstHdrIid_1.sessionId, 4u}, ContentID(23), metadata.toBinary())).WillOnce(Return(true));
        EXPECT_CALL(stack, sendUpdateContentMetadata(remoteIid, SomeIPMsgHeader{pid, firstHdrIid_3.sessionId, 4u}, ContentID(23), metadata.toBinary())).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendBroadcastUpdateContentMetadata(ContentID(23), metadata));

        expectRemoteDisconnects(stack, {2, 10});
    }

    TEST_P(ADcsmConnectionSystemConnected, cannotSendUnicastMessageToUnconnectedParticipant)
    {
        connectRemote(remoteIid, Guid(2));
        expectRemoteDisconnects(stack, {2});
        fromStack.handleServiceUnavailable(remoteIid);
        Mock::VerifyAndClearExpectations(&connections);

        EXPECT_FALSE(connsys->sendOfferContent(Guid(10), ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "fooname", 2));
        EXPECT_FALSE(connsys->sendCanvasSizeChange(Guid(10), ContentID(33), CategoryInfo{10, 11}, 300, AnimationInformation{100, 200}));
        EXPECT_FALSE(connsys->sendContentDescription(Guid(10), ContentID(2), TechnicalContentDescriptor(10)));
        EXPECT_FALSE(connsys->sendContentReady(Guid(10), ContentID(2)));
        EXPECT_FALSE(connsys->sendContentStateChange(Guid(10), ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211}));
        EXPECT_FALSE(connsys->sendContentEnableFocusRequest(Guid(10), ContentID(42434), 32));
        EXPECT_FALSE(connsys->sendContentDisableFocusRequest(Guid(10), ContentID(42434), 32));

        EXPECT_FALSE(connsys->sendOfferContent(Guid(2), ContentID(44), Category(55), static_cast<ETechnicalContentType>(5), "fooname", 2));
        EXPECT_FALSE(connsys->sendCanvasSizeChange(Guid(2), ContentID(33), CategoryInfo{10, 11}, 300, AnimationInformation{100, 200}));
        EXPECT_FALSE(connsys->sendContentDescription(Guid(2), ContentID(2), TechnicalContentDescriptor(10)));
        EXPECT_FALSE(connsys->sendContentReady(Guid(2), ContentID(2)));
        EXPECT_FALSE(connsys->sendContentStateChange(Guid(2), ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211}));
        EXPECT_FALSE(connsys->sendContentEnableFocusRequest(Guid(2), ContentID(42434), 32));
        EXPECT_FALSE(connsys->sendContentDisableFocusRequest(Guid(2), ContentID(42434), 32));
        EXPECT_FALSE(connsys->sendUpdateContentMetadata(Guid(2), ContentID(65464), metadata));
        EXPECT_FALSE(connsys->sendContentStatus(Guid(10), ContentID(42434), 32u, std::vector<Byte>(2)));
    }

    TEST_P(ADcsmConnectionSystemConnected, passesThroughMessagesFromConnectedParticipant)
    {
        const uint64_t session = connectRemote(remoteIid, Guid(10));

        EXPECT_CALL(consumer, handleOfferContent(ContentID(44), Category(55), static_cast<ETechnicalContentType>(9), "fooname", Guid(10)));
        fromStack.handleOfferContent(SomeIPMsgHeader{10, session, 2}, ContentID(44), Category(55), static_cast<ETechnicalContentType>(9), "fooname", 2);

        EXPECT_CALL(consumer, handleRequestStopOfferContent(ContentID(4324), Guid(10)));
        fromStack.handleRequestStopOfferContent(SomeIPMsgHeader{10, session, 3}, ContentID(4324), false);

        EXPECT_CALL(consumer, handleForceStopOfferContent(ContentID(4324), Guid(10)));
        fromStack.handleRequestStopOfferContent(SomeIPMsgHeader{10, session, 4}, ContentID(4324), true);

        EXPECT_CALL(provider, handleCanvasSizeChange(ContentID(33), CategoryInfo{10, 11}, AnimationInformation{100, 200}, Guid(10)));
        fromStack.handleCanvasSizeChange(SomeIPMsgHeader{10, session, 5}, ContentID(33), CategoryInfo{10, 11}, 300, AnimationInformation{100, 200});

        EXPECT_CALL(consumer, handleContentReady(ContentID(2), Guid(10)));
        fromStack.handleContentReady(SomeIPMsgHeader{10, session, 6}, ContentID(2));

        EXPECT_CALL(provider, handleContentStateChange(ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211}, Guid(10)));
        fromStack.handleContentStateChange(SomeIPMsgHeader{10, session, 7}, ContentID(16012), EDcsmState::Assigned, CategoryInfo{0xFFFFF2, 0x7FFFF1}, AnimationInformation{432342, 43211});

        EXPECT_CALL(consumer, handleContentEnableFocusRequest(ContentID(543), 32, Guid(10)));
        fromStack.handleContentEnableFocusRequest(SomeIPMsgHeader{10, session, 8}, ContentID(543), 32);

        EXPECT_CALL(consumer, handleContentDisableFocusRequest(ContentID(543), 32, Guid(10)));
        fromStack.handleContentDisableFocusRequest(SomeIPMsgHeader{ 10, session, 9 }, ContentID(543), 32);

        EXPECT_CALL(consumer, handleUpdateContentMetadata(ContentID(3543), metadata, Guid(10)));
        fromStack.handleUpdateContentMetadata(SomeIPMsgHeader{10, session, 10}, ContentID(3543), metadata.toBinary());

        EXPECT_CALL(consumer, handleContentDescription(ContentID(3543), TechnicalContentDescriptor{ 5 }, Guid(10)));
        fromStack.handleContentDescription(SomeIPMsgHeader{ 10, session, 11 }, ContentID(3543), TechnicalContentDescriptor{ 5 });

        std::vector<Byte> message{ 1, 2 ,3, 4 };
        EXPECT_CALL(provider, handleContentStatus(ContentID(3543), 23u, absl::Span<const Byte>{ message.data(), message.size() }, Guid(10)));
        fromStack.handleContentStatus(SomeIPMsgHeader{ 10, session, 12 }, ContentID(3543), 23u, absl::Span<const Byte>{ message.data(), message.size() });

        // no handler yet
        fromStack.handleResponse(SomeIPMsgHeader{ 10, session, 13 }, 0, 0, 0);

        expectRemoteDisconnects(stack, {10});
    }
}
