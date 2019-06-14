//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-framework-api/DcsmProvider.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "DcsmProviderImpl.h"
#include "Components/DcsmComponent.h"
#include "Components/IDcsmProviderEventHandler.h"
#include "DcsmGmockPrinter.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace ramses
{
    using namespace testing;

    class DcsmProviderEventHandlerMock : public IDcsmProviderEventHandler
    {
    public:
        MOCK_METHOD2(contentHidden, void(ContentID, AnimationInformation));
        MOCK_METHOD2(contentShown, void(ContentID, AnimationInformation));
        MOCK_METHOD2(contentUnregistered, void(ContentID, AnimationInformation));
        MOCK_METHOD3(canvasSizeChanged, void(ContentID, SizeInfo, AnimationInformation));
        MOCK_METHOD1(contentReadyRequest, void(ContentID));
    };

    class DcsmComponentMock : public ramses_internal::IDcsmComponent
    {
    public:
        MOCK_METHOD1(dispatchProviderEvents, bool(ramses_internal::IDcsmProviderEventHandler&));
        MOCK_METHOD2(sendRegisterContent, bool(ramses_internal::ContentID, ramses_internal::Category));
        MOCK_METHOD4(sendContentAvailable, bool(const ramses_internal::Guid&, ramses_internal::ContentID, ramses_internal::ETechnicalContentType, ramses_internal::TechnicalContentDescriptor));
        MOCK_METHOD2(sendCategoryContentSwitchRequest, bool(const ramses_internal::Guid&, ramses_internal::ContentID));
        MOCK_METHOD1(sendRequestUnregisterContent, bool(ramses_internal::ContentID));

        MOCK_METHOD3(sendCanvasSizeChange, bool(ramses_internal::ContentID, ramses_internal::SizeInfo, ramses_internal::AnimationInformation));
        MOCK_METHOD3(sendContentStatusChange, bool(ramses_internal::ContentID, ramses_internal::EDcsmStatus, ramses_internal::AnimationInformation));
        MOCK_METHOD1(dispatchConsumerEvents, bool(ramses::IDcsmConsumerEventHandler&));

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

        ramses_internal::Guid partId{true};
        ramses_internal::Guid wrongPartId{true};
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

    TEST_F(ADcsmProvider, canRegisterContent)
    {
        EXPECT_CALL(compMock, sendRegisterContent(ramses_internal::ContentID(123), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
    }

    TEST_F(ADcsmProvider, willIgnoreAllButOneRegisterContent)
    {
        EXPECT_CALL(compMock, sendRegisterContent(ramses_internal::ContentID(123), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_NE(provider->registerRamsesContent(ContentID(123), Category(5678), sceneId_t(432)), StatusOK);
        EXPECT_NE(provider->registerRamsesContent(ContentID(123), Category(5678), sceneId_t(4321)), StatusOK);
    }

    TEST_F(ADcsmProvider, willAcceptSameSceneIDForDifferentContents)
    {
        EXPECT_CALL(compMock, sendRegisterContent(ramses_internal::ContentID(123), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(compMock, sendRegisterContent(ramses_internal::ContentID(124), ramses_internal::Category(567))).WillOnce(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(124), Category(567), sceneId_t(433)), StatusOK);
    }

    TEST_F(ADcsmProvider, willAutomaticallyReplyContentAvailableIfContentIsMarkedReadyAndNotCallCallback)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_EQ(provider->markContentReady(ContentID(123)), StatusOK);

        EXPECT_CALL(compMock, sendContentAvailable(partId, ramses_internal::ContentID(123), ramses_internal::ETechnicalContentType::RamsesSceneID, ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, ramses::AnimationInformation(), partId);
    }

    TEST_F(ADcsmProvider, willCallCallbackIfContentNotMarkedReady)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(handler, contentReadyRequest(ContentID(123)));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, ramses::AnimationInformation(), partId);
    }

    TEST_F(ADcsmProvider, willReplyContentAvailableOnceContentIsMarkedReady)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(handler, contentReadyRequest(_));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, ramses::AnimationInformation(), partId);

        EXPECT_CALL(compMock, sendContentAvailable(partId, ramses_internal::ContentID(123), ramses_internal::ETechnicalContentType::RamsesSceneID, ramses_internal::TechnicalContentDescriptor(432))).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(ContentID(123)), StatusOK);
    }

    TEST_F(ADcsmProvider, ignoresStateChangeRequestToSameState)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
        EXPECT_EQ(provider->markContentReady(ContentID(123)), StatusOK);
        EXPECT_CALL(compMock, sendContentAvailable(_, _, _, _)).WillOnce(Return(true));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        EXPECT_CALL(handler, contentShown(_, _));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Shown, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Shown, AnimationInformation(), partId);
        EXPECT_CALL(handler, contentHidden(_, _));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
    }

    TEST_F(ADcsmProvider, callsCallbackForEachSizeChangedWithCorrectSizeInfoAndAnimInformation)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(handler, canvasSizeChanged(ContentID(123), SizeInfo{133, 337}, AnimationInformation{12345, 67890}));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 133, 337 }, AnimationInformation{ 12345, 67890 }, partId);

        EXPECT_CALL(handler, canvasSizeChanged(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation{ 875, 2 }));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation{ 875, 2 }, partId);

        EXPECT_CALL(handler, canvasSizeChanged(ContentID(123), SizeInfo{ 1920, 800 }, AnimationInformation{ 91385675902, 8237492095 }));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 1920, 800 }, AnimationInformation{ 91385675902, 8237492095 }, partId);
    }

    TEST_F(ADcsmProvider, doesNotCallSizeCallbackForWrongConsumer)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(handler, canvasSizeChanged(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation()));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 1920, 800 }, AnimationInformation{ 91385675902, 8237492095 }, wrongPartId);

        EXPECT_CALL(handler, canvasSizeChanged(ContentID(123), SizeInfo{ 555, 800 }, AnimationInformation()));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 800 }, AnimationInformation(), partId);
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 800 }, AnimationInformation{ 91385675902, 8237492095 }, wrongPartId);
    }

    TEST_F(ADcsmProvider, callsCallbackForEachStatusChangedWithCorrectAnimInformation)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);

        EXPECT_CALL(handler, contentReadyRequest(_)).WillOnce(Invoke([this](ContentID id)
        {
            EXPECT_EQ(id.getValue(), 123);
            EXPECT_CALL(compMock, sendContentAvailable(_, _, _, _)).WillRepeatedly(Return(true));
            EXPECT_EQ(provider->markContentReady(id), StatusOK);
        }));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);

        EXPECT_CALL(handler, contentShown(ContentID(123), AnimationInformation{ 91385675902, 8237492095 }));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Shown, AnimationInformation{ 91385675902, 8237492095 }, partId);
        EXPECT_CALL(handler, contentHidden(ContentID(123), AnimationInformation{ 123, 89576 }));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation{ 123, 89576 }, partId);
    }

    TEST_F(ADcsmProvider, doesNotCallStatusCallbackForWrongConsumer)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
        EXPECT_EQ(provider->markContentReady(ContentID(123)), StatusOK);

        EXPECT_CALL(compMock, sendContentAvailable(_, _, _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentShown(ContentID(123), AnimationInformation{}));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Shown, AnimationInformation(), wrongPartId); // ignored because wrong consumer
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId); // ignored because last wasn't valid, so no state change
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Shown, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), wrongPartId); // ignored because wrong consumer
    }

    TEST_F(ADcsmProvider, sendsRequestUnregisterMessageAndCallsUnregisteredOnStatusChange)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);

        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);

        EXPECT_EQ(provider->markContentReady(ContentID(123)), StatusOK);
        EXPECT_CALL(compMock, sendContentAvailable(_, _, _, _)).WillRepeatedly(Return(true));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);

        EXPECT_CALL(handler, contentShown(_, _));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Shown, AnimationInformation(), partId);

        EXPECT_CALL(compMock, sendRequestUnregisterContent(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_EQ(provider->unregisterRamsesContent(ContentID(123)), StatusOK);

        EXPECT_CALL(handler, contentUnregistered(ContentID(123), AnimationInformation{ 91385675902, 8237492095 }));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Unregistered, AnimationInformation{ 91385675902, 8237492095 }, partId);
    }

    TEST_F(ADcsmProvider, callsUnregisterCallbackWithoutAnyConsumerSubscribed)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, sendRequestUnregisterContent(ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_CALL(handler, contentUnregistered(ContentID(123), AnimationInformation()));
        EXPECT_EQ(provider->unregisterRamsesContent(ContentID(123)), StatusOK);
    }

    TEST_F(ADcsmProvider, sendsRequestContentShownMessageToConsumerIfUserCallsRequestContentShown)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
        provider->markContentReady(ContentID(123));
        EXPECT_CALL(compMock, sendContentAvailable(_, _, _, _)).WillRepeatedly(Return(true));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        EXPECT_CALL(compMock, sendCategoryContentSwitchRequest(partId, ramses_internal::ContentID(123))).WillOnce(Return(true));
        EXPECT_EQ(provider->requestContentShown(ContentID(123)), StatusOK);
    }

    TEST_F(ADcsmProvider, sendsNoRequestContentShownMessageIfThereIsNoConsumer)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_NE(provider->requestContentShown(ContentID(123)), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnUnregisterContent)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_NE(provider->unregisterRamsesContent(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnContentSwitchRequest)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
        provider->markContentReady(ContentID(123));
        EXPECT_CALL(compMock, sendCategoryContentSwitchRequest(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->requestContentShown(ContentID(123)), StatusOK);
        EXPECT_NE(provider->requestContentShown(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, doesNothingIfUserUsesWrongContentIDOnMarkContentReady)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
        EXPECT_CALL(handler, contentReadyRequest(_));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        EXPECT_NE(provider->markContentReady(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, marksContentReadyOnlyOnce)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
        provider->canvasSizeChange(ContentID(123), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
        EXPECT_CALL(handler, contentReadyRequest(_));
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        EXPECT_CALL(compMock, sendContentAvailable(_, _, _, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->markContentReady(ContentID(123)), StatusOK);
        EXPECT_NE(provider->markContentReady(ContentID(1234)), StatusOK);
    }

    TEST_F(ADcsmProvider, callsNoCanvasSizeCallbackIfDcsmSizeMessageHasUnknownContentID)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        provider->canvasSizeChange(ContentID(124), SizeInfo{ 555, 900 }, AnimationInformation(), partId);
    }

    TEST_F(ADcsmProvider, doesNothingIfEDcsmStatusMessageHasWrongContentID)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        provider->contentStatusChange(ContentID(124), EDcsmStatus::Ready, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(124), EDcsmStatus::Shown, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(124), EDcsmStatus::Ready, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(124), EDcsmStatus::Registered, AnimationInformation(), partId);
        provider->contentStatusChange(ContentID(124), EDcsmStatus::Unregistered, AnimationInformation(), partId);
    }

    TEST_F(ADcsmProvider, allowsToReregisterContentWithSameOrDifferentParameters)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, sendRequestUnregisterContent(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentUnregistered(_, _));
        EXPECT_EQ(provider->unregisterRamsesContent(ContentID(123)), StatusOK);

        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, sendRequestUnregisterContent(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentUnregistered(_, _));
        EXPECT_EQ(provider->unregisterRamsesContent(ContentID(123)), StatusOK);

        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(5678), sceneId_t(4321)), StatusOK);
    }

    TEST_F(ADcsmProvider, registerRamsesContentFailsIfSendRegisterContentFails)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillOnce(Return(false));
        EXPECT_NE(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
    }

    TEST_F(ADcsmProvider, requestUnregisterContentFailsIfSendRequestUnregisterContentFails)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, sendRequestUnregisterContent(_)).WillOnce(Return(false));
        EXPECT_NE(provider->unregisterRamsesContent(ContentID(123)), StatusOK);
    }

    TEST_F(ADcsmProvider, markContentReadyFailsIfSendContentAvailableFails)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentReadyRequest(_));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        EXPECT_CALL(compMock, sendContentAvailable(_, _, _, _)).WillOnce(Return(false));
        EXPECT_NE(provider->markContentReady(ContentID(123)), StatusOK);
    }

    TEST_F(ADcsmProvider, requestContentShownFailsIfSendCategoryContentSwitchRequestFails)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(handler, contentReadyRequest(_));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        provider->contentStatusChange(ContentID(123), EDcsmStatus::Ready, AnimationInformation(), partId);
        EXPECT_CALL(compMock, sendCategoryContentSwitchRequest(_, _)).WillOnce(Return(false));
        EXPECT_NE(provider->requestContentShown(ContentID(123)), StatusOK);
    }

    TEST_F(ADcsmProvider, dispatchesToTheHandlerProvidedByUser)
    {
        EXPECT_CALL(compMock, sendRegisterContent(_, _)).WillOnce(Return(true));
        EXPECT_EQ(provider->registerRamsesContent(ContentID(123), Category(567), sceneId_t(432)), StatusOK);
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce(Invoke([this](auto& h)
        {
            EXPECT_CALL(handler, canvasSizeChanged(_, _, _));
            h.canvasSizeChange(ContentID(123), SizeInfo{ 123, 321 }, AnimationInformation(), partId);
            return true;
        }));
        EXPECT_EQ(provider->dispatchEvents(handler), StatusOK);

        DcsmProviderEventHandlerMock handler2;
        EXPECT_CALL(compMock, dispatchProviderEvents(_)).WillOnce(Invoke([&handler2, this](auto& h)
        {
            EXPECT_CALL(handler2, canvasSizeChanged(_, _, _));
            h.canvasSizeChange(ContentID(123), SizeInfo{ 1234, 4321 }, AnimationInformation(), partId);
            return true;
        }));
        EXPECT_EQ(provider->dispatchEvents(handler2), StatusOK);
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
