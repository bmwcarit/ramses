//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmProvider.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "DcsmProviderImpl.h"
#include "DcsmEventHandlerMocks.h"
#include "Components/IDcsmProviderEventHandler.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"


namespace ramses
{
    using namespace testing;

    class DcsmComponentMock : public ramses_internal::IDcsmComponent
    {
    public:
        MOCK_METHOD(bool, dispatchProviderEvents, (ramses_internal::IDcsmProviderEventHandler&), (override));
        MOCK_METHOD(bool, sendOfferContent, (ramses_internal::ContentID, ramses_internal::Category, ramses_internal::ETechnicalContentType, const std::string&, bool), (override));
        MOCK_METHOD(bool, sendContentDescription, (ramses_internal::ContentID, ramses_internal::TechnicalContentDescriptor), (override));
        MOCK_METHOD(bool, sendContentReady, (ramses_internal::ContentID), (override));
        MOCK_METHOD(bool, sendRequestStopOfferContent, (ramses_internal::ContentID), (override));
        MOCK_METHOD(bool, sendUpdateContentMetadata, (ramses_internal::ContentID contentID, const ramses_internal::DcsmMetadata& metadata), (override));

        MOCK_METHOD(bool, sendCanvasSizeChange, (ramses_internal::ContentID, const ramses_internal::CategoryInfo&, ramses_internal::AnimationInformation), (override));
        MOCK_METHOD(bool, sendContentStateChange, (ramses_internal::ContentID, ramses_internal::EDcsmState, const ramses_internal::CategoryInfo&, ramses_internal::AnimationInformation), (override));
        MOCK_METHOD(bool, dispatchConsumerEvents, (IDcsmConsumerEventHandler&), (override));

        MOCK_METHOD(bool, setLocalProviderAvailability, (bool), (override));
        MOCK_METHOD(bool, setLocalConsumerAvailability, (bool), (override));
        MOCK_METHOD(bool, sendContentEnableFocusRequest, (ramses_internal::ContentID contentID, int32_t focusRequest), (override));
        MOCK_METHOD(bool, sendContentDisableFocusRequest, (ramses_internal::ContentID contentID, int32_t focusRequest), (override));

    };

    class ADcsmProvider : public Test
    {
    protected:
        ADcsmProvider()
            : compMock()
            , handler()
        {
            // we expect exactly two setLocalProviderAvailability calls
            EXPECT_CALL(compMock, setLocalProviderAvailability(true)).WillOnce(Return(true));
            EXPECT_CALL(compMock, setLocalProviderAvailability(false)).WillOnce(Return(true));
            provider.reset(new DcsmProvider(*new DcsmProviderImpl(compMock)));

            // "initialize" event handler for easier testing
            EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce(Return(true));
            provider->dispatchEvents(handler);

            const std::u32string mdDescription(U"sometext");
            metadataCreator.setPreviewDescription(mdDescription);
            metadata.setPreviewDescription(mdDescription);
        }

        StrictMock<DcsmComponentMock> compMock;
        StrictMock<DcsmProviderEventHandlerMock> handler;
        std::unique_ptr<DcsmProvider> provider;

        DcsmMetadataCreator metadataCreator;
        ramses_internal::DcsmMetadata metadata;

        CategoryInfoUpdate categoryInfo{{ 800u, 600u }, { 0u, 0u, 800u, 600u }};
        CategoryInfoUpdate categoryInfoDummy{{ 1u, 1u }, { 0u, 0u, 1u, 1u }};
        ContentID id = ContentID(123);
    };

    TEST_F(ADcsmProvider, callsLocalProviderAvailabilityOnConstructionAndDestruction)
    {
        DcsmComponentMock localCompMock;

        EXPECT_CALL(localCompMock, setLocalProviderAvailability(true));
        DcsmProviderImpl localProvider(localCompMock);
        EXPECT_CALL(localCompMock, setLocalProviderAvailability(false));
    }

