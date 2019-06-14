//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/DcsmComponent.h"
#include "TransportCommon/FakeConnectionStatusUpdateNotifier.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "CommunicationSystemMock.h"
#include "DcsmEventHandlerMocks.h"
#include "gmock/gmock.h"
#include "DcsmGmockPrinter.h"

#include <array>
#include "Utils/CommandLineParser.h"
#include "Utils/RamsesLogger.h"

namespace ramses_internal
{
    using namespace ::testing;

    class ADcsmComponent : public ::testing::Test
    {
    public:
        ADcsmComponent()
            : localId(true)
            , remoteId(true)
            , otherRemoteId(true)
            , comp(localId, comm, connNotifier, frameworkLock)
        {
            static bool initializedLogger = false;
            if (!initializedLogger)
            {
                initializedLogger = true;
                std::array<const char* const, 3> args{"", "-l", "info"};
                GetRamsesLogger().initialize(
                    CommandLineParser{static_cast<Int>(args.size()), args.data()}, "TEST", "TEST", true);
            }
        }

        void TearDown() override
        {
            // ensure no events pending
            Mock::VerifyAndClearExpectations(&comm);
            Mock::VerifyAndClearExpectations(&provider);
            Mock::VerifyAndClearExpectations(&consumer);
            comp.dispatchConsumerEvents(consumer);
            comp.dispatchProviderEvents(provider);
        }

        void enableLocalAndRemoteUsers()
        {
            EXPECT_TRUE(comp.setLocalProviderAvailability(true));
            EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
            comp.newParticipantHasConnected(remoteId);
            comp.newParticipantHasConnected(otherRemoteId);
        }

