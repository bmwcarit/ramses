//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/DcsmComponent.h"
#include "TransportCommon/FakeConnectionStatusUpdateNotifier.h"
#include "RamsesFrameworkTypesImpl.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "CommunicationSystemMock.h"
#include "DcsmEventHandlerMocks.h"
#include "gmock/gmock.h"
#include "DcsmGmockPrinter.h"
#include "DcsmMetadataUpdateImpl.h"
#include "TestPngHeader.h"

#include <array>
#include "Utils/CommandLineParser.h"
#include "Utils/RamsesLogger.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "Components/CategoryInfo.h"

namespace ramses_internal
{
    using namespace ::testing;

    class ADcsmComponent : public ::testing::Test
    {
    public:
        explicit ADcsmComponent(bool startConnected = true)
            : localId(444)
            , remoteId(555)
            , otherRemoteId(666)
            , comp(localId, comm, connNotifier, frameworkLock)
        {
            static bool initializedLogger = false;
            if (!initializedLogger)
            {
                initializedLogger = true;
                std::array<const char* const, 3> args{"", "-l", "info"};
                GetRamsesLogger().initialize(CommandLineParser{static_cast<Int>(args.size()), args.data()}, "TEST", "TEST", true, true);
            }
            if (startConnected)
            {
                EXPECT_TRUE(comp.connect());
            }

            std::vector<unsigned char> pngHeader = TestPngHeader::GetValidHeader();
            EXPECT_TRUE(metadata.setPreviewImagePng(pngHeader.data(), pngHeader.size()));
            EXPECT_TRUE(metadata.setPreviewDescription(U"abc"));
            EXPECT_TRUE(metadata.setWidgetOrder(123));
            EXPECT_TRUE(metadata.setWidgetBackgroundID(456));
            EXPECT_TRUE(metadata.setWidgetHUDLineID(678));
            EXPECT_TRUE(metadata.setCarModel(1234));
            EXPECT_TRUE(metadata.setCarModelView({ 1,2,3,4,5,6,7,0.1f,0.2f }, { 8,9 }));
            EXPECT_TRUE(metadata.setCarModelVisibility(true));
            EXPECT_TRUE(metadata.setExclusiveBackground(true));
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

        void getContentToState_LPLC(uint32_t id, CS state, bool hasLocalConsumer = true, bool offersLocalOnly = false)
        {
            if (static_cast<int>(state) >= static_cast<int>(CS::Unknown))
            {
                EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Offered))
            {
                if (!offersLocalOnly)
                    EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(id), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));

