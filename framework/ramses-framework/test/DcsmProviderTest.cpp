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


namespace ramses
{
    using namespace testing;

    class DcsmComponentMock : public ramses_internal::IDcsmComponent
    {
    public:
        MOCK_METHOD1(dispatchProviderEvents, bool(ramses_internal::IDcsmProviderEventHandler&));
        MOCK_METHOD2(sendOfferContent, bool(ramses_internal::ContentID, ramses_internal::Category));
        MOCK_METHOD3(sendContentReady, bool(ramses_internal::ContentID, ramses_internal::ETechnicalContentType, ramses_internal::TechnicalContentDescriptor));
        MOCK_METHOD1(sendContentFocusRequest, bool(ramses_internal::ContentID));
        MOCK_METHOD1(sendRequestStopOfferContent, bool(ramses_internal::ContentID));
        MOCK_METHOD2(sendAcceptStopOffer, bool(ramses_internal::ContentID, ramses_internal::AnimationInformation));

        MOCK_METHOD3(sendCanvasSizeChange, bool(ramses_internal::ContentID, ramses_internal::SizeInfo, ramses_internal::AnimationInformation));
        MOCK_METHOD4(sendContentStateChange, bool(ramses_internal::ContentID, ramses_internal::EDcsmState, ramses_internal::SizeInfo, ramses_internal::AnimationInformation));
        MOCK_METHOD1(dispatchConsumerEvents, bool(IDcsmConsumerEventHandler&));

        MOCK_METHOD1(setLocalProviderAvailability, bool(bool));
        MOCK_METHOD1(setLocalConsumerAvailability, bool(bool));
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
            provider.reset(new DcsmProviderImpl(compMock));

            // "initialize" event handler for easier testing
            EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce(Return(true));
            provider->dispatchEvents(handler);
        }

        StrictMock<DcsmComponentMock> compMock;
        StrictMock<DcsmProviderEventHandlerMock> handler;
        std::unique_ptr<DcsmProviderImpl> provider;