        void doRegisterContent(std::initializer_list<uint32_t> asLocalContentIDs, std::initializer_list<uint32_t> asRemoteContentIDs, bool hasLocalConsumer,
                               std::initializer_list<uint32_t> asOtherRemoteContentIDs = {})
        {
            for (const auto cid : asLocalContentIDs)
            {
                EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(cid), Category(3)));
                EXPECT_TRUE(comp.sendRegisterContent(ContentID(cid), Category(3)));
                if (hasLocalConsumer)
                    EXPECT_CALL(consumer, registerContent(ramses::ContentID(cid), ramses::Category(3)));
            }
            for (const auto cid : asRemoteContentIDs)
            {
                comp.handleRegisterContent(ContentID(cid), Category(3), remoteId);
                if (hasLocalConsumer)
                    EXPECT_CALL(consumer, registerContent(ramses::ContentID(cid), ramses::Category(3)));
            }
            for (const auto cid : asOtherRemoteContentIDs)
            {
                comp.handleRegisterContent(ContentID(cid), Category(3), otherRemoteId);
                if (hasLocalConsumer)
                    EXPECT_CALL(consumer, registerContent(ramses::ContentID(cid), ramses::Category(3)));
            }
            Mock::VerifyAndClearExpectations(&comm);
            if (hasLocalConsumer)
            {
                EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                Mock::VerifyAndClearExpectations(&consumer);
            }
        }

        PlatformLock frameworkLock;
        Guid localId;
        Guid remoteId;
        Guid otherRemoteId;
        FakeConnectionStatusUpdateNotifier connNotifier;
        StrictMock<CommunicationSystemMock> comm;
        StrictMock<DcsmProviderEventHandlerMock> provider;
        StrictMock<DcsmConsumerEventHandlerMock> consumer;
        DcsmComponent comp;
    };

    TEST_F(ADcsmComponent, registeresAndUnregistersForConnectionUpdates)
    {
        StrictMock<MockConnectionStatusUpdateNotifier> notifier;
        EXPECT_CALL(notifier, registerForConnectionUpdates(NotNull()));
        {
            DcsmComponent comp_(localId, comm, notifier, frameworkLock);
            Mock::VerifyAndClearExpectations(&notifier);
            EXPECT_CALL(notifier, unregisterForConnectionUpdates(NotNull()));
        }
    }

    TEST_F(ADcsmComponent, participantConnectDisconnectDoesNoEvent)
    {
        Guid p1(true);
        Guid p2(true);
        comp.newParticipantHasConnected(p1);
        comp.newParticipantHasConnected(p2);
        comp.participantHasDisconnected(p1);
        comp.participantHasDisconnected(p2);
        // expect no events
    }

    TEST_F(ADcsmComponent, unknownParticipantDisconnect)
    {
        comp.participantHasDisconnected(Guid(true));
        comp.participantHasDisconnected(Guid(true));
    }

    TEST_F(ADcsmComponent, lateConsumerGetsRemoteRegister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(1), Category(2), remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        Mock::VerifyAndClearExpectations(&consumer);
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(remoteId);
        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, earlyConsumerGetsRemoteRegister)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(1), Category(2), remoteId);

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(remoteId);
        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, lateProviderDoesNotGetEventFromRemoteRegister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(1), Category(2), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, earlyProviderDoesNotGetEventFromRemoteRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(1), Category(2), remoteId);
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, connectRegisterDisconnectDoesNoEvent)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(1), Category(2), remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
    }

    TEST_F(ADcsmComponent, noConsumerEventWhenRemoteRegisterAfterRemove)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));

        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(1), Category(2), remoteId);
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, noUnregisterAfterConsumerRemoved)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(1), Category(2), remoteId);

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndLateConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(4), Category(3)));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(4), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndEarlyConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(4), Category(3)));

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(4), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterLateNetworkGetsRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmRegisterContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, localRegisterEarlyNetworkGetsbroadcastRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, duplicateConnectSendRegisterAgain)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmRegisterContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmRegisterContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, multipleConnectDisconnectGetsRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmRegisterContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_CALL(comm, sendDcsmRegisterContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, onlyLocalRegistrationsAreSentOut)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);
        comp.handleRegisterContent(ContentID(2), Category(2), otherRemoteId);
        comp.handleRegisterContent(ContentID(4), Category(9), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(11), Category(8)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(11), Category(8)));

        Guid lateRemoteId(true);
        EXPECT_CALL(comm, sendDcsmRegisterContent(lateRemoteId, ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmRegisterContent(lateRemoteId, ContentID(11), Category(8)));
        comp.newParticipantHasConnected(lateRemoteId);

        comp.participantHasDisconnected(remoteId);
        comp.participantHasDisconnected(otherRemoteId);
        comp.participantHasDisconnected(lateRemoteId);
    }

    TEST_F(ADcsmComponent, multipleRegistrationsArriveAtLateConsumer)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);
        comp.handleRegisterContent(ContentID(2), Category(2), otherRemoteId);
        comp.handleRegisterContent(ContentID(4), Category(9), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(11), Category(8)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(11), Category(8)));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(4), ramses::Category(9)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(2), ramses::Category(2)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(10), ramses::Category(9)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(11), ramses::Category(8)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(remoteId);
        comp.participantHasDisconnected(otherRemoteId);
        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(1)));
        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(4)));
        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, multipleRegistrationsArriveAtEarlyConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);
        comp.handleRegisterContent(ContentID(2), Category(2), otherRemoteId);
        comp.handleRegisterContent(ContentID(4), Category(9), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(11), Category(8)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(11), Category(8)));

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(4), ramses::Category(9)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(2), ramses::Category(2)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(10), ramses::Category(9)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(11), ramses::Category(8)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localProviderDoesNotLeaveTracesAfterRemoval)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(10)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        Mock::VerifyAndClearExpectations(&comm);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, localProviderDoesNotLeaveTracesAfterUnregister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(10)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(10)));
        Mock::VerifyAndClearExpectations(&comm);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, remoteProviderDoesNotLeaveTracesAfterUnregister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(10), Category(9), remoteId);
        comp.handleRequestUnregisterContent(ContentID(10), remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, sendRegisterContent_forwardedToLocalAndRemote)
    {
        enableLocalAndRemoteUsers();
        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(1), Category(3)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, sendContentAvailable_forwardedToLocalAndRemote)
    {
        enableLocalAndRemoteUsers();
        doRegisterContent({1, 2}, {}, true);
        EXPECT_CALL(comm, sendDcsmContentAvailable(remoteId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.sendContentAvailable(remoteId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        Mock::VerifyAndClearExpectations(&comm);
        EXPECT_TRUE(comp.sendContentAvailable(localId, ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(6)));
        EXPECT_CALL(consumer, contentAvailable(ramses::ContentID(2), ramses::ETechnicalContentType::RamsesSceneID, ramses::TechnicalContentDescriptor(6)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, sendCategoryContentSwitchRequest_forwardedToLocalAndRemote)
    {
        enableLocalAndRemoteUsers();
        doRegisterContent({8, 9}, {}, true);
        EXPECT_CALL(comm, sendDcsmCategoryContentSwitchRequest(remoteId, ContentID(8)));
        EXPECT_TRUE(comp.sendCategoryContentSwitchRequest(remoteId, ContentID(8)));
        Mock::VerifyAndClearExpectations(&comm);
        EXPECT_TRUE(comp.sendCategoryContentSwitchRequest(localId, ContentID(9)));
        EXPECT_CALL(consumer, categoryContentSwitchRequest(ramses::ContentID(9)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, sendRequestUnregisterContent_forwardedToLocalAndRemote)
    {
        enableLocalAndRemoteUsers();
        doRegisterContent({8}, {}, true);
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(8)));
        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(8)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(8)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, sendCanvasSizeChange_forwardedToLocalAndRemote)
    {
        enableLocalAndRemoteUsers();
        doRegisterContent({12}, {8}, true);
        EXPECT_CALL(comm, sendDcsmCanvasSizeChange(remoteId, ContentID(8), SizeInfo{10, 20}, AnimationInformation{100, 200}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(8), SizeInfo{10, 20}, AnimationInformation{100, 200}));
        Mock::VerifyAndClearExpectations(&comm);
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(12), SizeInfo{11, 10}, AnimationInformation{200, 100}));
        EXPECT_CALL(provider, canvasSizeChange(ramses::ContentID(12), ramses::SizeInfo{11, 10}, ramses::AnimationInformation{200, 100}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, sendContentStatusChange_forwardedToLocalAndRemote)
    {
        enableLocalAndRemoteUsers();
        doRegisterContent({12}, {8}, true);
        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(8), EDcsmStatus::ContentRequested, AnimationInformation{100, 200}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(8), EDcsmStatus::ContentRequested, AnimationInformation{100, 200}));
        Mock::VerifyAndClearExpectations(&comm);
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(12), EDcsmStatus::Unregistered, AnimationInformation{200, 100}));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(12), ramses::EDcsmStatus::Unregistered, ramses::AnimationInformation{200, 100}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, correctlyMapsStatusInContentStatusChange)
    {

        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({8, 9, 10, 11}, {}, true);

        InSequence seq;
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(8), EDcsmStatus::Unregistered, AnimationInformation{100, 200}));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(8), ramses::EDcsmStatus::Unregistered, ramses::AnimationInformation{100, 200}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(9), EDcsmStatus::Registered, AnimationInformation{100, 200}));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(9), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{100, 200}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(10), EDcsmStatus::ContentRequested, AnimationInformation{100, 200}));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(10), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{100, 200}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(11), EDcsmStatus::Shown, AnimationInformation{100, 200}));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(11), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{100, 200}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, handleFunctionsDoNothingWithoutLocalReceiver)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleRegisterContent(ContentID(2), Category(10), remoteId);
        comp.handleContentAvailable(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(4), remoteId);
        comp.handleRequestUnregisterContent(ContentID(5), remoteId);
    }

    TEST_F(ADcsmComponent, handleFunctionsForwardToLocalReceiver)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({6, 1}, {3, 4, 5}, true);

        InSequence seq;
        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleRegisterContent(ContentID(2), Category(10), remoteId);
        comp.handleContentAvailable(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(4), remoteId);
        comp.handleRequestUnregisterContent(ContentID(5), remoteId);

        EXPECT_CALL(provider, canvasSizeChange(ramses::ContentID(6), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{3, 4}, remoteId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{7, 8}, remoteId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(2), ramses::Category(10)));
        EXPECT_CALL(consumer, contentAvailable(ramses::ContentID(3), ramses::ETechnicalContentType::RamsesSceneID, ramses::TechnicalContentDescriptor(11)));
        EXPECT_CALL(consumer, categoryContentSwitchRequest(ramses::ContentID(4)));
        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, informsLocalProviderWhenConsumerGoneForConsumedReadyContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({34}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, informsRemoteProviderWhenConsumerGoneForConsumedReadyContent)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({}, {34}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3, 4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, informsLocalProviderWhenConsumerGoneForConsumedShownContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({34}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Shown, AnimationInformation{3,4}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, informsRemoteProviderWhenConsumerGoneForConsumedShownContent)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({}, {34}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Shown, AnimationInformation{3, 4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Shown, AnimationInformation{3,4}));

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, doesNotInformLocalProviderWhenConsumerGoneForRegisteredContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({34}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Registered, AnimationInformation{3,4}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, doesNotInformRemoteProviderWhenConsumerGoneForRegisteredContent)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({}, {34}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Registered, AnimationInformation{3, 4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Registered, AnimationInformation{3,4}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, informsLocalProviderWhenConsumerGoneForUnregisteredContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({34}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Unregistered, AnimationInformation{3,4}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Unregistered, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, informsRemoteProviderWhenConsumerGoneForUnregisteredContent)
    {
        comp.newParticipantHasConnected(remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({}, {34}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Unregistered, AnimationInformation{3, 4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Unregistered, AnimationInformation{3,4}));

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, doesNotInformLocalProviderWhenConsumerGoneForPreviouslyConsumedContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({34}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Shown, AnimationInformation{3,4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Registered, AnimationInformation{3,4}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        Mock::VerifyAndClearExpectations(&provider);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, doesNotInformRemoteProviderWhenConsumerGoneForPreviouslyConsumedContent)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({}, {34}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Shown, AnimationInformation{3, 4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Shown, AnimationInformation{3,4}));
        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(34), EDcsmStatus::Registered, AnimationInformation{3, 4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Registered, AnimationInformation{3,4}));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, informsLocalProviderWhenConsumerGoneForPreviouslyToggledConsumedContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({34}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Shown, AnimationInformation{3,4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::Registered, AnimationInformation{3,4}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{3, 4}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        Mock::VerifyAndClearExpectations(&provider);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(34), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, doesNotDispatchFromUnknownParticipant)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({6, 7, 1, 3}, {8}, true);

        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleCanvasSizeChange(ContentID(7), SizeInfo{1, 2}, AnimationInformation{3, 4}, otherRemoteId);
        comp.handleRequestUnregisterContent(ContentID(8), remoteId);
        comp.handleRequestUnregisterContent(ContentID(9), otherRemoteId);
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, otherRemoteId);
        comp.handleRegisterContent(ContentID(2), Category(10), otherRemoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(4), otherRemoteId);
        comp.handleContentAvailable(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), otherRemoteId);

        EXPECT_CALL(provider, canvasSizeChange(ramses::ContentID(6), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{3, 4}, remoteId));
        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(8)));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, doesNotDispatchFromDisconnectedParticipant)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.participantHasDisconnected(remoteId);

        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleRequestUnregisterContent(ContentID(8), remoteId);
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleRegisterContent(ContentID(2), Category(10), remoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(4), remoteId);
        comp.handleContentAvailable(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);

        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, doesNotSendInvalidEnumValues)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID(3), static_cast<ETechnicalContentType>(100), TechnicalContentDescriptor(11)));
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(1), static_cast<EDcsmStatus>(12), AnimationInformation{7, 8}));
    }

    TEST_F(ADcsmComponent, doesNotHandleInvalidEnumValues)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleContentStatusChange(ContentID(1), static_cast<EDcsmStatus>(12), AnimationInformation{7, 8}, remoteId);
        comp.handleContentAvailable(ContentID(3), static_cast<ETechnicalContentType>(100), TechnicalContentDescriptor(11), remoteId);
    }

    TEST_F(ADcsmComponent, doesNotSendToInvalidParticipantAsProvider)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID(9)));

        EXPECT_FALSE(comp.sendContentAvailable(remoteId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(remoteId, ContentID(9)));

        comp.newParticipantHasConnected(remoteId);
        EXPECT_FALSE(comp.sendContentAvailable(Guid(false), ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(Guid(false), ContentID(9)));
    }

    TEST_F(ADcsmComponent, doesNotSendToInvalidParticipantAsConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(12), SizeInfo{11, 10}, AnimationInformation{200, 100}));
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));

        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(12), SizeInfo{11, 10}, AnimationInformation{200, 100}));
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));

        comp.newParticipantHasConnected(remoteId);
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(12), SizeInfo{11, 10}, AnimationInformation{200, 100}));
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));
    }

    TEST_F(ADcsmComponent, doesNotHandleFromInvalidParticipant)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, localId);
        comp.handleRequestUnregisterContent(ContentID(8), localId);
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, localId);
        comp.handleRegisterContent(ContentID(2), Category(10), localId);
        comp.handleCategoryContentSwitchRequest(ContentID(4), localId);
        comp.handleContentAvailable(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), localId);

        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleRequestUnregisterContent(ContentID(8), remoteId);
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleRegisterContent(ContentID(2), Category(10), remoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(4), remoteId);
        comp.handleContentAvailable(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);

        comp.newParticipantHasConnected(remoteId);
        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, Guid(false));
        comp.handleRequestUnregisterContent(ContentID(8), Guid(false));
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, Guid(false));
        comp.handleRegisterContent(ContentID(2), Category(10), Guid(false));
        comp.handleCategoryContentSwitchRequest(ContentID(4), Guid(false));
        comp.handleContentAvailable(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), Guid(false));
    }

    TEST_F(ADcsmComponent, doesNotSendAsProviderWithoutEnablingFirst)
    {
        comp.setLocalProviderAvailability(false);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_FALSE(comp.sendRegisterContent(ContentID(1), Category(3)));
        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID(9)));
        EXPECT_FALSE(comp.sendRequestUnregisterContent(ContentID(8)));
    }

    TEST_F(ADcsmComponent, doesNotSendAsProviderAfterDisabling)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_FALSE(comp.sendRegisterContent(ContentID(1), Category(3)));
        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID(9)));
        EXPECT_FALSE(comp.sendRequestUnregisterContent(ContentID(8)));
    }

    TEST_F(ADcsmComponent, doesNotSendAsConsumerWithoutEnablingFirst)
    {
        comp.setLocalConsumerAvailability(false);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(12), SizeInfo{11, 10}, AnimationInformation{200, 100}));
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));
    }

    TEST_F(ADcsmComponent, doesNotSendAsConsumerAfterDisabling)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));

        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(12), SizeInfo{11, 10}, AnimationInformation{200, 100}));
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(34), EDcsmStatus::ContentRequested, AnimationInformation{3,4}));
    }

    TEST_F(ADcsmComponent, doesNotDispatchWhenNotEnabled)
    {
        EXPECT_FALSE(comp.dispatchProviderEvents(provider));
        EXPECT_FALSE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_FALSE(comp.dispatchProviderEvents(provider));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_FALSE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_FALSE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, canDispatchProviderEventsAfterSenderDisconnected)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        doRegisterContent({7}, {}, false);

        EXPECT_CALL(comm, sendDcsmRegisterContent(remoteId, ContentID(7), Category(3)));
        comp.newParticipantHasConnected(remoteId);
        comp.handleCanvasSizeChange(ContentID(7), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_CALL(provider, canvasSizeChange(ramses::ContentID(7), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{3, 4}, remoteId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, canDispatchConsumerEventsAfterSenderDisconnected)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        comp.newParticipantHasConnected(remoteId);
        comp.handleRegisterContent(ContentID(2), Category(10), remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(2), ramses::Category(10)));
        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, ignoresParticipantStateChangeWhenRemoteIdEqualsLocal)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({2}, {}, true);

        comp.newParticipantHasConnected(localId);
        comp.handleCategoryContentSwitchRequest(ContentID{2}, localId);

        EXPECT_TRUE(comp.sendCategoryContentSwitchRequest(localId, ContentID{2}));
        EXPECT_CALL(consumer, categoryContentSwitchRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(localId);

        EXPECT_TRUE(comp.sendCategoryContentSwitchRequest(localId, ContentID{2}));
        EXPECT_CALL(consumer, categoryContentSwitchRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, clearsProviderQueueWhenEnabledAndDisabled)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleCanvasSizeChange(ContentID(7), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);

        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, clearsConsumerQueueWhenEnabledAndDisabled)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(4), remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, errorWhenLocalProviderStateSetToCurrentState)
    {
        EXPECT_FALSE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_FALSE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_FALSE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, errorWhenLocalConsumerStateSetToCurrentState)
    {
        EXPECT_FALSE(comp.setLocalConsumerAvailability(false));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_FALSE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_FALSE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, cannotSendAsProviderWhenNotProvidingContent)
    {
        comp.setLocalProviderAvailability(true);
        comp.setLocalConsumerAvailability(true);
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {3}, true);

        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID{3}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{1}));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID{3}));
        EXPECT_FALSE(comp.sendRequestUnregisterContent(ContentID{3}));
    }

    TEST_F(ADcsmComponent, cannotSendToLocalWhenNoExistingLocalConsumer)
    {
        comp.setLocalProviderAvailability(true);
        doRegisterContent({3}, {}, false);

        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID{3}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{1}));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID{3}));
    }

    TEST_F(ADcsmComponent, cannotRegisterContentLocallyTwice)
    {
        comp.setLocalProviderAvailability(true);
        comp.setLocalConsumerAvailability(true);

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(1), Category(3)));
        EXPECT_FALSE(comp.sendRegisterContent(ContentID(1), Category(4)));

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotRegisterContentRemotelyTwice)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.setLocalConsumerAvailability(true);

        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);
        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotRegisterContentRemotelyWhenAlreadyRegisteredLocal)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(1), Category(3)));
        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotRegisterContentLocallyWhenAlreadyRegisteredRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);

        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);
        EXPECT_FALSE(comp.sendRegisterContent(ContentID(1), Category(3)));

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, canRegisterContentLocallyWhenUnregisteredBefore)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(1), Category(3)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(1)));

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(1), Category(3)));

        InSequence seq;
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, canRegisterContentRemotelyWhenUnregisteredBefore)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(1), Category(3)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(1)));

        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);

        InSequence seq;
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotUnregisterRemoteContentFromLocal)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        comp.handleRegisterContent(ContentID(1), Category(3), remoteId);
        EXPECT_FALSE(comp.sendRequestUnregisterContent(ContentID(1)));

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotUnregisterLocalContentFromRemote)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendRegisterContent(ContentID(1), Category(3)));
        comp.handleRequestUnregisterContent(ContentID(1), remoteId);

        EXPECT_CALL(consumer, registerContent(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotControlNonExistingContentAsRemote)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        comp.handleCanvasSizeChange(ContentID(1), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleContentAvailable(ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(1), remoteId);
        comp.handleRequestUnregisterContent(ContentID(1), remoteId);
    }

    TEST_F(ADcsmComponent, cannotControlContentFromOtherAsRemoteProvider)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        comp.handleContentAvailable(ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);
        comp.handleCategoryContentSwitchRequest(ContentID(1), remoteId);
        comp.handleRequestUnregisterContent(ContentID(1), remoteId);
    }

    TEST_F(ADcsmComponent, remoteProviderDisconnectRemovesAllContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        doRegisterContent({1, 3}, {2, 5}, false, {8});

        comp.participantHasDisconnected(remoteId);
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(3)));
        EXPECT_EQ(otherRemoteId, comp.getContentProviderID(ContentID(8)));
        EXPECT_EQ(Guid(false), comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentProviderID(ContentID(5)));
    }

    TEST_F(ADcsmComponent, localProviderDisconnectRemovesAllContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1, 3}, {2, 5}, false);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(1)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(3)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_EQ(Guid(false), comp.getContentProviderID(ContentID(1)));
        EXPECT_EQ(Guid(false), comp.getContentProviderID(ContentID(3)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(5)));
    }

    TEST_F(ADcsmComponent, localProviderCannotUnregisterTwiceWhileUnregisterStillPending)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{200, 100}));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{200, 100}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(1)));

        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));
        EXPECT_FALSE(comp.sendRequestUnregisterContent(ContentID(1)));
    }

    TEST_F(ADcsmComponent, remoteProviderCannotUnregisterTwiceWhileUnregisterStillPending)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {1}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::Shown, AnimationInformation{200, 100}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{200, 100}));
        comp.handleRequestUnregisterContent(ContentID(1), remoteId);

        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(1)));
        comp.handleRequestUnregisterContent(ContentID(1), remoteId);
    }

    TEST_F(ADcsmComponent, localConsumerDisableDoesNotTouchContentFromOtherConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{7, 8}, remoteId);
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(1)));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{7, 8}, remoteId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));
    }

    TEST_F(ADcsmComponent, remoteConsumerDisconnectDoesNotTouchContentFromLocalConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{200, 100}));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(1)));

        comp.participantHasDisconnected(remoteId);

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{200, 100}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        comp.dispatchConsumerEvents(consumer);
    }

    TEST_F(ADcsmComponent, removesUnregisterRequestedContentFromLocalProviderWhenLocalConsumerSendStateUnregister)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestUnregisterContent(ContentID(1)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Unregistered, AnimationInformation{0, 0}));
        EXPECT_EQ(Guid(false), comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Unregistered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        comp.dispatchConsumerEvents(consumer);
    }

    TEST_F(ADcsmComponent, removesUnregisterRequestedContentFromRemoteProviderWhenLocalConsumerSendStateUnregister)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {1}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        comp.handleRequestUnregisterContent(ContentID(1), remoteId);
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::Unregistered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Unregistered, AnimationInformation{0, 0}));
        EXPECT_EQ(Guid(false), comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        comp.dispatchConsumerEvents(consumer);
    }

    TEST_F(ADcsmComponent, keepLocallyProvidedContentWhenNoActiveConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, keepRemotelyProvidedContentWhenNoActiveConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {1}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(1)));
    }

    TEST_F(ADcsmComponent, localProviderGoesBackToRegisteredWhenLocalConsumerGoneAfterContentRequested)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProviderGoesBackToRegisteredWhenLocalConsumerGoneAfterShown)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Registered, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, remoteProviderGoesBackToRegisteredWhenLocalConsumerGoneAfterContentRequested)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {1}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(1)));
    }

    TEST_F(ADcsmComponent, remoteProviderGoesBackToRegisteredWhenLocalConsumerGoneAfterShown)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {1}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}));
        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::Registered, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(1)));
    }

    TEST_F(ADcsmComponent, remoteConsumerCannotControlLocallyConsumedContentInStateContentRequested)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}, remoteId);
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, remoteConsumerCannotControlLocallyConsumedContentInStateShown)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}));
        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}, remoteId);
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{0, 0}, localId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localConsumerCannotControlRemotelyConsumedContentInStateContentRequested)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}, remoteId);
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Ready, ramses::AnimationInformation{0, 0}, remoteId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localConsumerCannotControlRemotelyConsumedContentInStateShown)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        comp.handleContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}, remoteId);
        EXPECT_FALSE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}));

        EXPECT_CALL(provider, contentStatusChange(ramses::ContentID(1), ramses::EDcsmStatus::Shown, ramses::AnimationInformation{0, 0}, remoteId));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }


    /*
      - can reregister content after local provider is gone

      - remote consumer disconnect, local provider
        - unregister req pendidng -> state unregistered (local) + remove
        - state registered: nothing happens
        - state != registred -> send register to local

      - local provider, unreg req content + state change
        - unreq req + unreg or reg -> remoev
        - > ready, shown -> keep
        - no unreg req -> keep

      - local provider, remote consumer, handle state chnage
        - unreq req + unreg or reg -> remoev
        - > ready, shown -> keep
        - no unreg req -> keep

      - disallow canvas size change (local, remote) when not active consumer (??)

    TEST_F(ADcsmComponent, )
    {
    }

     */
}