                EXPECT_TRUE(comp.sendOfferContent(ContentID(id), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", offersLocalOnly));
                EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentOffered(ramses::ContentID(id), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Assigned))
            {
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Assigned, CategoryInfo(2, 3), AnimationInformation{1, 2}));
                EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
                    ramses::CategoryInfoUpdate categoryInfo{};
                    categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
                    EXPECT_EQ(categoryInfo, infoupdate);
                    });
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                EXPECT_TRUE(comp.sendContentDescription(ContentID(id), TechnicalContentDescriptor(5)));
                EXPECT_CALL(consumer, contentDescription(ramses::ContentID(id), ramses::TechnicalContentDescriptor(5)));
                EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::ReadyRequested))
            {
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
                EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Ready, _, ramses::AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Ready))
            {
                EXPECT_TRUE(comp.sendContentReady(ContentID(id)));
                EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentReady(ramses::ContentID(id)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Shown))
            {
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
                EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Shown, _, ramses::AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::StopOfferRequested))
            {
                if (!offersLocalOnly)
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
                comp.handleOfferContent(ContentID(id), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", usedRemote);
                EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentOffered(ramses::ContentID(id), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Assigned))
            {
                EXPECT_CALL(comm, sendDcsmContentStateChange(usedRemote, ContentID(id), EDcsmState::Assigned, _, AnimationInformation{1, 2}));
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{1, 2}));
                EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();

                comp.handleContentDescription(ContentID(id), TechnicalContentDescriptor(5), usedRemote);
                EXPECT_CALL(consumer, contentDescription(ramses::ContentID(id), ramses::TechnicalContentDescriptor(5)));
                EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::ReadyRequested))
            {
                EXPECT_CALL(comm, sendDcsmContentStateChange(usedRemote, ContentID(id), EDcsmState::Ready, _, AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
                EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Ready))
            {
                comp.handleContentReady(ContentID(id), usedRemote);
                EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentReady(ramses::ContentID(id)));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Shown))
            {
                EXPECT_CALL(comm, sendDcsmContentStateChange(usedRemote, ContentID(id), EDcsmState::Shown, _, AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
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
                EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(id), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));

                EXPECT_TRUE(comp.sendOfferContent(ContentID(id), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
                EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(id)));
                if (hasLocalConsumer)
                {
                    EXPECT_CALL(consumer, contentOffered(ramses::ContentID(id), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
                    EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
                }
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Assigned))
            {
                comp.handleContentStateChange(ContentID(id), EDcsmState::Assigned, CategoryInfo(2, 3), AnimationInformation{1, 2}, usedRemote);
                EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
                    ramses::CategoryInfoUpdate categoryInfo{};
                    categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
                    EXPECT_EQ(categoryInfo, infoupdate);
                    });
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                EXPECT_CALL(comm, sendDcsmContentDescription(usedRemote, ContentID(id), TechnicalContentDescriptor(5)));
                EXPECT_TRUE(comp.sendContentDescription(ContentID(id), TechnicalContentDescriptor(5)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::ReadyRequested))
            {
                comp.handleContentStateChange(ContentID(id), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}, usedRemote);
                EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Ready, _, ramses::AnimationInformation{4, 5}));
                EXPECT_TRUE(comp.dispatchProviderEvents(provider));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Ready))
            {
                EXPECT_CALL(comm, sendDcsmContentReady(usedRemote, ContentID(id)));
                EXPECT_TRUE(comp.sendContentReady(ContentID(id)));
                EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(id)));
                ensureNoEventsPending();
            }
            if (static_cast<int>(state) >= static_cast<int>(CS::Shown))
            {
                comp.handleContentStateChange(ContentID(id), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}, remoteId);
                EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(id)));
                EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Shown, _, ramses::AnimationInformation{4, 5}));
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
        DcsmMetadata metadata;
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
        Guid p1(123);
        Guid p2(543);
        comp.newParticipantHasConnected(p1);
        comp.newParticipantHasConnected(p2);
        comp.participantHasDisconnected(p1);
        comp.participantHasDisconnected(p2);
        // expect no events
    }

    TEST_F(ADcsmComponent, unknownParticipantDisconnect)
    {
        comp.participantHasDisconnected(Guid(654));
        comp.participantHasDisconnected(Guid(765));
    }

    TEST_F(ADcsmComponent, offerFailsWithInvalidContentID)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        ensureNoEventsPending();
        EXPECT_FALSE(comp.sendOfferContent(ContentID::Invalid(), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
    }

    TEST_F(ADcsmComponent, offerFailsWithInvalidCategory)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        ensureNoEventsPending();
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category::Invalid(), ETechnicalContentType::RamsesSceneID, "mycontent", false));
    }

    TEST_F(ADcsmComponent, offerFailsWithInvalidTechnicalContentType)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        ensureNoEventsPending();
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::Invalid, "mycontent", false));
    }

    TEST_F(ADcsmComponent, lateConsumerGetsRemoteRegister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(1)));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(2), ramses::ETechnicalContentType::RamsesSceneID));
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
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(2), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.participantHasDisconnected(remoteId);
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(1)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, lateProviderDoesNotGetEventFromRemoteRegister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, earlyProviderDoesNotGetEventFromRemoteRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, connectRegisterDisconnectDoesNoEvent)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
    }

    TEST_F(ADcsmComponent, noConsumerEventWhenRemoteRegisterAfterRemove)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));

        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, noUnregisterAfterConsumerRemoved)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(2), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndLateConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndLateConsumer_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndEarlyConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterToNetworkAndEarlyConsumer_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(4)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localRegisterLateNetworkGetsRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, localRegisterLateNetworkGetsNoRegister_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));

        comp.newParticipantHasConnected(remoteId);

        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, localRegisterEarlyNetworkGetsbroadcastRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, localRegisterEarlyNetworkGetsNoBroadcastRegister_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));

        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponent, duplicateConnectSendRegisterAgain)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, multipleConnectDisconnectGetsRegister)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        comp.newParticipantHasConnected(remoteId);
        comp.participantHasDisconnected(remoteId);

        EXPECT_CALL(comm, sendDcsmOfferContent(remoteId, ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        comp.newParticipantHasConnected(remoteId);
        comp.participantHasDisconnected(remoteId);
    }

    TEST_F(ADcsmComponent, onlyLocalRegistrationsAreSentOut)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleOfferContent(ContentID(1), Category(3), ETechnicalContentType::RamsesSceneID, "", remoteId);
        comp.handleOfferContent(ContentID(2), Category(2), ETechnicalContentType::RamsesSceneID, "", otherRemoteId);
        comp.handleOfferContent(ContentID(4), Category(9), ETechnicalContentType::RamsesSceneID, "", remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(11), Category(8), ETechnicalContentType::RamsesSceneID, "mycontent2"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(11), Category(8), ETechnicalContentType::RamsesSceneID, "mycontent2", false));

        Guid lateRemoteId(9000);
        EXPECT_CALL(comm, sendDcsmOfferContent(lateRemoteId, ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_CALL(comm, sendDcsmOfferContent(lateRemoteId, ContentID(11), Category(8), ETechnicalContentType::RamsesSceneID, "mycontent2"));
        comp.newParticipantHasConnected(lateRemoteId);

        comp.participantHasDisconnected(remoteId);
        comp.participantHasDisconnected(otherRemoteId);
        comp.participantHasDisconnected(lateRemoteId);
    }

    TEST_F(ADcsmComponent, multipleRegistrationsArriveAtLateConsumer)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        comp.handleOfferContent(ContentID(1), Category(3), ETechnicalContentType::RamsesSceneID, "", remoteId);
        comp.handleOfferContent(ContentID(2), Category(2), ETechnicalContentType::RamsesSceneID, "", otherRemoteId);
        comp.handleOfferContent(ContentID(4), Category(9), ETechnicalContentType::RamsesSceneID, "", remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(11), Category(8), ETechnicalContentType::RamsesSceneID, "mycontent2"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(11), Category(8), ETechnicalContentType::RamsesSceneID, "mycontent2", false));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(9), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(2), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(10), ramses::Category(9), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(11), ramses::Category(8), ramses::ETechnicalContentType::RamsesSceneID));
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
        comp.handleOfferContent(ContentID(1), Category(3), ETechnicalContentType::RamsesSceneID, "", remoteId);
        comp.handleOfferContent(ContentID(2), Category(2), ETechnicalContentType::RamsesSceneID, "", otherRemoteId);
        comp.handleOfferContent(ContentID(4), Category(9), ETechnicalContentType::RamsesSceneID, "", remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(11), Category(8), ETechnicalContentType::RamsesSceneID, "mycontent2"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(11), Category(8), ETechnicalContentType::RamsesSceneID, "mycontent2", false));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(1), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(4), ramses::Category(9), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(2), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(10), ramses::Category(9), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(11), ramses::Category(8), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localProviderDoesNotLeaveTracesAfterRemoval)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent", false));
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
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(10)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(10)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(10), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        Mock::VerifyAndClearExpectations(&comm);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        comp.newParticipantHasConnected(remoteId);
    }

    TEST_F(ADcsmComponent, remoteProviderDoesNotLeaveTracesAfterUnregister)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(10), Category(9), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        comp.handleRequestStopOfferContent(ContentID(10), remoteId);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, clearsProviderQueueWhenEnabledAndDisabled)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo(2, 3), AnimationInformation{1, 2}));

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
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(7), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(7), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(7)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(7)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(7), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 0, 0 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
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
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Unknown_Offered_Unknown_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));

        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);

        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{2, 3}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Assigned_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered, true, true);

        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)).WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_Offered_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned, true, true);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{ 1, 2 }));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_ReadyRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
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
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_StopOfferRequested_Unknown_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned, true, true);

        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
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
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_StopOfferRequested_Unknown_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested, true, true);

        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 4, 5 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_ReadyRequested_Ready)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        EXPECT_TRUE(comp.sendContentReady(ContentID(2)));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentReady(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{4, 5}));
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
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_StopOfferRequested_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready, true, true);

        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 4, 5 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Ready_Shown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Shown, _, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_Ready)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, _, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{4, 5}));
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
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Shown_StopOfferRequested_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown, true, true);

        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 4, 5 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_StopOfferRequested_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested);

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_StopOfferRequested_Offered_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested, true, true);

        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Unknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Unknown);

        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{1, 2}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{0, 0}));
        EXPECT_FALSE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        // EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        EXPECT_FALSE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Offered)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);

        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{0, 0}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));

        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(2), 32));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Assigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{1, 2}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(2), 32));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentDisableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), _, ramses::AnimationInformation{ 5, 6 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 1u, 2u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_ReadyRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::ReadyRequested);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{1, 2}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5})); //
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        // EXPECT_FALSE(comp.sendContentReady(ContentID(2), ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(1)));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(2), 32));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentDisableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), _, ramses::AnimationInformation{ 5, 6 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 1u, 2u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Ready)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Ready);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{1, 2}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5})); //
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(2), 32));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentDisableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), _, ramses::AnimationInformation{ 5, 6 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 1u, 2u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_Shown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Shown);

        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{1, 2}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, SizeInfo{0, 0}, AnimationInformation{4, 5})); //
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        // EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), _, ramses::AnimationInformation{ 5, 6 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 1u, 2u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, localProvider_nonTransitions_StopOfferRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested);

        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{1, 2}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        // EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}));
        // EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3)));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));

        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 32));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), _, ramses::AnimationInformation{ 5, 6 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 1u, 2u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_AllStates)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmUpdateContentMetadata(remoteId, ContentID(2), metadata));
        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{2, 3}, AnimationInformation{1, 2}, remoteId);
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));

        EXPECT_CALL(comm, sendDcsmContentEnableFocusRequest(remoteId, ContentID(2), 32));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}, remoteId);
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(comm, sendDcsmContentReady(remoteId, ContentID(2)));
        EXPECT_TRUE(comp.sendContentReady(ContentID(2)));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}, remoteId);
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Shown, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        comp.handleCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}, remoteId);
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentSizeChange(ramses::ContentID(2), _, ramses::AnimationInformation{ 5, 6 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 1u, 2u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentConsumerID(ContentID(2)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{4, 5})).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
                });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderRemoteConsumer_AllStates_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Unknown_Offered_Unknown)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        comp.handleOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        comp.handleRequestStopOfferContent(ContentID(2), remoteId);
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Offered_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Offered);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, CategoryInfo{2, 3}, AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{2, 3}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));

    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{1, 2}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_ReadyRequested)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
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

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_ReadyRequested_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::ReadyRequested);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_ReadyRequested_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::ReadyRequested);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));
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

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_ReadyRequested_Ready)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::ReadyRequested);

        comp.handleContentReady(ContentID(2), remoteId);
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));

        EXPECT_CALL(consumer, contentReady(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Ready_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Ready_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
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

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Ready_Shown)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Shown_Ready)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Shown);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Shown_Assigned)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Shown);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(localId, comp.getContentConsumerID(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Shown_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Shown);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));
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

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_StopOfferRequested_Offered)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::StopOfferRequested);

        comp.handleOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, handleFunctionsDoNothingWithoutLocalReceiver)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleCanvasSizeChange(ContentID(6), CategoryInfo{1, 2}, AnimationInformation{3, 4}, remoteId);
        comp.handleContentStateChange(ContentID(1), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{7, 8}, remoteId);
        comp.handleOfferContent(ContentID(2), Category(10), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        comp.handleContentReady(ContentID(3), remoteId);
        comp.handleContentEnableFocusRequest(ContentID(4), 32, remoteId);
        comp.handleRequestStopOfferContent(ContentID(5), remoteId);
        comp.handleUpdateContentMetadata(ContentID(8), metadata, remoteId);
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
        EXPECT_FALSE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        EXPECT_FALSE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_FALSE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_FALSE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
    }

    TEST_F(ADcsmComponent, sendFailsWithoutActiveConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{}, AnimationInformation{1, 2}));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Disable_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
    }


    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Disable_Consumer_localOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered, true, true);
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

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Offered_Disable_Provider_localOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered, true, true);

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
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_Disable_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_Assigned_Disable_Consumer)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, _, AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));
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

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_Assigned_Disable_Provider_localOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned, true, true);

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
        EXPECT_EQ(Guid(), comp.getContentConsumerID(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_StopOfferRequested_Disable_Consumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested);

        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, stateFlow_RemoteProviderLocalConsumer_StopOfferRequested_Disable_Consumer)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::StopOfferRequested);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{0, 0}));
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

    TEST_F(ADcsmComponent, stateFlow_LocalProviderLocalConsumer_StopOfferRequested_Disable_Provider_localOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::StopOfferRequested, true, true);

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

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{0, 0}));
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
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(3), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        getContentToState_RPLC(3, CS::Offered, remoteId, false);
        EXPECT_EQ(remoteId, comp.getContentProviderID(ContentID(3)));
    }

    TEST_F(ADcsmComponent, canRemotelyOfferContentPreviouslyOfferedByLocal_LocalOnly)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPLC(3, CS::Offered, false, true);
        EXPECT_EQ(localId, comp.getContentProviderID(ContentID(3)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(3)));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(3)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(3), EDcsmState::AcceptStopOffer, _,  ramses::AnimationInformation{ 0, 0 }));
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
        EXPECT_FALSE(comp.sendContentReady(ContentID(3)));
    }

    TEST_F(ADcsmComponent, sendContentReadyFailsWhenContentStateUnknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, sendContentReadyFailsWhenContentProvidedByOther)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_RPLC(2, CS::ReadyRequested);
        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
    }

    TEST_F(ADcsmComponent, sendContentDescriptionFailsWithInvalidTechnicalContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_FALSE(comp.sendContentDescription(ContentID::Invalid(), TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();

        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
    }

    TEST_F(ADcsmComponent, sendEnableContentFocusRequestFailsWhenNotProvidingContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_RPLC(2, CS::Assigned);

        EXPECT_FALSE(comp.sendContentEnableFocusRequest(ContentID(2), 32));
        EXPECT_FALSE(comp.sendContentDisableFocusRequest(ContentID(2), 32));
    }

    TEST_F(ADcsmComponent, sendCanvasSizeChangeFailsForContentStateUnknown)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        EXPECT_FALSE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{1, 2}, AnimationInformation{5, 6}));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
    }

    TEST_F(ADcsmComponent, canSendCanvasSizeChangeToRemoteProvider)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Ready);
        EXPECT_CALL(comm, sendDcsmCanvasSizeChange(remoteId, ContentID(2), CategoryInfo{10, 11}, AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.sendCanvasSizeChange(ContentID(2), CategoryInfo{10, 11}, AnimationInformation{1, 2}));
    }

    TEST_F(ADcsmComponent, sendContentStateChangeFailsForInvalidState)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);
        EXPECT_FALSE(comp.sendContentStateChange(ContentID(2), static_cast<EDcsmState>(8888), CategoryInfo{}, AnimationInformation{4, 5}));
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
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(11), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{4, 5}));
        comp.logInfo();
        comp.triggerLogMessageForPeriodicLog();
        ignorePendingEvents();
    }

    TEST_F(ADcsmComponent, forceStopOfferForLocalContentOnDisconnect)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);

        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(5), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent2"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(5), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent2", false));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(4)));
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(5)));
        EXPECT_TRUE(comp.disconnect());
    }

    TEST_F(ADcsmComponent, forceStopOfferForLocalContentOnDisconnect_LocalOnly)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.handleOfferContent(ContentID(1), Category(2), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);

        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(5), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent2", true));

        EXPECT_TRUE(comp.disconnect());
    }

    TEST_F(ADcsmComponent, canSendMetadataUpdateWithoutConsumer)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(4), metadata));
    }

    TEST_F(ADcsmComponent, cannotSendMetadataUpdateForUnknownContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_FALSE(comp.sendUpdateContentMetadata(ContentID(2), metadata));
    }

    TEST_F(ADcsmComponent, sendingSameFocusRequestFails)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(4), 17));
        EXPECT_FALSE(comp.sendContentEnableFocusRequest(ContentID(4), 17));
        EXPECT_FALSE(comp.sendContentEnableFocusRequest(ContentID(4), 17));
    }

    TEST_F(ADcsmComponent, sendingUnknownFocusReqeustDisableFails)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(4), 17));
        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(4), 17));
        EXPECT_FALSE(comp.sendContentDisableFocusRequest(ContentID(4), 15));
    }

    TEST_F(ADcsmComponent, sendingSameDisableFocusReqeustMultipleTimesFails)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));

        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(4), 17));
        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(4), 17));
        EXPECT_FALSE(comp.sendContentDisableFocusRequest(ContentID(4), 17));
    }

    TEST_F(ADcsmComponent, cannotSendFocusRequestMethodsForUnknownContent)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_FALSE(comp.sendContentEnableFocusRequest(ContentID(2), 17));
        EXPECT_FALSE(comp.sendContentDisableFocusRequest(ContentID(2), 17));
    }

    TEST_F(ADcsmComponent, storesMetadataUpdateForLateLocalConsumerAndSendsUpdatesAfterwards)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPLC(2, CS::Offered, false);

        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));

        DcsmMetadata updateDm;
        updateDm.setPreviewDescription(U"foobar");
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), updateDm));
        ensureNoEventsPending();

        DcsmMetadata mergedDm(metadata);
        mergedDm.setPreviewDescription(U"foobar");

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{2, 3}, AnimationInformation{1, 2}));

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(mergedDm, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        // delta update
        DcsmMetadata deltaDm;
        deltaDm.setPreviewDescription(U"testtext");
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), deltaDm));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(deltaDm, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, storesMetadataUpdateForLateRemoteConsumerAndSendsUpdatesAfterwards)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPRC(2, CS::Offered);

        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));

        DcsmMetadata updateDm;
        updateDm.setPreviewDescription(U"foobar");
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), updateDm));
        ensureNoEventsPending();

        DcsmMetadata mergedDm(metadata);
        mergedDm.setPreviewDescription(U"foobar");

        EXPECT_CALL(comm, sendDcsmUpdateContentMetadata(remoteId, ContentID(2), mergedDm)).WillOnce(Return(true));
        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{2, 3}, AnimationInformation{1, 2}, remoteId);

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{1, 2}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        // delta update
        DcsmMetadata deltaDm;
        deltaDm.setPreviewDescription(U"testtext");
        EXPECT_CALL(comm, sendDcsmUpdateContentMetadata(remoteId, ContentID(2), deltaDm)).WillOnce(Return(true));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), deltaDm));
    }

    TEST_F(ADcsmComponent, storesFocusRequestsForLateLocalConsumerAndSendsAfterwards)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPLC(2, CS::Offered, false);

        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 3));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 5));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 17));

        ensureNoEventsPending();

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));

        ramses::CategoryInfoUpdate categoryInfo{};
        categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, Eq(ByRef(categoryInfo)), ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 3));
        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 5));
        EXPECT_CALL(consumer, contentEnableFocusRequest(ramses::ContentID(2), 17));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(2), 17));
        EXPECT_CALL(consumer, contentDisableFocusRequest(ramses::ContentID(2), 17));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, storesFocusRequestForLateRemoteConsumerAndSendsAfterwards)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPRC(2, CS::Offered);

        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 3));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 5));
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 17));

        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmContentEnableFocusRequest(remoteId, ContentID(2), 3));
        EXPECT_CALL(comm, sendDcsmContentEnableFocusRequest(remoteId, ContentID(2), 5));
        EXPECT_CALL(comm, sendDcsmContentEnableFocusRequest(remoteId, ContentID(2), 17));

        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }, remoteId);

        ramses::CategoryInfoUpdate categoryInfo{};
        categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, Eq(ByRef(categoryInfo)), ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_CALL(comm, sendDcsmContentDisableFocusRequest(remoteId, ContentID(2), 17));
        EXPECT_TRUE(comp.sendContentDisableFocusRequest(ContentID(2), 17));
    }


    TEST_F(ADcsmComponent, canHandleMetadataUpdateFromRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        comp.handleUpdateContentMetadata(ContentID(2), metadata, remoteId);
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _)). WillOnce(Invoke([&](auto, auto& mdp) { EXPECT_EQ(metadata, mdp.impl.getMetadata()); }));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, ignoredMetadataUpdateInOfferedFromRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Offered);
        comp.handleUpdateContentMetadata(ContentID(2), metadata, remoteId);
    }

    TEST_F(ADcsmComponent, ignoresMetadataUpdateFromUnknownRemote)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.handleUpdateContentMetadata(ContentID(2), metadata, remoteId);
    }

    TEST_F(ADcsmComponent, ignoresMetadataUpdateForUnknownContentFromRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.handleUpdateContentMetadata(ContentID(2), metadata, remoteId);
    }

    TEST_F(ADcsmComponent, ignoresFocusRequestForUnknownContentFromRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.handleContentEnableFocusRequest(ContentID(2), 17, remoteId);
    }

    TEST_F(ADcsmComponent, ignoresMetadataUpdateFromWrongRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        comp.handleUpdateContentMetadata(ContentID(2), metadata, otherRemoteId);
    }

    TEST_F(ADcsmComponent, ignoresFocusRequestFromWrongRemote)
    {
        comp.newParticipantHasConnected(remoteId);
        comp.newParticipantHasConnected(otherRemoteId);
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_RPLC(2, CS::Assigned);

        comp.handleContentEnableFocusRequest(ContentID(2), 17, otherRemoteId);
    }

    TEST_F(ADcsmComponent, metadataClearedWhenLocalProviderGone)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPLC(2, CS::Offered, false);
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned, true);
    }

    TEST_F(ADcsmComponent, focusRequestClearedWhenLocalProviderGone)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPLC(2, CS::Offered, false);
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 17));

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));

        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned, true);
    }

    TEST_F(ADcsmComponent, metadataClearedWhenLocalProviderUnoffers)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPLC(2, CS::Offered, false);
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), metadata));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{0, 0}));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned, true);
    }

    TEST_F(ADcsmComponent, focusRequestClearedWhenLocalProviderUnoffers)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPLC(2, CS::Offered, false);
        EXPECT_TRUE(comp.sendContentEnableFocusRequest(ContentID(2), 17));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned, true);
    }

    TEST_F(ADcsmComponent, ignoresAnyMessageFromRemoteConsumerForLocalOnlyContent)
    {
        uint64_t id = 2;

        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(id)));
        ensureNoEventsPending();

        comp.handleContentStateChange(ContentID(id), EDcsmState::Assigned, { 1, 1 }, { 5, 5 }, remoteId);
        comp.handleCanvasSizeChange(ContentID(id), { 1, 1 }, { 5, 5 }, remoteId);

        EXPECT_TRUE(comp.sendOfferContent(ContentID(id), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", true));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(id)));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(id), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();

        comp.handleCanvasSizeChange(ContentID(id), { 1, 1 }, { 5, 5 }, remoteId);
        comp.handleContentStateChange(ContentID(id), EDcsmState::Assigned, { 1, 1 }, { 5, 5 }, remoteId);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(id)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_TRUE(comp.sendContentDescription(ContentID(id), TechnicalContentDescriptor(5)));
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(id), ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        comp.handleCanvasSizeChange(ContentID(id), { 1, 1 }, { 5, 5 }, remoteId);
        comp.handleContentStateChange(ContentID(id), EDcsmState::Ready, { 1, 1 }, { 5, 5 }, remoteId);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(id)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Ready, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_TRUE(comp.sendContentReady(ContentID(id)));
        EXPECT_EQ(CS::Ready, comp.getContentState(ContentID(id)));
        EXPECT_CALL(consumer, contentReady(ramses::ContentID(id)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();

        comp.handleCanvasSizeChange(ContentID(id), { 1, 1 }, { 5, 5 }, remoteId);
        comp.handleContentStateChange(ContentID(id), EDcsmState::Shown, { 1, 1 }, { 5, 5 }, remoteId);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(id), EDcsmState::Shown, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::Shown, comp.getContentState(ContentID(id)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(id), EDcsmState::Shown, _, ramses::AnimationInformation{ 4, 5 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        comp.handleCanvasSizeChange(ContentID(id), { 1, 1 }, { 5, 5 }, remoteId);
        comp.handleContentStateChange(ContentID(id), EDcsmState::Ready, { 1, 1 }, { 5, 5 }, remoteId);
        comp.handleContentStateChange(ContentID(id), EDcsmState::Assigned, { 1, 1 }, { 5, 5 }, remoteId);
        comp.handleContentStateChange(ContentID(id), EDcsmState::Offered, { 1, 1 }, { 5, 5 }, remoteId);

        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(id)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(id)));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(id)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, sendsAndDeliversContentDescriptionLocallyWhileAssigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Assigned);
    }

    TEST_F(ADcsmComponent, sendsAndDeliversContentDescriptionLocallyWhileReadyRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        getContentToState_LPLC(2, CS::Offered);

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{ 4, 5 }));
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, _, ramses::AnimationInformation{ 4, 5 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(2), ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, sendsContentDescriptionToRemoteWhileAssigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Assigned);
    }

    TEST_F(ADcsmComponent, sendsContentDescriptionToRemoteWhileReadyRequested)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Offered);

        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }, remoteId);
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        comp.handleContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{ 4, 5 }, remoteId);
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, _, ramses::AnimationInformation{ 4, 5 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(ramses::CategoryInfoUpdate(), infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, deliversContentDescriptionFromRemote)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_RPLC(2, CS::Assigned);
    }

    TEST_F(ADcsmComponent, rejectsSecondContentDescriptionCall)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Assigned);

        EXPECT_FALSE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, rejectsSendContentReadyCallWithoutPreviousSendContentDescription)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Offered);

        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }, remoteId);
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        comp.handleContentStateChange(ContentID(2), EDcsmState::Ready, CategoryInfo{}, AnimationInformation{ 4, 5 }, remoteId);
        EXPECT_EQ(CS::ReadyRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Ready, _, ramses::AnimationInformation{ 4, 5 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_FALSE(comp.sendContentReady(ContentID(2)));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, allowsToChangeContentDescriptionOnlyAfterReoffering)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Ready);

        EXPECT_FALSE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        ensureNoEventsPending();

        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 0u, 0u }, AnimationInformation{ 0, 0 }, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_FALSE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));

        comp.handleContentStateChange(ContentID(2), EDcsmState::Offered, CategoryInfo{ 0u, 0u }, AnimationInformation{ 0, 0 }, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Offered, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));

        EXPECT_FALSE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        comp.handleContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{ 0u, 0u }, AnimationInformation{ 0, 0 }, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        getContentToState_LPRC(2, CS::Offered);
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor(5)));
        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 0u, 0u }, AnimationInformation{ 0, 0 }, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, acceptsSendContentDescriptionCallWhenInOfferedStateAndSendsContentDescriptionAfterBeingAssigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Offered);

        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor(5)));
        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, sendsOutContentDescriptionBeforeMetaDataWhenCallingSendContentDescriptionInOfferedState)
    {
        comp.newParticipantHasConnected(remoteId);
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPRC(2, CS::Offered);
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));

        DcsmMetadata md;
        md.setPreviewDescription(U"woah");
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), md));
        ensureNoEventsPending();

        InSequence seq;
        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor{ 5 })).WillOnce(Return(true));
        EXPECT_CALL(comm, sendDcsmUpdateContentMetadata(remoteId, ContentID(2), md)).WillOnce(Return(true));
        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }, remoteId);

        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 })).WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate categoryInfo{};
            categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
            EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, emitsContentDescriptionCallbackBeforeMetaDataCallbackWhenCallingSendContentDescriptionInOfferedState)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPLC(2, CS::Offered);
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));

        DcsmMetadata md;
        md.setPreviewDescription(U"woah");
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(2), md));
        ensureNoEventsPending();

        comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 });

        InSequence seq;
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(2), ramses::TechnicalContentDescriptor{ 5 }));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
    }

    TEST_F(ADcsmComponent, emitsContentDescriptionCallbackBeforeMetaDataCallbackWhenArrivingFromNetworkLikeThis)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_RPLC(2, CS::Offered);

        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, _, AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo(2, 3), AnimationInformation{ 1, 2 }));
        ensureNoEventsPending();

        DcsmMetadata md;
        md.setPreviewDescription(U"woah");
        comp.handleContentDescription(ContentID(2), TechnicalContentDescriptor{ 5 }, remoteId);
        comp.handleUpdateContentMetadata(ContentID(2), md, remoteId);

        InSequence seq;
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(2), ramses::TechnicalContentDescriptor{ 5 }));
        EXPECT_CALL(consumer, contentMetadataUpdated(ramses::ContentID(2), _));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
    }

    TEST_F(ADcsmComponent, contentDescriptionClearedWhenLocalProviderUnoffers)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        getContentToState_LPRC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo(0u, 0u), AnimationInformation{ 1, 2 }, remoteId);
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
    }

    TEST_F(ADcsmComponent, contentDescriptionClearedWhenLocalProviderGetsDisabled)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        getContentToState_LPRC(2, CS::Assigned);
        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        getContentToState_LPRC(2, CS::Assigned);
    }

    TEST_F(ADcsmComponent, contentDescriptionClearedWhenRemoteProviderUnoffers)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        getContentToState_RPLC(2, CS::Assigned);

        comp.handleRequestStopOfferContent(ContentID(2), remoteId);
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::AcceptStopOffer, _, AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{ }, AnimationInformation{ 0, 0 }));
        ensureNoEventsPending();

        comp.handleOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, _, AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        comp.handleContentDescription(ContentID(2), TechnicalContentDescriptor(5), remoteId);
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(2), ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, contentDescriptionClearedWhenLocalConsumerGetsDisabled)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        getContentToState_RPLC(2, CS::Assigned);
        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Offered, _, AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(false));
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));

        comp.handleOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, _, AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        comp.handleContentDescription(ContentID(2), TechnicalContentDescriptor(5), remoteId);
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(2), ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, contentDescriptionClearedWhenRemoteProviderDisconnects)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        comp.newParticipantHasConnected(remoteId);

        getContentToState_RPLC(2, CS::Assigned);

        comp.participantHasDisconnected(remoteId);
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        comp.newParticipantHasConnected(remoteId);

        comp.handleOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", remoteId);
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_CALL(comm, sendDcsmContentStateChange(remoteId, ContentID(2), EDcsmState::Assigned, _, AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        comp.handleContentDescription(ContentID(2), TechnicalContentDescriptor(5), remoteId);
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(2), ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, contentDescriptionClearedWhenLocalProviderUnoffers_LocalConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(2)));
        EXPECT_EQ(CS::StopOfferRequested, comp.getContentState(ContentID(2)));
        EXPECT_CALL(consumer, contentStopOfferRequest(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 0, 0 }));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::AcceptStopOffer, _, ramses::AnimationInformation{ 0, 0 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent"));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_CALL(consumer, contentOffered(ramses::ContentID(2), ramses::Category(3), ramses::ETechnicalContentType::RamsesSceneID));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));

        EXPECT_TRUE(comp.sendContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo{ 2, 3 }, AnimationInformation{ 1, 2 }));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{ 1, 2 }));
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        EXPECT_CALL(consumer, contentDescription(ramses::ContentID(2), ramses::TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, contentDescriptionClearedWhenLocalProviderGetsDisabled_LocalConsumer)
    {
        EXPECT_TRUE(comp.setLocalConsumerAvailability(true));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));

        getContentToState_LPLC(2, CS::Assigned);

        EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(ContentID(2)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_EQ(CS::Unknown, comp.getContentState(ContentID(2)));
        EXPECT_CALL(consumer, forceContentOfferStopped(ramses::ContentID(2)));
        EXPECT_TRUE(comp.dispatchConsumerEvents(consumer));
        ensureNoEventsPending();

        getContentToState_LPLC(2, CS::Assigned);
    }

    TEST_F(ADcsmComponent, contentDescriptionCanBeSetAfterAssigned)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        getContentToState_LPRC(2, CS::Offered);

        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo(2, 3), AnimationInformation{1, 2}, remoteId);
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{1, 2}))
            .WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
                ramses::CategoryInfoUpdate categoryInfo{};
                categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
                EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        ensureNoEventsPending();
    }

    TEST_F(ADcsmComponent, acceptsWaylandSurfaceIDInDcsmContentDescription)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        comp.newParticipantHasConnected(remoteId);
        EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(ContentID(2), Category(3), ETechnicalContentType::WaylandIviSurfaceID, "mycontent"));

        EXPECT_TRUE(comp.sendOfferContent(ContentID(2), Category(3), ETechnicalContentType::WaylandIviSurfaceID, "mycontent", false));
        EXPECT_EQ(CS::Offered, comp.getContentState(ContentID(2)));
        ensureNoEventsPending();

        comp.handleContentStateChange(ContentID(2), EDcsmState::Assigned, CategoryInfo(2, 3), AnimationInformation{1, 2}, remoteId);
        EXPECT_EQ(CS::Assigned, comp.getContentState(ContentID(2)));
        EXPECT_CALL(provider, contentStateChange(ramses::ContentID(2), EDcsmState::Assigned, _, ramses::AnimationInformation{1, 2}))
            .WillOnce([&](const auto&, const auto&, const auto& infoupdate, const auto&) {
                ramses::CategoryInfoUpdate categoryInfo{};
                categoryInfo.setCategoryRect({ 0u, 0u, 2u, 3u });
                EXPECT_EQ(categoryInfo, infoupdate);
            });
        EXPECT_TRUE(comp.dispatchProviderEvents(provider));
        ensureNoEventsPending();

        EXPECT_CALL(comm, sendDcsmContentDescription(remoteId, ContentID(2), TechnicalContentDescriptor(5)));
        EXPECT_TRUE(comp.sendContentDescription(ContentID(2), TechnicalContentDescriptor(5)));
        ensureNoEventsPending();
    }

    class ADcsmComponentNotConnected : public ADcsmComponent
    {
    public:
        ADcsmComponentNotConnected()
            : ADcsmComponent(false)
        {}
    };

    TEST_F(ADcsmComponentNotConnected, hasProperConnectDisconnectChecking)
    {
        EXPECT_FALSE(comp.disconnect());
        EXPECT_TRUE(comp.connect());
        EXPECT_FALSE(comp.connect());
        EXPECT_TRUE(comp.disconnect());
        EXPECT_FALSE(comp.disconnect());
    }

    TEST_F(ADcsmComponentNotConnected, doesNotSendBroadcastsBeforeConnected)
    {
        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(5), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent2", false));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(4), metadata));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmComponentNotConnected, doesNotSendBroadcastsAfterDisconnect)
    {
        EXPECT_TRUE(comp.connect());
        EXPECT_TRUE(comp.disconnect());

        EXPECT_TRUE(comp.setLocalProviderAvailability(true));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(4), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent", false));
        EXPECT_TRUE(comp.sendOfferContent(ContentID(5), Category(3), ETechnicalContentType::RamsesSceneID, "mycontent2", false));
        EXPECT_TRUE(comp.sendUpdateContentMetadata(ContentID(4), metadata));
        EXPECT_TRUE(comp.sendRequestStopOfferContent(ContentID(4)));
        EXPECT_TRUE(comp.setLocalProviderAvailability(false));
    }

    // TODO: test everything force stop offer on disconnect(?)
}