    TEST_F(ADcsmProvider, callsDispatchOnDcsmClientWhenDispatchIsCalledAndReturnsOK)
    {
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce([this](auto& h)
        {
            EXPECT_EQ(&provider->impl, &h);
            return true;
        });
        EXPECT_EQ(provider->dispatchEvents(handler), StatusOK);
    }

    TEST_F(ADcsmProvider, canOfferContent)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
    }

    TEST_F(ADcsmProvider, canOfferContentLocallyOnly)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", true)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalOnly), StatusOK);
    }

    TEST_F(ADcsmProvider, canOfferContentWithMetadata)
    {
        InSequence seq;
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendUpdateContentMetadata(ramses_internal::ContentID(123), metadata)).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, canOfferContentWithWaylandSurfaceId)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::WaylandIviSurfaceID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(
            compMock,
            sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(5432)))
            .WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), waylandIviSurfaceId_t(5432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
    }

    TEST_F(ADcsmProvider, canOfferContentWithMetadataWithWaylandSurfaceId)
    {
        InSequence seq;
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::WaylandIviSurfaceID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(
            compMock,
            sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(5432)))
            .WillOnce(Return(true));
        EXPECT_CALL(compMock, sendUpdateContentMetadata(ramses_internal::ContentID(123), metadata)).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContentWithMetadata(id, Category(567), waylandIviSurfaceId_t(5432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, willIgnoreAllButOneOfferContent)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_NE(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalOnly), StatusOK);
        EXPECT_NE(provider->offerContent(id, Category(5678), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_NE(provider->offerContent(id, Category(5678), sceneId_t(4321), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_NE(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, willIgnoreOfferContentForInvalidCategory)
    {
        EXPECT_NE(provider->offerContent(id, Category::Invalid(), sceneId_t(432), EDcsmOfferingMode::LocalOnly), StatusOK);
        EXPECT_NE(provider->offerContentWithMetadata(id, Category::Invalid(), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, willIgnoreAllButOneOfferContentAfterOfferWithMetadata)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendUpdateContentMetadata(ramses_internal::ContentID(123), metadata)).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
        EXPECT_NE(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalOnly, metadataCreator), StatusOK);
        EXPECT_NE(provider->offerContent(id, Category(5678), sceneId_t(4321), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_NE(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, willAcceptSameSceneIDForDifferentContents)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);

        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(124), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(124), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(ContentID(124), Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
    }

    TEST_F(ADcsmProvider, willAutomaticallyReplyContentReadyIfContentIsMarkedReadyAndNotCallCallback)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation{});

        EXPECT_CALL(compMock, sendContentReady(ramses_internal::ContentID(123))).WillOnce(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
    }

    TEST_F(ADcsmProvider, willCallCallbackIfContentNotMarkedReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(id));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
    }

    TEST_F(ADcsmProvider, willReplyContentReadyOnceContentIsMarkedReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(_));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, ignoresStateChangeRequestToSameState)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        EXPECT_CALL(handler, contentShow(_, _));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfoDummy, AnimationInformation());
        EXPECT_CALL(handler, contentHide(_, _));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsCallbackForEachSizeChangedWithCorrectSizeInfoAndAnimInformation)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);

        CategoryInfoUpdate categoryInfoUpdate1{ {133, 337}, {0, 0, 133, 337} };
        CategoryInfoUpdate categoryInfoUpdate2{{ 555, 900 }, { 0, 0, 555, 900 }};
        CategoryInfoUpdate categoryInfoUpdate3{{ 1920, 800 }, { 0, 0, 1920, 800 }};

        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation{ 12345, 67890 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(categoryInfoUpdate1, infoupdate);
        });
        provider->impl.contentSizeChange(id, categoryInfoUpdate1, AnimationInformation{ 12345, 67890 });

        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation{ 875, 2 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(categoryInfoUpdate2, infoupdate);
        });
        provider->impl.contentSizeChange(id, categoryInfoUpdate2, AnimationInformation{ 875, 2 });

        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation{ 91385675902, 8237492095 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            EXPECT_EQ(categoryInfoUpdate3, infoupdate);
        });
        provider->impl.contentSizeChange(id, categoryInfoUpdate3, AnimationInformation{ 91385675902, 8237492095 });
    }

    TEST_F(ADcsmProvider, callsCallbackForEachStatusChangedWithCorrectAnimInformation)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(_)).WillOnce([this](ContentID id_)
        {
            EXPECT_EQ(id, id_);
            EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
            EXPECT_EQ(provider->markContentReady(id_), StatusOK);
        });
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation{ 91385675902, 8237492095 }));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfoDummy, AnimationInformation{ 91385675902, 8237492095 });
        EXPECT_CALL(handler, contentHide(id, AnimationInformation{ 123, 89576 }));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation{ 123, 89576 });
    }

    TEST_F(ADcsmProvider, callsSizeChangedCallbackOnStatusChangedToAssignedFromOffered)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, doesNotCallSizeChangedCallbackOnStatusChangedToAssignedFromAboveButContentReleasedCallback)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation{ 325, 43290587 }));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation{ 325, 43290587 });
    }

    TEST_F(ADcsmProvider, doesNotCallSizeChangedCallbackOnStatusChangedToAnyOtherStateThanAssignedUpwards)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentReady(_)).Times(AtLeast(1)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(AtLeast(1)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentShow(id, AnimationInformation())).Times(AtLeast(1));
        EXPECT_CALL(handler, contentHide(id, AnimationInformation())).Times(AtLeast(1));
        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).Times(AtLeast(1));
        EXPECT_CALL(handler, contentReadyRequested(id)).Times(AtLeast(1));
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation{ 91385675902, 8237492095 })).Times(1);

        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);

        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfo, AnimationInformation());
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfo, AnimationInformation());

        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, categoryInfo, AnimationInformation{ 91385675902, 8237492095 });
    }

    TEST_F(ADcsmProvider, sendsRequestStopOfferMessageAndCallsStopOfferAcceptedOnStatusChange)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);

        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentShow(_, _));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);

        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation{ 91385675902, 8237492095 }));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, categoryInfoDummy, AnimationInformation{ 91385675902, 8237492095 });
    }

    TEST_F(ADcsmProvider, sendsEnableContentFocusRequestMessageToConsumerIfUserCallsEnableContentFocusRequest)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        provider->markContentReady(id);
        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        EXPECT_CALL(compMock, sendContentEnableFocusRequest(ramses_internal::ContentID(123), 17)).WillOnce(Return(true));

        EXPECT_EQ(provider->enableFocusRequest(id, 17u), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnStopOfferContent)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_NE(provider->requestStopOfferContent(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, returnsErrorOnBadFocusRequestNumber)
    {
        EXPECT_NE(provider->enableFocusRequest(id, 0), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnEnableFocusRequest)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        provider->markContentReady(id);
        EXPECT_CALL(compMock, sendContentEnableFocusRequest(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->enableFocusRequest(id, 14), StatusOK);
        EXPECT_NE(provider->enableFocusRequest(ContentID(1234), 14), StatusOK);
    }

    TEST_F(ADcsmProvider, givesEnableFocusRequestToComponentForHandling)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        provider->markContentReady(id);
        EXPECT_CALL(compMock, sendContentEnableFocusRequest(ramses_internal::ContentID(id.getValue()), 64));
        EXPECT_NE(provider->enableFocusRequest(id, 64), StatusOK);

        EXPECT_CALL(compMock, sendContentEnableFocusRequest(ramses_internal::ContentID(id.getValue()), 13));
        EXPECT_NE(provider->enableFocusRequest(id, 13), StatusOK);
    }

    TEST_F(ADcsmProvider, givesDisableFocusRequestToComponentForHandling)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        provider->markContentReady(id);
        EXPECT_CALL(compMock, sendContentDisableFocusRequest(ramses_internal::ContentID(id.getValue()), 13));
        EXPECT_NE(provider->disableFocusRequest(id, 13), StatusOK);

        EXPECT_CALL(compMock, sendContentDisableFocusRequest(ramses_internal::ContentID(id.getValue()), 31));
        EXPECT_NE(provider->disableFocusRequest(id, 31), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnMarkContentReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        EXPECT_CALL(handler, contentReadyRequested(_));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        EXPECT_NE(provider->markContentReady(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, marksContentReadyOnlyOnce)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
        EXPECT_CALL(handler, contentReadyRequested(_));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_NE(provider->markContentReady(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, callsNoCanvasSizeCallbackIfDcsmSizeMessageHasUnknownContentID)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        CategoryInfoUpdate categoryInfoUpdate{{555, 900}, {0, 0, 555, 900}};
        provider->impl.contentSizeChange(ContentID(124), categoryInfoUpdate, AnimationInformation());
    }

    TEST_F(ADcsmProvider, doesNothingIfContentStateMessageHasWrongContentID)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        provider->impl.contentStateChange(ContentID(124), ramses_internal::EDcsmState::Assigned, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(ContentID(124), ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(ContentID(124), ramses_internal::EDcsmState::Shown, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(ContentID(124), ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(ContentID(124), ramses_internal::EDcsmState::Offered, categoryInfoDummy, AnimationInformation());
        provider->impl.contentStateChange(ContentID(124), ramses_internal::EDcsmState::AcceptStopOffer, categoryInfoDummy, AnimationInformation());
    }

    TEST_F(ADcsmProvider, allowsToReofferContentWithSameOrDifferentParameters)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, stopOfferAccepted(_, _));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, stopOfferAccepted(_, _));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(5678), sceneId_t(4321), EDcsmOfferingMode::LocalOnly), StatusOK);
    }

    TEST_F(ADcsmProvider, offerContentFailsIfSendOfferContentFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillOnce(Return(false));
        EXPECT_NE(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
    }

    TEST_F(ADcsmProvider, offerContentWithMetadataFailsIfSendOfferContentFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillOnce(Return(false));
        EXPECT_NE(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, offerContentWithMetadataFailsIfSendMetadataFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendUpdateContentMetadata(_, _)).WillOnce(Return(false));
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).WillOnce(Return(true));
        EXPECT_NE(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, requestStopOfferContentFailsIfSendRequestUnregisterContentFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).WillOnce(Return(false));
        EXPECT_NE(provider->requestStopOfferContent(id), StatusOK);
    }

    TEST_F(ADcsmProvider, markContentReadyFailsIfsendContentReadyFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentReadyRequested(_));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(false));
        EXPECT_NE(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, enableContentFocusRequestFailsIfSendEnableContentFocusRequestFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentReadyRequested(_));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendContentEnableFocusRequest(ramses_internal::ContentID(id.getValue()), _)).WillOnce(Return(false));
        EXPECT_NE(provider->enableFocusRequest(id, 3), StatusOK);
    }

    TEST_F(ADcsmProvider, disableContentFocusRequestFailsIfSendDisableContentFocusRequestFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentReadyRequested(_));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendContentDisableFocusRequest(ramses_internal::ContentID(id.getValue()), _)).WillOnce(Return(false));
        EXPECT_NE(provider->disableFocusRequest(id, 3), StatusOK);
    }

    TEST_F(ADcsmProvider, dispatchesToTheHandlerProvidedByUser)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce([this](auto& h)
        {
            CategoryInfoUpdate categoryInfoUpdate{{ 123, 321 }, { 0, 0, 123, 321 }};
            EXPECT_CALL(handler, contentSizeChange(_, _, _));
            h.contentSizeChange(id, categoryInfoUpdate, AnimationInformation());
            return true;
        });
        EXPECT_EQ(provider->dispatchEvents(handler), StatusOK);

        DcsmProviderEventHandlerMock handler2;
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce([&handler2, this](auto& h)
        {
            CategoryInfoUpdate categoryInfoUpdate{{ 1234, 4321 }, { 0, 0, 1234, 4321 }};
            EXPECT_CALL(handler2, contentSizeChange(_, _, _));
            h.contentSizeChange(id, categoryInfoUpdate, AnimationInformation());
            return true;
        });
        EXPECT_EQ(provider->dispatchEvents(handler2), StatusOK);
    }

    TEST_F(ADcsmProvider, callsShowCallbackIfNewStateIsShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation{ 325, 43290587 }));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfo, AnimationInformation{ 325, 43290587 });
    }

    TEST_F(ADcsmProvider, callsHideCallbackIfNewStateIsReadyAndWasShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfo, AnimationInformation());

        EXPECT_CALL(handler, contentHide(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsReleaseCallbackIfNewStateIsAssignedAndWasShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfo, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsReleaseCallbackIfNewStateIsAssignedAndWasReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsStopOfferCallbackIfNewStateIsUnregisteredAndWasShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Shown, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(1).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation{ 325, 43290587 }));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, categoryInfo, AnimationInformation{ 325, 43290587 });
    }

    TEST_F(ADcsmProvider, callsStopOfferCallbackIfNewStateIsUnregisteredAndWasReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(1).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsStopOfferCallbackIfNewStateIsUnregisteredAndWasAssigned)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(1).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, doesNotCallReleaseCallbackIfNewStateIsOfferedAndWasAssigned)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Offered, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, needsANewMarkReadyAfterContentHasBeenSetToAssigned)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(id));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, needsANewMarkReadyAfterContentHasBeenSetToOffered)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Offered, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(id));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, allowsUserToCallMarkReadyWithinContentReleaseCallback)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillRepeatedly(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).WillOnce([this](auto id_, auto)
        {
            provider->markContentReady(id_);
        });
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());
    }

    TEST_F(ADcsmProvider, doesNotCallSendContentReadyWhenContentHasBeenMarkedReadyButNotRequestedAfterUnreadying)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).WillOnce([this](auto id_, auto)
        {
            provider->markContentReady(id_);
        });
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfoDummy, AnimationInformation());
    }


    TEST_F(ADcsmProvider, doesNotCallSendContentReadyWhenContentHasBeenMarkedReadyButNotRequestedAfterReassigning)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_)).WillOnce(Return(true));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Ready, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).WillOnce([this](auto id_, auto)
        {
            provider->markContentReady(id_);
        });
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Offered, categoryInfoDummy, AnimationInformation());

        EXPECT_CALL(handler, contentSizeChange(id, _, AnimationInformation()));
        provider->impl.contentStateChange(id, ramses_internal::EDcsmState::Assigned, categoryInfo, AnimationInformation());
    }

    TEST_F(ADcsmProvider, canUpdateContentMetadata)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(compMock, sendUpdateContentMetadata(ramses_internal::ContentID(123), metadata)).WillOnce(Return(true));
        EXPECT_EQ(provider->updateContentMetadata(id, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, sendsAllCurrentFocusRequestOnAssignment)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);

        EXPECT_CALL(compMock, sendContentEnableFocusRequest(ramses_internal::ContentID(123), 3)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentEnableFocusRequest(ramses_internal::ContentID(123), 5)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentEnableFocusRequest(ramses_internal::ContentID(123), 17)).WillOnce(Return(true));

        provider->enableFocusRequest(id, 3);
        provider->enableFocusRequest(id, 5);
        provider->enableFocusRequest(id, 17);

    }

    TEST_F(ADcsmProvider, updateContentMetadataFailsWithoutOffer)
    {
        EXPECT_NE(provider->updateContentMetadata(id, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, updateContentMetadataFailsWhenSendUpdateContentMetadataFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(ramses_internal::ContentID(123), ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(compMock, sendUpdateContentMetadata(ramses_internal::ContentID(123), metadata)).WillOnce(Return(false));
        EXPECT_NE(provider->updateContentMetadata(id, metadataCreator), StatusOK);
    }

    TEST_F(ADcsmProvider, willForgetContentIfSendContentDescriptionFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillOnce(Return(false));
        EXPECT_CALL(compMock, sendRequestStopOfferContent(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_NE(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(_, _));
        CategoryInfoUpdate categoryInfoUpdate{{ 0,0 }, { 0, 0, 0, 0 }};
        provider->impl.contentStateChange(ContentID(123), ramses_internal::EDcsmState::AcceptStopOffer, categoryInfoUpdate, { 0, 0 });

        // if content is not forgotten, provider should return false and not call component
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
    }

    TEST_F(ADcsmProvider, willForgetContentIfSendUpdateContentMetaDataFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendUpdateContentMetadata(ramses_internal::ContentID(123), metadata)).WillOnce(Return(false));
        EXPECT_CALL(compMock, sendRequestStopOfferContent(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_NE(provider->offerContentWithMetadata(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote, metadataCreator), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(_, _));
        CategoryInfoUpdate categoryInfoUpdate{{ 0,0 }, { 0, 0, 0, 0 }};
        provider->impl.contentStateChange(ContentID(123), ramses_internal::EDcsmState::AcceptStopOffer, categoryInfoUpdate, { 0, 0 });

        // if content is not forgotten, provider should return false and not call component
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567), ramses_internal::ETechnicalContentType::RamsesSceneID, "", false)).WillOnce(Return(true));
        EXPECT_CALL(compMock, sendContentDescription(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
    }

    class ADcsmProviderFromFramework : public Test
    {
    public:
        ADcsmProviderFromFramework()
        {}

        RamsesFramework fw;
        StrictMock<DcsmProviderEventHandlerMock> handler;
    };

    TEST_F(ADcsmProviderFromFramework, canCreateAndDestroy)
    {
        DcsmProvider* c = fw.createDcsmProvider();
        ASSERT_TRUE(c != nullptr);
        EXPECT_EQ(StatusOK, fw.destroyDcsmProvider(*c));
    }

    TEST_F(ADcsmProviderFromFramework, canCreateAndLetFrameworkDestroy)
    {
        DcsmProvider* c = fw.createDcsmProvider();
        ASSERT_TRUE(c != nullptr);
    }

    TEST_F(ADcsmProviderFromFramework, cannotCreateTwiceOnSameFramework)
    {
        DcsmProvider* c1 = fw.createDcsmProvider();
        ASSERT_TRUE(c1 != nullptr);
        DcsmProvider* c2 = fw.createDcsmProvider();
        ASSERT_TRUE(c2 == nullptr);
    }

    TEST_F(ADcsmProviderFromFramework, cannotDestroyProviderFromOtherFramework)
    {
        DcsmProvider* c = fw.createDcsmProvider();
        ASSERT_TRUE(c != nullptr);

        RamsesFramework otherFw;
        EXPECT_NE(StatusOK, otherFw.destroyDcsmProvider(*c));
    }

    TEST_F(ADcsmProviderFromFramework, canCreateAndDestroyProviderOnDifferentFrameworksSimultaneously)
    {
        DcsmProvider* c1 = fw.createDcsmProvider();
        ASSERT_TRUE(c1 != nullptr);

        RamsesFramework otherFw;
        DcsmProvider* c2 = otherFw.createDcsmProvider();
        ASSERT_TRUE(c2 != nullptr);

        EXPECT_EQ(StatusOK, fw.destroyDcsmProvider(*c1));
        EXPECT_EQ(StatusOK, otherFw.destroyDcsmProvider(*c2));
    }

    TEST_F(ADcsmProviderFromFramework, emptyDispatchSucceeds)
    {
        DcsmProvider* c = fw.createDcsmProvider();
        ASSERT_TRUE(c != nullptr);
        EXPECT_EQ(StatusOK, c->dispatchEvents(handler));
    }
}
