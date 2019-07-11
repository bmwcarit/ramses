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

        using CS = DcsmComponent::ContentState;

        void TearDown() override
        {
            if (ensureNoPendingEventsOnTearDown)
                ensureNoEventsPending();
        }

        void ignorePendingEvents()
        {
            ensureNoPendingEventsOnTearDown = false;
        }

        void ensureNoEventsPending()
        {
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

        void getContentToState_LPLC(uint32_t id, CS state, bool hasLocalConsumer = true)
        {
            if (static_cast<int>(state) >= static_cast<int>(CS::Unknown))
            {
                EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Offered))
            {
                EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(id), Category(3)));
                EXPECT_TRUE(comp.sendOfferContent(ContentID(id), Category(3)));
                EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentOffered(ramses::ContentID(id), ramses::Category(3)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Assigned))
            {
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}));
                EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Assigned, ramses::SizeInfo{2, 3}, ramses::AnimationInformation{1, 2}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::ReadyRequested))
            {
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
                EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Ready, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Ready))
            {
                EXPECT_TRUE(comp.sendContentReady(ContentID(id), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
                EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentReady(ramses::ContentID(id), ramses::ETechnicalContentType::RamsesSceneID, ramses::TechnicalContentDescriptor(5)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Shown))
            {
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
                EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Shown, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::StopOfferRequested))
            {
                EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(id)));
                EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(id)));
                EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(id)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) > static_cast<int>(CS::StopOfferRequested))
                assert(false);
        }

        void getContentToState_RPLC(uint32_t id, CS state, Guid usedRemote = Guid(), bool hasLocalConsumer = true)
        {
            if (usedRemote.isInvalid())
                usedRemote = remoteId;

            if (static_cast<int>(state) >= static_cast<int>(CS::Unknown))
            {
                EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Offered))
            {
                comp.handleOfferContent(ContentID(id), Category(3), usedRemote);
                EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentOffered(ramses::ContentID(id), ramses::Category(3)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Assigned))
            {
                EXPECT_CALL(comm, sendDcsmContentStateChange(usedRemote, ContentID(id), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}));
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}));
                EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::ReadyRequested))
            {
                EXPECT_CALL(comm, sendDcsmContentStateChange(usedRemote, ContentID(id), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
                EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Ready))
            {
                comp.handleContentReady(ContentID(id), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5), usedRemote);
                EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentReady(ramses::ContentID(id), ramses::ETechnicalContentType::RamsesSceneID, ramses::TechnicalContentDescriptor(5)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Shown))
            {
                EXPECT_CALL(comm, sendDcsmContentStateChange(usedRemote, ContentID(id), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
                EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::StopOfferRequested))
            {
                comp.handleRequestStopOfferContent(ContentID(id), usedRemote);
                EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(id)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) > static_cast<int>(CS::StopOfferRequested))
                assert(false);
        }

        void getContentToState_LPRC(uint32_t id, CS state, bool hasLocalConsumer = false, Guid usedRemote = Guid())
        {
            if (usedRemote.isInvalid())
                usedRemote = remoteId;

            if (static_cast<int>(state) >= static_cast<int>(CS::Unknown))
            {
                EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Offered))
            {
                EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(id), Category(3)));
                EXPECT_TRUE(comp.sendOfferContent(ContentID(id), Category(3)));
                EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentOffered(ramses::ContentID(id), ramses::Category(3)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Assigned))
            {
                comp.handleContentStateChange(ContentID(id), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}, usedRemote);
                EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Assigned, ramses::SizeInfo{2, 3}, ramses::AnimationInformation{1, 2}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::ReadyRequested))
            {
                comp.handleContentStateChange(ContentID(id), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}, usedRemote);
                EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Ready, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Ready))
            {
                EXPECT_CALL(comm, sendDcsmContentReady(usedRemote, ContentID(id), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
                EXPECT_TRUE(comp.sendContentReady(ContentID(id), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
                EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Shown))
            {
                comp.handleContentStateChange(ContentID(id), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}, remoteId);
                EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Shown, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::StopOfferRequested))
            {
                EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(id)));
                EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(id)));
                EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) > static_cast<int>(CS::StopOfferRequested))
                assert(false);
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
        bool ensureNoPendingEventsOnTearDown = true;
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
        comp.handleOfferContent(ContentID(1), Category(2), remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(1)));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        Mock::VerifyAndClearExpectations(&consumer);
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(remoteId);
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, earlyConsumerGetsRemoteRegister)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(remoteId);
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, lateProviderDoesNotGetEventFromRemoteRegister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, earlyProviderDoesNotGetEventFromRemoteRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), remoteId);
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, connectRegisterDisconnectDoesNoEvent)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
    }

    TEST_F(ADcsmComponent, noConsumerEventWhenRemoteRegisterAfterRemove)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));

        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), remoteId);
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, noUnregisterAfterConsumerRemoved)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndLateConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3)));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndEarlyConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterLateNetworkGetsRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, localRegisterEarlyNetworkGetsbroadcastRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, duplicateConnectSendRegisterAgain)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, multipleConnectDisconnectGetsRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3)));

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3)));
        comp.newParticipantHasConnected(remoteId);
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, onlyLocalRegistrationsAreSentOut)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleOfferContent(ContentID(1), Category(3), remoteId);
        comp.handleOfferContent(ContentID(2), Category(2), otherRemoteId);
        comp.handleOfferContent(ContentID(4), Category(9), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(11), Category(8)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(11), Category(8)));

        Guid lateRemoteId(true);
        EXPECT_CALL(comm, sendDcsmOfferContent(lateRemoteId, ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmOfferContent(lateRemoteId, ContentID(11), Category(8)));
        comp.newParticipantHasConnected(lateRemoteId);

        comp.participantHasDisconnected(remoteId);
        comp.participantHasDisconnected(otherRemoteId);
        comp.participantHasDisconnected(lateRemoteId);
    }

    TEST_F(ADcsmComponent, multipleRegistrationsArriveAtLateConsumer)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleOfferContent(ContentID(1), Category(3), remoteId);
        comp.handleOfferContent(ContentID(2), Category(2), otherRemoteId);
        comp.handleOfferContent(ContentID(4), Category(9), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(11), Category(8)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(11), Category(8)));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(9)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(2)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(10), ramses::Category(9)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(11), ramses::Category(8)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(remoteId);
        comp.participantHasDisconnected(otherRemoteId);
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(1)));
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(4)));
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, multipleRegistrationsArriveAtEarlyConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleOfferContent(ContentID(1), Category(3), remoteId);
        comp.handleOfferContent(ContentID(2), Category(2), otherRemoteId);
        comp.handleOfferContent(ContentID(4), Category(9), remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(11), Category(8)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(11), Category(8)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(9)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(2)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(10), ramses::Category(9)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(11), ramses::Category(8)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localProviderDoesNotLeaveTracesAfterRemoval)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(10)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        Mock::VerifyAndClearExpectations(&comm);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, localProviderDoesNotLeaveTracesAfterUnregister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(10)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(10)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(10), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        Mock::VerifyAndClearExpectations(&comm);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, remoteProviderDoesNotLeaveTracesAfterUnregister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(10), Category(9), remoteId);
        comp.handleRequestStopOfferContent(ContentID(10), remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, clearsProviderQueueWhenEnabledAndDisabled)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, clearsConsumerQueueWhenEnabledAndDisabled)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(7), Category(2)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(7), Category(2)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(7)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(7)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(7), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

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

    TEST_F(ADcsmComponent, ignoresRemoteConnectWithSameidAsLocal)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(localId);
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, ignoresRemoteDisconnectWithSameidAsLocal)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.participantHasDisconnected(localId);
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Unknown_Offered_Unknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3)));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, ramses::SizeInfo{2, 3}, ramses::AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_ReadyRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_StopOfferRequested_Unknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_StopOfferRequested_Unknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_Ready)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        EXPECT_TRUE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentReady(ramses::ContentID(2), ramses::ETechnicalContentType::RamsesSceneID, ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_StopOfferRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_Shown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Shown, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_Ready)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_StopOfferRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_StopOfferRequested_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested);

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Unknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Unknown);

        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{1, 2}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{0, 0}));
        // EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_FALSE(comp.sendContentFocusRequest(ContentID(2)));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);

        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{1, 2}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{0, 0}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_FALSE(comp.sendContentFocusRequest(ContentID(2)));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{1, 2}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_TRUE(comp.sendContentFocusRequest(ContentID(2)));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));

        EXPECT_CALL(consumer, contentFocusRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_ReadyRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{1, 2}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5})); //
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        // EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_TRUE(comp.sendContentFocusRequest(ContentID(2)));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));

        EXPECT_CALL(consumer, contentFocusRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Ready)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{1, 2}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5})); //
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_TRUE(comp.sendContentFocusRequest(ContentID(2)));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));

        EXPECT_CALL(consumer, contentFocusRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Shown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{1, 2}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5})); //
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_TRUE(comp.sendContentFocusRequest(ContentID(2)));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));

        EXPECT_CALL(consumer, contentFocusRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_StopOfferRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested);

        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{1, 2}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}));
        // EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_FALSE(comp.sendContentFocusRequest(ContentID(2)));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));

        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_AllStates)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}, remoteId);
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, ramses::SizeInfo{2, 3}, ramses::AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(comm, sendDcsmContentFocusRequest(remoteId, ContentID(2)));
        EXPECT_TRUE(comp.sendContentFocusRequest(ContentID(2)));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}, remoteId);
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(comm, sendDcsmContentReady(remoteId, ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}, remoteId);
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Shown, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        comp.handleCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}, remoteId);
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), ramses::SizeInfo{1, 2}, ramses::AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentConsumerID(ContentID(2)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Unknown_Offered_Unknown)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        comp.handleOfferContent(ContentID(2), Category(3), remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        comp.handleRequestStopOfferContent(ContentID(2), remoteId);
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3)));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Offered_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Offered);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{2, 3}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_ReadyRequested)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_StopOfferRequested_Unknown)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        comp.handleRequestStopOfferContent(ContentID(2), remoteId);
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_ReadyRequested_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::ReadyRequested);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_ReadyRequested_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::ReadyRequested);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_ReadyRequested_StopOfferRequested_Unknown)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::ReadyRequested);

        comp.handleRequestStopOfferContent(ContentID(2), remoteId);
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_ReadyRequested_Ready)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::ReadyRequested);

        comp.handleContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5), remoteId);
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentReady(ramses::ContentID(2), ramses::ETechnicalContentType::RamsesSceneID, ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Ready_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Ready_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Ready_StopOfferRequested)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);

        comp.handleRequestStopOfferContent(ContentID(2), remoteId);
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Ready_Shown)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Shown_Ready)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Shown);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Shown_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Shown);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Shown_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Shown);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Shown_StopOfferRequested)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Shown);

        comp.handleRequestStopOfferContent(ContentID(2), remoteId);
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_StopOfferRequested_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::StopOfferRequested);

        comp.handleOfferContent(ContentID(2), Category(3), remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, handleFunctionsDoNothingWithoutLocalReceiver)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleContentStateChange(ContentID(1), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{7, 8}, remoteId);
        comp.handleOfferContent(ContentID(2), Category(10), remoteId);
        comp.handleContentReady(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);
        comp.handleContentFocusRequest(ContentID(4), remoteId);
        comp.handleRequestStopOfferContent(ContentID(5), remoteId);
    }

    TEST_F(ADcsmComponent, remoteProviderDisconnectRemovesAllContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        getContentToState_LPLC(1, CS::Offered, false);
        getContentToState_LPLC(3, CS::Offered, false);
        getContentToState_RPLC(2, CS::Offered, remoteId, false);
        getContentToState_RPLC(5, CS::Offered, remoteId, false);
        getContentToState_RPLC(8, CS::Offered, otherRemoteId, false);

        comp.participantHasDisconnected(remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(1)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));

        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(3)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(3)));

        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(8)));
        EXPECT_EQ(otherRemoteId, comp.getContentProviderID(ContentID(8)));

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(5)));
    }

    TEST_F(ADcsmComponent, localProviderDisconnectRemovesAllContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPLC(1, CS::Offered, false);
        getContentToState_LPLC(3, CS::Offered, false);
        getContentToState_RPLC(2, CS::Offered, remoteId, false);
        getContentToState_RPLC(5, CS::Offered, remoteId, false);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(1)));
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(3)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(1)));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(3)));

        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));

        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(5)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(5)));
    }

    TEST_F(ADcsmComponent, sendFailsWithoutActiveProvider)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendContentFocusRequest(ContentID(2)));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
    }

    TEST_F(ADcsmComponent, sendFailsWithoutActiveConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{1, 2}));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Disable_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Offered_Disable_Consumer)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Offered);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Disable_Provider)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Offered_Disconnect_Provider)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Offered);

        comp.participantHasDisconnected(remoteId);
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_Offered_Disable_Provider)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Offered);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_Offered_Disconnect_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Offered);

        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_Disable_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_Disable_Consumer)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_Disable_Provider)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_Disconnect_Provider)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        comp.participantHasDisconnected(remoteId);
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_Assigned_Disable_Provider)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_Assigned_Disconnect_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Assigned);

        comp.participantHasDisconnected(remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(Guid(false), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_StopOfferRequested_Disable_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_StopOfferRequested_Disable_Consumer)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::StopOfferRequested);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_StopOfferRequested_Disable_Provider)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_StopOfferRequested_Disconnect_Provider)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::StopOfferRequested);

        comp.participantHasDisconnected(remoteId);
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_StopOfferRequested_Disable_Provider)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::StopOfferRequested);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_StopOfferRequested_Disconnect_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::StopOfferRequested);

        comp.participantHasDisconnected(remoteId);
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, canLocallyOfferContentPreviouslyOfferedByRemote)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_RPLC(3, CS::Offered, remoteId, false);
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(3)));
        comp.handleRequestStopOfferContent(ContentID(3), remoteId);
        // comp.handleForceStopOfferContent(ContentID(3), remoteId);
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(3)));

        getContentToState_LPLC(3, CS::Offered, false);
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(3)));
    }

    TEST_F(ADcsmComponent, canRemotelyOfferContentPreviouslyOfferedByLocal)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPLC(3, CS::Offered, false);
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(3)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(3)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(3)));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(3)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(3), EDcsmState::AcceptStopOffer, ramses::SizeInfo{0, 0}, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        getContentToState_RPLC(3, CS::Offered, remoteId, false);
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(3)));
    }

    TEST_F(ADcsmComponent, canRemotelyOfferContentPreviouslyOfferedByOtherRemote)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        getContentToState_RPLC(3, CS::Offered, remoteId, false);
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(3)));
        comp.handleForceStopOfferContent(ContentID(3), remoteId);
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(3)));

        getContentToState_RPLC(3, CS::Offered, otherRemoteId, false);
        EXPECT_EQ(otherRemoteId, comp.getContentProviderID(ContentID(3)));
    }

    TEST_F(ADcsmComponent, sendContentReadyFailsWhenContentNotKnown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);
        EXPECT_FALSE(comp.sendContentReady(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
    }

    TEST_F(ADcsmComponent, sendContentReadyFailsWhenContentStateUnknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, sendContentReadyFailsWhenContentProvidedByOther)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_RPLC(2, CS::ReadyRequested);
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
    }

    TEST_F(ADcsmComponent, sendContentReadyFailsWithOInvalidTechnicalContentType)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);
        EXPECT_FALSE(comp.sendContentReady(ContentID(2), static_cast<ETechnicalContentType>(99999), TechnicalContentDescriptor(5)));
    }

    TEST_F(ADcsmComponent, sendContentFocusRequestFailsWhenNotProvidingContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_FALSE(comp.sendContentFocusRequest(ContentID(2)));
    }

    TEST_F(ADcsmComponent, sendCanvasSizeChangeFailsForContentStateUnknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, canSendCanvasSizeChangeToRemoteProvider)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);
        EXPECT_CALL(comm, sendDcsmCanvasSizeChange(remoteId, ContentID(2), SizeInfo{10, 11}, AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), SizeInfo{10, 11}, AnimationInformation{1, 2}));
    }

    TEST_F(ADcsmComponent, sendContentStateChangeFailsForInvalidState)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), static_cast<EDcsmState>(8888), SizeInfo{0, 0}, AnimationInformation{4, 5}));
    }
    // TODO: remove provider/consumer does not affect other providers/consumers (test from assigned only?)
    // TODO: test handleForceStopOfferContent in every state + does not show for late new local consumer


    TEST_F(ADcsmComponent, canCallLogFunctions)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        getContentToState_LPLC(2, CS::Offered);
        getContentToState_LPLC(5, CS::Shown);
        getContentToState_LPLC(11, CS::Assigned);
        getContentToState_RPLC(1, CS::StopOfferRequested);
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(11)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(11)));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(11), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        comp.logInfo();
        comp.triggerLogMessageForPeriodicLog();
        ignorePendingEvents();
    }