        SizeInfo size{ 800u, 600u };
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
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce(Invoke([this](auto& h)
        {
            EXPECT_EQ(provider.get(), &h);
            return true;
        }));
        EXPECT_EQ(provider->dispatchEvents(handler), StatusOK);
    }

    TEST_F(ADcsmProvider, canOfferContent)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
    }

    TEST_F(ADcsmProvider, willIgnoreAllButOneOfferContent)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_NE(provider->offerContent(id, Category(5678), sceneId_t(432)), StatusOK);
        EXPECT_NE(provider->offerContent(id, Category(5678), sceneId_t(4321)), StatusOK);
    }

    TEST_F(ADcsmProvider, willAcceptSameSceneIDForDifferentContents)
    {
        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(123), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(compMock, sendOfferContent(ramses_internal::ContentID(124), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(ContentID(124), Category(567), sceneId_t(433)), StatusOK);
    }

    TEST_F(ADcsmProvider, willAutomaticallyReplyContentReadyIfContentIsMarkedReadyAndNotCallCallback)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(ramses_internal::ContentID(123), ramses_internal::ETechnicalContentType::RamsesSceneID, ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
    }

    TEST_F(ADcsmProvider, willCallCallbackIfContentNotMarkedReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(id));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
    }

    TEST_F(ADcsmProvider, willReplyContentReadyOnceContentIsMarkedReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(_));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(ramses_internal::ContentID(123), ramses_internal::ETechnicalContentType::RamsesSceneID, ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, ignoresStateChangeRequestToSameState)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        EXPECT_CALL(handler, contentShow(_, _));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, SizeInfo(), AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, SizeInfo(), AnimationInformation());
        EXPECT_CALL(handler, contentHide(_, _));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsCallbackForEachSizeChangedWithCorrectSizeInfoAndAnimInformation)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(handler, contentSizeChange(id, SizeInfo{133, 337}, AnimationInformation{12345, 67890}));
        provider->contentSizeChange(id, SizeInfo{ 133, 337 }, AnimationInformation{ 12345, 67890 });

        EXPECT_CALL(handler, contentSizeChange(id, SizeInfo{ 555, 900 }, AnimationInformation{ 875, 2 }));
        provider->contentSizeChange(id, SizeInfo{ 555, 900 }, AnimationInformation{ 875, 2 });

        EXPECT_CALL(handler, contentSizeChange(id, SizeInfo{ 1920, 800 }, AnimationInformation{ 91385675902, 8237492095 }));
        provider->contentSizeChange(id, SizeInfo{ 1920, 800 }, AnimationInformation{ 91385675902, 8237492095 });
    }

    TEST_F(ADcsmProvider, callsCallbackForEachStatusChangedWithCorrectAnimInformation)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(_)).WillOnce(Invoke([this](ContentID id_)
        {
            EXPECT_EQ(id, id_);
            EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
            EXPECT_EQ(provider->markContentReady(id_), StatusOK);
        }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation{ 91385675902, 8237492095 }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, SizeInfo(), AnimationInformation{ 91385675902, 8237492095 });
        EXPECT_CALL(handler, contentHide(id, AnimationInformation{ 123, 89576 }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation{ 123, 89576 });
    }

    TEST_F(ADcsmProvider, callsSizeChangedCallbackOnStatusChangedToAssignedFromOffered)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
    }

    TEST_F(ADcsmProvider, doesNotCallSizeChangedCallbackOnStatusChangedToAssignedFromAboveButContentReleasedCallback)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation{ 325, 43290587 }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation{ 325, 43290587 });
    }

    TEST_F(ADcsmProvider, doesNotCallSizeChangedCallbackOnStatusChangedToAnyOtherStateThanAssignedUpwards)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).Times(AtLeast(1)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendContentReady(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(true));
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(AtLeast(1)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentShow(id, AnimationInformation())).Times(AtLeast(1));
        EXPECT_CALL(handler, contentHide(id, AnimationInformation())).Times(AtLeast(1));
        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).Times(AtLeast(1));
        EXPECT_CALL(handler, contentReadyRequested(id)).Times(AtLeast(1));
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation{ 91385675902, 8237492095 })).Times(1);

        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);

        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, size, AnimationInformation());
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, size, AnimationInformation());

        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        provider->contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, size, AnimationInformation{ 91385675902, 8237492095 });
    }

    TEST_F(ADcsmProvider, sendsRequestStopOfferMessageAndCallsStopOfferAcceptedOnStatusChange)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentShow(_, _));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);

        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation{ 91385675902, 8237492095 }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, SizeInfo(), AnimationInformation{ 91385675902, 8237492095 });
    }

    TEST_F(ADcsmProvider, sendsRequestContentFacosMessageToConsumerIfUserCallsRequestContentFocus)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
        provider->markContentReady(id);
        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        EXPECT_CALL(compMock, sendContentFocusRequest(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_EQ(provider->requestContentFocus(id), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnStopOfferContent)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_NE(provider->requestStopOfferContent(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnRequestContentFocus)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
        provider->markContentReady(id);
        EXPECT_CALL(compMock, sendContentFocusRequest(_)).WillOnce(Return(true));
        EXPECT_EQ(provider->requestContentFocus(id), StatusOK);
        EXPECT_NE(provider->requestContentFocus(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnMarkContentReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
        EXPECT_CALL(handler, contentReadyRequested(_));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        EXPECT_NE(provider->markContentReady(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, marksContentReadyOnlyOnce)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
        EXPECT_CALL(handler, contentReadyRequested(_));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_NE(provider->markContentReady(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, callsNoCanvasSizeCallbackIfDcsmSizeMessageHasUnknownContentID)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        provider->contentSizeChange(ContentID(124), SizeInfo{ 555, 900 }, AnimationInformation());
    }

    TEST_F(ADcsmProvider, doesNothingIfContentStateMessageHasWrongContentID)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        provider->contentStateChange(ContentID(124), ramses_internal::EDcsmState::Assigned, SizeInfo(), AnimationInformation());
        provider->contentStateChange(ContentID(124), ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        provider->contentStateChange(ContentID(124), ramses_internal::EDcsmState::Shown, SizeInfo(), AnimationInformation());
        provider->contentStateChange(ContentID(124), ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        provider->contentStateChange(ContentID(124), ramses_internal::EDcsmState::Offered, SizeInfo(), AnimationInformation());
        provider->contentStateChange(ContentID(124), ramses_internal::EDcsmState::AcceptStopOffer, SizeInfo(), AnimationInformation());
    }

    TEST_F(ADcsmProvider, allowsToReofferContentWithSameOrDifferentParameters)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, stopOfferAccepted(_, _));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        provider->contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, stopOfferAccepted(_, _));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        provider->contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(5678), sceneId_t(4321)), StatusOK);
    }

    TEST_F(ADcsmProvider, offerContentFailsIfSendOfferContentFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillOnce(Return(false));
        EXPECT_NE(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
    }

    TEST_F(ADcsmProvider, requestStopOfferContentFailsIfSendRequestUnregisterContentFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).WillOnce(Return(false));
        EXPECT_NE(provider->requestStopOfferContent(id), StatusOK);
    }

    TEST_F(ADcsmProvider, markContentReadyFailsIfsendContentReadyFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentReadyRequested(_));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(false));
        EXPECT_NE(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, requestContentFocusFailsIfSendContentFocusRequestFails)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentReadyRequested(_));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
        EXPECT_CALL(compMock, sendContentFocusRequest(_)).WillOnce(Return(false));
        EXPECT_NE(provider->requestContentFocus(id), StatusOK);
    }

    TEST_F(ADcsmProvider, dispatchesToTheHandlerProvidedByUser)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce(Invoke([this](auto& h)
        {
            EXPECT_CALL(handler, contentSizeChange(_, _, _));
            h.contentSizeChange(id, SizeInfo{ 123, 321 }, AnimationInformation());
            return true;
        }));
        EXPECT_EQ(provider->dispatchEvents(handler), StatusOK);

        DcsmProviderEventHandlerMock handler2;
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce(Invoke([&handler2, this](auto& h)
        {
            EXPECT_CALL(handler2, contentSizeChange(_, _, _));
            h.contentSizeChange(id, SizeInfo{ 1234, 4321 }, AnimationInformation());
            return true;
        }));
        EXPECT_EQ(provider->dispatchEvents(handler2), StatusOK);
    }

    TEST_F(ADcsmProvider, callsShowCallbackIfNewStateIsShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation{ 325, 43290587 }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, size, AnimationInformation{ 325, 43290587 });
    }

    TEST_F(ADcsmProvider, callsHideCallbackIfNewStateIsReadyAndWasShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, size, AnimationInformation());

        EXPECT_CALL(handler, contentHide(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, size, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsReleaseCallbackIfNewStateIsAssignedAndWasShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, size, AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsReleaseCallbackIfNewStateIsAssignedAndWasReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsStopOfferCallbackIfNewStateIsUnregisteredAndWasShown)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentShow(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Shown, size, AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(1).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation{ 325, 43290587 }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, size, AnimationInformation{ 325, 43290587 });
    }

    TEST_F(ADcsmProvider, callsStopOfferCallbackIfNewStateIsUnregisteredAndWasReady)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(1).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, size, AnimationInformation());
    }

    TEST_F(ADcsmProvider, callsStopOfferCallbackIfNewStateIsUnregisteredAndWasAssigned)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendRequestStopOfferContent(_)).Times(1).WillOnce(Return(true));
        EXPECT_EQ(provider->requestStopOfferContent(id), StatusOK);
        EXPECT_CALL(handler, stopOfferAccepted(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::AcceptStopOffer, size, AnimationInformation());
    }

    TEST_F(ADcsmProvider, needsANewMarkReadyAfterContentHasBeenSetToAssigned)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(id));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, needsANewMarkReadyAfterContentHasBeenSetToOffered)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Offered, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(handler, contentReadyRequested(id));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
    }

    TEST_F(ADcsmProvider, allowsUserToCallMarkReadyWithinContentReleaseCallback)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillRepeatedly(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).WillOnce(Invoke([this](auto id_, auto)
        {
            provider->markContentReady(id_);
        }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, SizeInfo(), AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());
    }

    TEST_F(ADcsmProvider, doesNotCallSendContentReadyWhenContentHasBeenMarkedReadyButNotRequestedAfterUnreadying)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).WillOnce(Invoke([this](auto id_, auto)
        {
            provider->markContentReady(id_);
        }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, SizeInfo(), AnimationInformation());
    }


    TEST_F(ADcsmProvider, doesNotCallSendContentReadyWhenContentHasBeenMarkedReadyButNotRequestedAfterReassigning)
    {
        EXPECT_CALL(compMock, sendOfferContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->offerContent(id, Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(id), StatusOK);
        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());

        EXPECT_CALL(compMock, sendContentReady(_, _, _)).WillOnce(Return(true));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Ready, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentRelease(id, AnimationInformation())).WillOnce(Invoke([this](auto id_, auto)
        {
            provider->markContentReady(id_);
        }));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Offered, SizeInfo(), AnimationInformation());

        EXPECT_CALL(handler, contentSizeChange(id, size, AnimationInformation()));
        provider->contentStateChange(id, ramses_internal::EDcsmState::Assigned, size, AnimationInformation());
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