#if 0

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
        comp.handleRequestStopOfferContent(ContentID(8), remoteId);
        comp.handleRequestStopOfferContent(ContentID(9), otherRemoteId);
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, otherRemoteId);
        comp.handleOfferContent(ContentID(2), Category(10), otherRemoteId);
        comp.handleContentFocusRequest(ContentID(4), otherRemoteId);
        comp.handleContentReady(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), otherRemoteId);

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
        comp.handleRequestStopOfferContent(ContentID(8), remoteId);
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleOfferContent(ContentID(2), Category(10), remoteId);
        comp.handleContentFocusRequest(ContentID(4), remoteId);
        comp.handleContentReady(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);

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
        comp.handleContentStateChange(ContentID(1), static_cast<EDcsmStatus>(12), AnimationInformation{7, 8}, remoteId);
        comp.handleContentReady(ContentID(3), static_cast<ETechnicalContentType>(100), TechnicalContentDescriptor(11), remoteId);
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
        comp.handleRequestStopOfferContent(ContentID(8), localId);
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, localId);
        comp.handleOfferContent(ContentID(2), Category(10), localId);
        comp.handleContentFocusRequest(ContentID(4), localId);
        comp.handleContentReady(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), localId);

        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleRequestStopOfferContent(ContentID(8), remoteId);
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleOfferContent(ContentID(2), Category(10), remoteId);
        comp.handleContentFocusRequest(ContentID(4), remoteId);
        comp.handleContentReady(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);

        comp.newParticipantHasConnected(remoteId);
        comp.handleCanvasSizeChange(ContentID(6), SizeInfo{1, 2}, AnimationInformation{3, 4}, Guid(false));
        comp.handleRequestStopOfferContent(ContentID(8), Guid(false));
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, Guid(false));
        comp.handleOfferContent(ContentID(2), Category(10), Guid(false));
        comp.handleContentFocusRequest(ContentID(4), Guid(false));
        comp.handleContentReady(ContentID(3), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), Guid(false));
    }

    TEST_F(ADcsmComponent, doesNotSendAsProviderWithoutEnablingFirst)
    {
        comp.setLocalProviderAvailability(false);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_FALSE(comp.sendOfferContent(ContentID(1), Category(3)));
        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID(9)));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(8)));
    }

    TEST_F(ADcsmComponent, doesNotSendAsProviderAfterDisabling)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_FALSE(comp.sendOfferContent(ContentID(1), Category(3)));
        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(5)));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID(9)));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(8)));
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
        comp.handleOfferContent(ContentID(2), Category(10), remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(10)));
        EXPECT_CALL(consumer, forceUnregisterContent(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, ignoresParticipantStateChangeWhenRemoteIdEqualsLocal)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        doRegisterContent({2}, {}, true);

        comp.newParticipantHasConnected(localId);
        comp.handleContentFocusRequest(ContentID{2}, localId);

        EXPECT_TRUE(comp.sendCategoryContentSwitchRequest(localId, ContentID{2}));
        EXPECT_CALL(consumer, categoryContentSwitchRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(localId);

        EXPECT_TRUE(comp.sendCategoryContentSwitchRequest(localId, ContentID{2}));
        EXPECT_CALL(consumer, categoryContentSwitchRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotSendAsProviderWhenNotProvidingContent)
    {
        comp.setLocalProviderAvailability(true);
        comp.setLocalConsumerAvailability(true);
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {3}, true);

        EXPECT_FALSE(comp.sendContentAvailable(localId, ContentID{3}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{1}));
        EXPECT_FALSE(comp.sendCategoryContentSwitchRequest(localId, ContentID{3}));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID{3}));
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

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(1), Category(3)));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(1), Category(4)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotRegisterContentRemotelyTwice)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.setLocalConsumerAvailability(true);

        comp.handleOfferContent(ContentID(1), Category(3), remoteId);
        comp.handleOfferContent(ContentID(1), Category(3), remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotRegisterContentRemotelyWhenAlreadyRegisteredLocal)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(1), Category(3)));
        comp.handleOfferContent(ContentID(1), Category(3), remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotRegisterContentLocallyWhenAlreadyRegisteredRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);

        comp.handleOfferContent(ContentID(1), Category(3), remoteId);
        EXPECT_FALSE(comp.sendOfferContent(ContentID(1), Category(3)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, canRegisterContentLocallyWhenUnregisteredBefore)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(1), Category(3)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(1)));

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(1), Category(3)));

        InSequence seq;
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, canRegisterContentRemotelyWhenUnregisteredBefore)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(1), Category(3)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(1)));

        comp.handleOfferContent(ContentID(1), Category(3), remoteId);

        InSequence seq;
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotUnregisterRemoteContentFromLocal)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        comp.handleOfferContent(ContentID(1), Category(3), remoteId);
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(1)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotUnregisterLocalContentFromRemote)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(1), Category(3)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(1), Category(3)));
        comp.handleRequestStopOfferContent(ContentID(1), remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, cannotControlNonExistingContentAsRemote)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);

        comp.handleCanvasSizeChange(ContentID(1), SizeInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Registered, AnimationInformation{7, 8}, remoteId);
        comp.handleContentReady(ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);
        comp.handleContentFocusRequest(ContentID(1), remoteId);
        comp.handleRequestStopOfferContent(ContentID(1), remoteId);
    }

    TEST_F(ADcsmComponent, cannotControlContentFromOtherAsRemoteProvider)
    {
        comp.setLocalConsumerAvailability(true);
        comp.setLocalProviderAvailability(true);
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        comp.handleContentReady(ContentID(1), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(11), remoteId);
        comp.handleContentFocusRequest(ContentID(1), remoteId);
        comp.handleRequestStopOfferContent(ContentID(1), remoteId);
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

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(1)));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(3)));
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

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(1)));

        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(1)));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(1)));
    }

    TEST_F(ADcsmComponent, remoteProviderCannotUnregisterTwiceWhileUnregisterStillPending)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({}, {1}, true);

        EXPECT_CALL(comm, sendDcsmContentStatusChange(remoteId, ContentID(1), EDcsmStatus::Shown, AnimationInformation{200, 100}));
        EXPECT_TRUE(comp.sendContentStatusChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{200, 100}));
        comp.handleRequestStopOfferContent(ContentID(1), remoteId);

        EXPECT_CALL(consumer, requestUnregisterContent(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(1)));
        comp.handleRequestStopOfferContent(ContentID(1), remoteId);
    }

    TEST_F(ADcsmComponent, localConsumerDisableDoesNotTouchContentFromOtherConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        doRegisterContent({1}, {}, true);

        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{7, 8}, remoteId);
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(1)));
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
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(1)));

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
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(1)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(1)));
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
        comp.handleRequestStopOfferContent(ContentID(1), remoteId);
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
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}, remoteId);
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
        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}, remoteId);
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

        comp.handleContentStateChange(ContentID(1), EDcsmStatus::ContentRequested, AnimationInformation{0, 0}, remoteId);
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

        comp.handleContentStateChange(ContentID(1), EDcsmStatus::Shown, AnimationInformation{0, 0}, remoteId);
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

      - TODO: stop offer requested -> offer

    TEST_F(ADcsmComponent, )
    {
    }

     */
#endif
}
