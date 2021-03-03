//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "DcsmContentControlImpl.h"
#include "ramses-renderer-api/IDcsmContentControlEventHandler.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "RendererSceneControlImpl.h"
#include "DcsmMetadataUpdateImpl.h"
#include "DcsmConsumerMock.h"
#include <memory.h>
#include <array>
#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-renderer-api/RamsesRenderer.h"

using namespace ramses;
using namespace testing;

class DcsmContentControlEventHandlerMock : public IDcsmContentControlEventHandler
{
public:
    MOCK_METHOD(void, contentAvailable, (ContentID, Category), (override));
    MOCK_METHOD(void, contentReady, (ContentID, DcsmContentControlEventResult), (override));
    MOCK_METHOD(void, contentShown, (ContentID), (override));
    MOCK_METHOD(void, contentStopOfferRequested, (ContentID), (override));
    MOCK_METHOD(void, contentNotAvailable, (ContentID), (override));
    MOCK_METHOD(void, contentMetadataUpdated, (ContentID, const DcsmMetadataUpdate&), (override));
    MOCK_METHOD(void, offscreenBufferLinked, (displayBufferId_t, ContentID, dataConsumerId_t, bool), (override));
    MOCK_METHOD(void, contentLinkedToTextureConsumer, (ContentID, ContentID, dataConsumerId_t, bool), (override));
    MOCK_METHOD(void, dataLinked, (ContentID, dataProviderId_t, ContentID, dataConsumerId_t, bool), (override));
    MOCK_METHOD(void, dataUnlinked, (ContentID, dataConsumerId_t, bool), (override));
    MOCK_METHOD(void, objectsPicked, (ContentID, const pickableObjectId_t*, uint32_t), (override));
    MOCK_METHOD(void, dataProviderCreated, (ContentID, dataProviderId_t), (override));
    MOCK_METHOD(void, dataProviderDestroyed, (ContentID, dataProviderId_t), (override));
    MOCK_METHOD(void, dataConsumerCreated, (ContentID, dataConsumerId_t), (override));
    MOCK_METHOD(void, dataConsumerDestroyed, (ContentID, dataConsumerId_t), (override));
    MOCK_METHOD(void, contentFlushed, (ContentID, sceneVersionTag_t), (override));
    MOCK_METHOD(void, contentExpirationMonitoringEnabled, (ContentID), (override));
    MOCK_METHOD(void, contentExpirationMonitoringDisabled, (ContentID), (override));
    MOCK_METHOD(void, contentExpired, (ContentID), (override));
    MOCK_METHOD(void, contentRecoveredFromExpiration, (ContentID), (override));
    MOCK_METHOD(void, streamAvailabilityChanged, (waylandIviSurfaceId_t, bool), (override));
    MOCK_METHOD(void, contentEnableFocusRequest, (ContentID, int32_t), (override));
    MOCK_METHOD(void, contentDisableFocusRequest, (ContentID, int32_t), (override));
};

class RendererSceneControlMock : public IRendererSceneControl
{
public:
    MOCK_METHOD(status_t, setSceneState, (sceneId_t, RendererSceneState), (override));
    MOCK_METHOD(status_t, setSceneMapping, (sceneId_t, displayId_t), (override));
    MOCK_METHOD(status_t, setSceneDisplayBufferAssignment, (sceneId_t, displayBufferId_t, int32_t), (override));
    MOCK_METHOD(status_t, linkOffscreenBuffer, (displayBufferId_t, sceneId_t, dataConsumerId_t), (override));
    MOCK_METHOD(status_t, linkStreamBuffer, (streamBufferId_t, sceneId_t, dataConsumerId_t), (override));
    MOCK_METHOD(status_t, linkData, (sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t), (override));
    MOCK_METHOD(status_t, unlinkData, (sceneId_t, dataConsumerId_t), (override));
    MOCK_METHOD(status_t, handlePickEvent, (sceneId_t, float, float), (override));
    MOCK_METHOD(status_t, flush, (), (override));
    MOCK_METHOD(status_t, dispatchEvents, (IRendererSceneControlEventHandler&), (override));
    MOCK_METHOD(status_t, dispatchSpecialEvents, (IRendererSceneControlEventHandler_SpecialForWayland& eventHandler), (override));
    MOCK_METHOD(streamBufferId_t, createStreamBuffer,(displayId_t display, waylandIviSurfaceId_t source), (override));
    MOCK_METHOD(status_t, destroyStreamBuffer,(displayId_t display, streamBufferId_t streamBuffer), (override));
    MOCK_METHOD(status_t, setStreamBufferState, (displayId_t display, streamBufferId_t streamBufferId, bool state), (override));
};

class ADcsmContentControl : public Test
{
public:
    ADcsmContentControl()
        : m_dcsmContentControl(m_dcsmConsumerMock, m_sceneControlMock)
        , m_dcsmHandler(m_dcsmContentControl)
        , m_sceneControlHandler(m_dcsmContentControl)
        , m_sceneControlHandlerWayland(m_dcsmContentControl)
    {
        m_dcsmContentControl.addContentCategory(m_categoryID1, m_displayId, m_CategoryInfoUpdate1);
        m_dcsmContentControl.addContentCategory(m_categoryID2, m_displayId, m_CategoryInfoUpdate2);
        EXPECT_CALL(m_sceneControlMock, dispatchSpecialEvents(_)).Times(AnyNumber());
    }

    virtual void TearDown() override
    {
        // make sure no pending events
        update(std::numeric_limits<uint64_t>::max());
    }

    void update(uint64_t timeStamp = 0)
    {
        EXPECT_CALL(m_dcsmConsumerMock, dispatchEvents(_));
        EXPECT_CALL(m_sceneControlMock, dispatchEvents(_));
        EXPECT_CALL(m_sceneControlMock, dispatchSpecialEvents(_));
        EXPECT_CALL(m_sceneControlMock, flush());
        EXPECT_EQ(StatusOK, m_dcsmContentControl.update(timeStamp, m_eventHandlerMock));
        m_lastUpdateTS = timeStamp;
    }

    void makeDcsmContentReady_ramses(ContentID contentID, Category categoryID, sceneId_t sceneId = SceneId1, bool alreadyAssignedToOtherContentAndRequestedReady = false)
    {
        // offered
        EXPECT_CALL(m_eventHandlerMock, contentAvailable(contentID, categoryID));
        EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(contentID,_)).WillOnce([&](const auto&, const auto& infoupdate){
            if (categoryID==m_categoryID1)
                EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
            else
                EXPECT_EQ(m_CategoryInfoUpdate2, infoupdate);
            return StatusOK;
        });
        m_dcsmHandler.contentOffered(contentID, categoryID, ETechnicalContentType::RamsesSceneID);
        update(m_lastUpdateTS);

        // provider sends content description right after assign
        m_dcsmHandler.contentDescription(contentID, TechnicalContentDescriptor{ sceneId.getValue() });
        update(m_lastUpdateTS);
        if (!alreadyAssignedToOtherContentAndRequestedReady)
            m_sceneControlHandler.sceneStateChanged(sceneId, RendererSceneState::Available);

        // request ready
        EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(contentID, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
        if (!alreadyAssignedToOtherContentAndRequestedReady)
        {
            EXPECT_CALL(m_sceneControlMock, setSceneMapping(sceneId, m_displayId));
            EXPECT_CALL(m_sceneControlMock, setSceneState(sceneId, RendererSceneState::Ready));
        }
        EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(contentID, 0));
        update(m_lastUpdateTS);

        m_dcsmHandler.contentReady(contentID);
        update(m_lastUpdateTS);
    }

protected:
    static constexpr sceneId_t SceneId1{ 33 };
    static constexpr sceneId_t SceneId2{ 34 };

    const Category m_categoryID1{ 6 };
    const Category m_categoryID2{ 7 };
    const Category m_categoryAddedLater{ 8 };
    const Category m_categoryUninteresting{ 9 };
    const ContentID m_contentID1{ 321 };
    const ContentID m_contentID2{ 322 };
    const ContentID m_contentID3{ 323 };
    const ContentID m_contentID4{ 324 };
    const displayId_t m_displayId{ 0 };
    CategoryInfoUpdate m_CategoryInfoUpdate1{{ 16, 16 }, { 2, 3, 16, 16 }, {1, 3, 6, 8}};
    CategoryInfoUpdate m_CategoryInfoUpdate2{{ 32, 32 }, { 1, 6, 32, 32 }, {9, 2, 4, 7}};
    CategoryInfoUpdate m_CategoryInfoUpdate3{{ 64, 64 }, { 4, 7, 64, 64 }, {5, 3, 2, 4}};

    StrictMock<DcsmConsumerMock> m_dcsmConsumerMock;
    StrictMock<RendererSceneControlMock> m_sceneControlMock;
    DcsmContentControlImpl m_dcsmContentControl;
    IDcsmConsumerEventHandler& m_dcsmHandler;
    IRendererSceneControlEventHandler& m_sceneControlHandler;
    IRendererSceneControlEventHandler_SpecialForWayland& m_sceneControlHandlerWayland;
    StrictMock<DcsmContentControlEventHandlerMock> m_eventHandlerMock;
    uint64_t m_lastUpdateTS = 0;
};

class ADcsmContentControlP : public ADcsmContentControl, public testing::WithParamInterface<ramses::ETechnicalContentType>
{
    public:
        ADcsmContentControlP()
        {
        }

        void makeDcsmContentReady(ContentID contentID, Category categoryID, TechnicalContentDescriptor techId, bool alreadyAssignedToOtherContentAndRequestedReady = false)
        {
            if (GetParam()==ETechnicalContentType::RamsesSceneID)
            {
                const sceneId_t sceneId { techId.getValue()};
                makeDcsmContentReady_ramses(contentID, categoryID, sceneId, alreadyAssignedToOtherContentAndRequestedReady);
            }
        }

        void signalTechnicalContentReady()
        {
            if (GetParam()==ETechnicalContentType::RamsesSceneID)
            {
                m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
            }
        }

        void expectRequestToShowTechnicalContent()
        {
            if (GetParam()==ETechnicalContentType::RamsesSceneID)
            {
                EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
            }
        }

        void signalTechnicalContentShown()
        {
            if (GetParam()==ETechnicalContentType::RamsesSceneID)
            {
                m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
            }
        }
};

INSTANTIATE_TEST_SUITE_P(ContentTypeSpecific,
    ADcsmContentControlP,
    testing::Values(ETechnicalContentType::RamsesSceneID));

constexpr sceneId_t ADcsmContentControl::SceneId1;
constexpr sceneId_t ADcsmContentControl::SceneId2;

TEST(DcsmContentControl, failsToAddCategoryWithoutSettingRenderSizeAndCategoryRect)
{
    ramses::RamsesFramework framework;
    ramses::RamsesRenderer* renderer = framework.createRenderer({});
    ramses::DcsmContentControl* dcc = renderer->createDcsmContentControl();
    EXPECT_NE(StatusOK, dcc->addContentCategory(ramses::Category(1), ramses::displayId_t(0), {}));

    CategoryInfoUpdate categoryInfo1{};
    categoryInfo1.setRenderSize({ 200, 200 });
    EXPECT_NE(StatusOK, dcc->addContentCategory(ramses::Category(1), ramses::displayId_t(0), categoryInfo1));

    CategoryInfoUpdate categoryInfo2{};
    categoryInfo2.setCategoryRect({ 0, 0, 100, 100 });
    EXPECT_NE(StatusOK, dcc->addContentCategory(ramses::Category(1), ramses::displayId_t(0), categoryInfo2));
}

TEST(DcsmContentControl, failsToAddCategoryThatHasAlreadyBeenAdded)
{
    ramses::RamsesFramework framework;
    ramses::RamsesRenderer* renderer = framework.createRenderer({});
    ramses::DcsmContentControl* dcc = renderer->createDcsmContentControl();
    const CategoryInfoUpdate categoryInfo{{200, 200}, {0, 0, 100, 100}};
    const Category category(1);
    EXPECT_EQ(StatusOK, dcc->addContentCategory(category, ramses::displayId_t(0), categoryInfo));
    EXPECT_NE(StatusOK, dcc->addContentCategory(category, ramses::displayId_t(0), categoryInfo));
}

TEST(DcsmContentControl, canAddAndRemoveCategories)
{
    ramses::RamsesFramework framework;
    ramses::RamsesRenderer* renderer = framework.createRenderer({});
    ramses::DcsmContentControl* dcc = renderer->createDcsmContentControl();
    const CategoryInfoUpdate categoryInfo{{200, 200}, {0, 0, 100, 100}};
    EXPECT_EQ(StatusOK, dcc->addContentCategory(ramses::Category(1), ramses::displayId_t(0), categoryInfo));
    EXPECT_EQ(StatusOK, dcc->removeContentCategory(ramses::Category(1)));
}

TEST_P(ADcsmContentControlP, handlesDcsmOffered)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, GetParam());
    update();
}

TEST_F(ADcsmContentControl, handleRequestContentReadyFails)
{
    // offer
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();
    // stop offer
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
    // let changing state on dcsmConsumer fail
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, _, _)).WillRepeatedly(Return(2));
    // causes requestContentReady to return error
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 500));
    update();
    // no further events even when time progresses
    update(1000);
    update(1010);
    update(1020);
    update(1020);
    update(1020);
    update(1020);
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);
    Mock::VerifyAndClearExpectations(&m_eventHandlerMock);

    // can get content to ready if offered again
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(1050);
    update(1070);

}

TEST_F(ADcsmContentControl, stopOfferRequestWhileWaitingForReadyWithTimeout)
{
    // offer
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();

    // requestReady with timeout
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, _));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 500));
    update(200);

    // stop offer
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update(300);

    // accept stopoffer
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, _));
    m_dcsmContentControl.acceptStopOffer(m_contentID1, {});

    // no further events even when time progresses
    update(1000);
    update(1010);
    update(1020);
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);
    Mock::VerifyAndClearExpectations(&m_eventHandlerMock);

    // can get content to ready if offered again
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(1050);
    update(1070);
}

TEST_F(ADcsmContentControl, contentDoesNotBecomeAvailableIfAssigningFails)
{
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1)).Times(0);
    // let assignContent fail
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto&) {
        return 1;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
}

TEST_P(ADcsmContentControlP, handlesDcsmContentDescription)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));

    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, GetParam());

    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });

    update();
}

TEST_F(ADcsmContentControl, doesNotAssignWhenDcsmOfferedContentForUnregisteredCategory)
{
    m_dcsmHandler.contentOffered(m_contentID1, Category{ 666 }, ETechnicalContentType::RamsesSceneID);
    update();
}

TEST_F(ADcsmContentControl, handlesDcsmOfferedForMultipleContentsAndCategories)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    // content3 uses another category
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate2, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID2));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2, ETechnicalContentType::RamsesSceneID);
    update();

    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_dcsmHandler.contentDescription(m_contentID2, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_dcsmHandler.contentDescription(m_contentID3, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();
}

TEST_P(ADcsmContentControlP, categoryAddedLater)
{
    m_dcsmHandler.contentOffered(m_contentID4, m_categoryUninteresting, GetParam());
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryAddedLater));
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();
}

TEST_P(ADcsmContentControlP, categoryAddedLater_OfferAfterThat)
{
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryAddedLater));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);
}

TEST_F(ADcsmContentControl, categoryAddedLater_canBeReadified)
{
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, ETechnicalContentType::RamsesSceneID);
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryAddedLater));
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();

    m_dcsmHandler.contentDescription(m_contentID3, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID3, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(_, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(_, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID3, 0));
    update();
}

TEST_P(ADcsmContentControlP, categoryAddedLater_MultipleOffersExistingBefore)
{
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    m_dcsmHandler.contentOffered(m_contentID4, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID4, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryAddedLater));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID4, m_categoryAddedLater));
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();
}

TEST_P(ADcsmContentControlP, categoryAddedLater_reAddingCategoryGivesEventsAgain)
{
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryAddedLater));
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();

    m_dcsmContentControl.removeContentCategory(m_categoryAddedLater);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryAddedLater));
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();
}

TEST_P(ADcsmContentControlP, categoryAddedLater_NoEventIfCategoryRemovedBeforeOffer)
{
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();

    m_dcsmContentControl.removeContentCategory(m_categoryAddedLater);
    update();

    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);
}

TEST_P(ADcsmContentControlP, removeCategory_withContentsAssigned)
{
    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();

    // offer some contents
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate3, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryAddedLater));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    m_dcsmContentControl.removeContentCategory(m_categoryAddedLater);
    update();
}

TEST_P(ADcsmContentControlP, categoryAddedLater_NoEventIfUnofferedBeforeAddingCategory)
{
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    m_dcsmHandler.contentStopOfferRequest(m_contentID3);
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();
}

TEST_P(ADcsmContentControlP, categoryAddedLater_NoEventIfForceStoppedBeforeAddingCategory)
{
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryAddedLater, GetParam());
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    m_dcsmHandler.forceContentOfferStopped(m_contentID3);
    update();
    Mock::VerifyAndClearExpectations(&m_dcsmConsumerMock);

    m_dcsmContentControl.addContentCategory(m_categoryAddedLater, m_displayId, m_CategoryInfoUpdate3);
    update();
}

TEST_F(ADcsmContentControl, requestsContentReady)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();

    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();
}

TEST_F(ADcsmContentControl, canAlreadyRequestReadyBeforeDescriptionComes)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

    // needs to wait for description message
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });

    // request ready
    update();
}

TEST_P(ADcsmContentControlP, handlesContentAndSceneReady)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    signalTechnicalContentReady();
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
}

TEST_P(ADcsmContentControlP, handlesContentAndConditionsReadyForMultipleContentsSharingSingleScene)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    makeDcsmContentReady(m_contentID2, m_categoryID1, TechnicalContentDescriptor{ SceneId1.getValue() }, true);

    // handle scene ready
    signalTechnicalContentReady();
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();

    // now request content3 ready
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID3, DcsmContentControlEventResult::OK));
    makeDcsmContentReady(m_contentID3, m_categoryID2, TechnicalContentDescriptor{ SceneId1.getValue() }, true);
    update();
}

TEST_F(ADcsmContentControl, failsUpdateIfTimeNotContinuous)
{
    EXPECT_CALL(m_dcsmConsumerMock, dispatchEvents(_)).Times(4);
    EXPECT_CALL(m_sceneControlMock, dispatchEvents(_)).Times(4);
    EXPECT_CALL(m_sceneControlMock, flush()).Times(4);

    EXPECT_EQ(StatusOK, m_dcsmContentControl.update(0, m_eventHandlerMock));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.update(10, m_eventHandlerMock));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.update(20, m_eventHandlerMock));
    EXPECT_NE(StatusOK, m_dcsmContentControl.update(19, m_eventHandlerMock));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.update(100, m_eventHandlerMock));
    EXPECT_NE(StatusOK, m_dcsmContentControl.update(99, m_eventHandlerMock));
}

TEST_P(ADcsmContentControlP, canGetContentRenderedAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1, TechnicalContentDescriptor{ SceneId1.getValue() });

    // handle scene ready
    signalTechnicalContentReady();
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, timing));

    // start time not reached -> scene not shown -> combined state still ready
    update(10);

    // scene show triggered but not confirmed yet
    expectRequestToShowTechnicalContent();
    update(20);
    update(30);

    // now report scene rendered
    signalTechnicalContentShown();
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(40);

    // stays rendered...
    update(50);
    update(60);
}

TEST_F(ADcsmContentControl, canGetContentRenderedAndThenHiddenAtGivenTime)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, timing));

    // scene shown
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update(20);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(20);

    // announce hide at another time
    const AnimationInformation timing2{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.hideContent(m_contentID1, timing2));

    // content ready event is emitted immediately, but scene is still shown
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(30);
    update(100);
    update(120);

    // at finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    update(150);

    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(200);
}

TEST_F(ADcsmContentControl, hideContentAtGivenTimeAndThenOverrideWithEarlierTiming)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // announce show at a time
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, { 0, 0 }));

    // scene shown
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update(0);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(0);

    // announce hide at a time
    const AnimationInformation timing{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.hideContent(m_contentID1, timing));

    // content ready event is emitted immediately, but scene is still shown
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(110);

    // now request hiding at earlier point
    const AnimationInformation timing2{ 100, 120 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.hideContent(m_contentID1, timing2));

    // still no change
    update(115);

    // at updated finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    update(120);

    // only after scene really hidden content state changes
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(121);
}

TEST_F(ADcsmContentControl, hideContentAtGivenTimeAndThenOverrideWithLaterTiming)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // announce show at a time
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, { 0, 0 }));

    // scene shown
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update(0);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(0);

    // announce hide at a time
    const AnimationInformation timing{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.hideContent(m_contentID1, timing));

    // content ready event is emitted immediately, but scene is still shown
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(110);
    // still rendering to allow fade out animation

    // now request hiding at later point
    const AnimationInformation timing2{ 100, 180 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.hideContent(m_contentID1, timing2));

    // still no change
    update(115);
    update(160);

    // at updated finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    update(180);

    // only after scene really hidden content state changes
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(181);
}

TEST_F(ADcsmContentControl, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, timing));

    // content state unchanged till scene really shown
    update(10);

    // scene shown
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update(20);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(20);

    //////////////////////////
    // new content offered, initiate content switch
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update(30);
    // announce new content's description
    const sceneId_t otherScene{ SceneId1.getValue() + 10 };
    m_dcsmHandler.contentDescription(m_contentID2, TechnicalContentDescriptor{ otherScene.getValue() });
    update(30);
    m_sceneControlHandler.sceneStateChanged(otherScene, RendererSceneState::Available);
    update(30);

    // request new content ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(otherScene, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(otherScene, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID2, 0));
    update(30);

    // announce original content not needed at 1000 and start provider transition animation (if any) immediately
    // scene is still shown within this period, but content reports as assigned
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 1000 }));

    update(50);

    // bit later new content should become ready
    m_dcsmHandler.contentReady(m_contentID2);
    update(100);
    // new content is not combined ready because its scene is not ready
    // handle new scene ready
    m_sceneControlHandler.sceneStateChanged(otherScene, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update(100);

    // announce show at 1000, starting provider side transition (if any) immediately
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Shown, AnimationInformation{ 0, 1000 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(otherScene, RendererSceneState::Rendered));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID2, AnimationInformation{ 0, 1000 }));
    update(110);
    // simulate scene shown
    m_sceneControlHandler.sceneStateChanged(otherScene, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID2));
    update(120);

    ///////

    // now reaching time 1000 original content's scene can be safely hidden - desired state was offered
    // content states already reached their final states
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    update(1000);

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(1100);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
}

TEST_F(ADcsmContentControl, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime_sharingScene)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, timing));

    // scene shown
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update(20);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(20);

    //////////////////////////
    // new content offered, initiate content switch
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update(30);

    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID2, TechnicalContentDescriptor{ SceneId1.getValue() });
    update(30);

    // first request new content ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID2, 0));
    update(30);

    // announce original content not needed at 1000 and start provider transition animation (if any) immediately
    // scene is still shown within this period
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 1000 }));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));

    // bit later new content should become ready
    m_dcsmHandler.contentReady(m_contentID2);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update(100);
    // original content state still rendered
    // new content is fully ready because scene is already mapped (used by original content)

    // announce show at 1000, starting provider side transition (if any) immediately
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Shown, AnimationInformation{ 200, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID2, AnimationInformation{ 200, 1000 }));

    update(110);

    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID2));
    update(210);
    // as scene is already shown (used by original content) new content is in rendered state right away

    ///////

    // actual scene state was being kept rendered and scheduled state change comes into effect, but no content change here
    update(1000);
    update(1100);
}

TEST_F(ADcsmContentControl, forcedStopOfferIsPropagatedAsEventAndMakesContentInvalid)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // content invalidated, cannot set state
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmContentControl, forcedStopOfferedContentCanBecomeOfferedAgain)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Unavailable);

    // offer again and start new cycle
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
}

TEST_F(ADcsmContentControl, forcedStopOfferForUnknownContentIsIgnored)
{
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    update();
}

TEST_F(ADcsmContentControl, propagatesContentEnableFocusRequestAsEvent)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.contentEnableFocusRequest(m_contentID1, 16);
    EXPECT_CALL(m_eventHandlerMock, contentEnableFocusRequest(m_contentID1, 16));
    update();

    m_dcsmHandler.contentDisableFocusRequest(m_contentID1, 16);
    EXPECT_CALL(m_eventHandlerMock, contentDisableFocusRequest(m_contentID1, 16));
    update();

}

TEST_F(ADcsmContentControl, enableAndDisableFocusRequestForUnknownContentIsIgnored)
{
    m_dcsmHandler.contentEnableFocusRequest(m_contentID1, 17);
    update();

    m_dcsmHandler.contentDisableFocusRequest(m_contentID1, 17);
    update();
}

TEST_F(ADcsmContentControl, stopOfferRequestForUnknownContentIsNotAutomaticallyAcceptedRightAway)
{
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 })).Times(0);
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
}

TEST_F(ADcsmContentControl, stopOfferRequestCanBeAccepted)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    update(15);
    update(100);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmContentControl, canReofferWhileStillShowingSceneAfterAcceptStopOffer_SlowHide)
{
    // show content
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    update();

    // sudden reoffer while scene is still showing
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();

    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();
}

TEST_F(ADcsmContentControl, canReofferWhileStillShowingSceneAfterAcceptStopOffer_FastHide)
{
    // show content
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    update();

    // scene is reported hidden (and continues to unsubscribe)
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);

    // sudden reoffer while scene is still showing
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();

    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // dcsm goes back to ready and our scene state is still ready, but no event here, because scene is unsubscribing!
    m_dcsmHandler.contentReady(m_contentID1);
    update();

    // full reramp up of scene
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, _));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // finally at our requested ready state, emit event
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();
}

TEST_F(ADcsmContentControl, scheduleContentHideBeforeAcceptingStopOfferRequest)
{
    // show content
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update();

    // request stop offer
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    // schedule release of content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 20, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 20, 100 }));
    // and accept stop offer with same timing
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 20, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 20, 100 }));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));

    update(10);
    update(50);

    // 2 scene state change requests were scheduled - one for hide request, one as result of accepted stop offer
    // the latter overrides the first
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    update(100);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Unavailable);
    update(110);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, contentCanBeReofferedAfterStopOfferRequestAccepted)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));

    update(10);
    update(20);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Unavailable);
    update(21);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, failsToSetSizeOfUnknownCategory)
{
    CategoryInfoUpdate categoryInfo{ {0, 0}, { 0, 0, 1, 1 } };
    EXPECT_NE(StatusOK, m_dcsmContentControl.setCategoryInfo(Category{ 9999 }, categoryInfo, AnimationInformation{}));
}

TEST_F(ADcsmContentControl, sendsCategoryRectChangeToAllContentsAssociatedWithIt)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate2, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2, ETechnicalContentType::RamsesSceneID);

    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID2));
    update();

    CategoryInfoUpdate newCategoryInfo{ {0, 0}, { 0, 0, 333, 111 } };
    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, _, AnimationInformation{ 500, 1000 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
        EXPECT_EQ(newCategoryInfo, infoupdate);
        return StatusOK;
        });
    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID2, _, AnimationInformation{ 500, 1000 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
        EXPECT_EQ(newCategoryInfo, infoupdate);
        return StatusOK;
        });
    EXPECT_EQ(StatusOK, m_dcsmContentControl.setCategoryInfo(m_categoryID1, newCategoryInfo, AnimationInformation{ 500, 1000 }));

    CategoryInfoUpdate otherNewCategoryInfo{ {0, 0}, { 0, 0, 666, 999 } };
    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID3, _, AnimationInformation{ 10, 100 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
        EXPECT_EQ(otherNewCategoryInfo, infoupdate);
        return StatusOK;
        });
    EXPECT_EQ(StatusOK, m_dcsmContentControl.setCategoryInfo(m_categoryID2, otherNewCategoryInfo, AnimationInformation{ 10, 100 }));
}

TEST_F(ADcsmContentControl, sendsLatestCategoryRectToNewContentOfferedForItEvenIfNotReachedTheFinishTimeForTheSizeChange)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1,  _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    CategoryInfoUpdate categoryInfo{ {0, 0}, { 0, 0, 333, 111 } };
    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, _, AnimationInformation{ 500, 1000 })).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
        EXPECT_EQ(categoryInfo, infoupdate);
        return StatusOK;
        });
    EXPECT_EQ(StatusOK, m_dcsmContentControl.setCategoryInfo(m_categoryID1, categoryInfo, AnimationInformation{ 500, 1000 }));

    // before start time
    update(10);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(categoryInfo, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));

    // between start and end time
    update(700);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(categoryInfo, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID1));

    // after end time
    const ContentID contentID4{ m_contentID3.getValue() + 1 };
    update(1100);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(contentID4, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(categoryInfo, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(contentID4, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID4, m_categoryID1));

    update(2000);
}

TEST_F(ADcsmContentControl, failsToShowAfterReleasingContent)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));

    // attempt to show content now fails
    EXPECT_NE(StatusOK, m_dcsmContentControl.showContent(m_contentID1, { 0, 0 }));

    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    update(100);
}

TEST_F(ADcsmContentControl, canRequestReadyAgainAfterReleasingContent)
{
    // show content
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update();

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));
    update(10);

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    update(100);
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(110);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(110);

    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

    m_dcsmHandler.contentReady(m_contentID1);
    update(120);
}

TEST_F(ADcsmContentControl, canRequestReadyAgainWhileReleasingContent)
{
    // show content
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update();

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));
    update(10);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(50);

    m_dcsmHandler.contentReady(m_contentID1);
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, _));
    update(100);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update(110);
}

TEST_F(ADcsmContentControl, canRequestReadyButReleaseBeforeReadyConfirmedFromDcsmProvider)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    // no call to renderer to change state because it is already only available
    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    // stays available
    update(100);
    update(1000);
}

TEST_F(ADcsmContentControl, canRequestReadyButReleaseBeforeReadyConfirmedFromRenderer)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // dcsm ready
    m_dcsmHandler.contentReady(m_contentID1);
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate scene just became ready from renderer
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    // no change reported but internally will set scene state to available because it got ready from renderer
    update(10);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update(10);

    // stays available
    update(100);
    update(1000);
}

TEST_F(ADcsmContentControl, canRequestReadyButReleaseBeforeDcsmReadyConfirmedAndThenMakeReadyAgain)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    // no call to renderer to change state because it is already only available
    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

    update(100);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(100);
    m_dcsmHandler.contentReady(m_contentID1);
    update(100);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(100);
}

TEST_F(ADcsmContentControl, canRequestReadyButReleaseBeforeRendererReadyConfirmedAndThenMakeReadyAgain)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // dcsm ready
    m_dcsmHandler.contentReady(m_contentID1);
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate scene just became ready from renderer
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    // no change reported but internally will set scene state to available because it got ready from renderer
    update(10);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update(10);

    update(100);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(100);
    m_dcsmHandler.contentReady(m_contentID1);
    update(100);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(100);
}

TEST_F(ADcsmContentControl, willNotTryToMapSceneIfReadyRequestedButForcedStopOfferedBeforeProviderReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // simulate force stop offer
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    update(100);

    // request ready fails as content not available
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmContentControl, willNotTryToMapSceneIfReadyRequestedButAcceptedStopOfferBeforeProviderReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update();
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // simulate request stop offer
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();
    // accept right away
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    // no scene state change requested, stays at Available
    update();

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    update(100);

    // request ready fails as content not available
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmContentControl, handlesTimeOutWhenRequestReadyButNotReachedDcsmReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update(100); // set initial time
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update(100);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 50)); // give 50 time units for timeout
    update(111);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);

    // dcsm ready does not come
    update(121);
    update(131);
    update(141);

    // will explicitly request state back to DCSM assigned
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::TimedOut));
    update(151);

    // simulate race - dcsm ready arrives right after timeout before dcsm set state back to assigned
    m_dcsmHandler.contentReady(m_contentID1);
    // nothing expected
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(200);
    // handle dcsm ready
    m_dcsmHandler.contentReady(m_contentID1);
    update(200);
    // handle new scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, handlesTimeOutWhenRequestReadyButNotReachedRendererReadyInTime)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update(100); // set initial time
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update(100);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 50)); // give 50 time units for timeout
    update(111);

    // handle dcsm ready
    update(121);
    m_dcsmHandler.contentReady(m_contentID1);

    // renderer ready does not come
    update(131);
    update(141);

    // will explicitly request state back to DCSM assigned
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    // will set scene state back to available on renderer side
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::TimedOut));
    update(151);

    // simulate race - renderer ready arrives right after timeout before renderer set state back to available
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    // nothing expected
    update(200);
    // due to race renderer actually reached ready state and will change state back to available which was requested when timed out
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update(200);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(200);
    // handle dcsm ready
    m_dcsmHandler.contentReady(m_contentID1);
    update(200);
    // handle new scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, handlesTimeOutWhenRequestReadyButNotReachedRendererReadyEver)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update(100); // set initial time
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update(100);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 50)); // give 50 time units for timeout
    update(111);

    // handle dcsm ready
    update(121);
    m_dcsmHandler.contentReady(m_contentID1);

    // renderer ready does not come
    update(131);
    update(141);

    // will explicitly request state back to DCSM assigned
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    // will set scene state back to available on renderer side
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::TimedOut));
    update(151);

    // nothing expected
    update(200);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(200);
    // handle dcsm ready
    m_dcsmHandler.contentReady(m_contentID1);
    update(200);
    // handle new scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, handlesTimeOutWhenRequestReadyButNotReachedDcsmAndRendererReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    update(100); // set initial time
    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update(100);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 50)); // give 50 time units for timeout
    update(111);

    // dcsm ready does not come
    update(121);
    update(131);
    update(141);

    // will explicitly request state back to DCSM assigned
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    // will set scene state back to available on renderer side
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::TimedOut));
    update(151);

    // nothing expected

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(200);
    // handle dcsm ready
    m_dcsmHandler.contentReady(m_contentID1);
    update(200);
    // handle new scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, failsToDataLinkUnknownProviderContent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, failsToDataLinkUnknownConsumerContent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, failsToDataLinkProviderContentWithUnknownScene)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, failsToDataLinkConsumerContentWithUnknownScene)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));

    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, requestsDataLink)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();

    // contents share same scene, so provider scene is same as consumer scene
    EXPECT_CALL(m_sceneControlMock, linkData(SceneId1, providerId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, reportsDataLinkedWithCorrectContents)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_eventHandlerMock, dataLinked(m_contentID1, providerId, m_contentID2, consumerId, true));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(m_contentID1, providerId, m_contentID2, consumerId, false));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);
}

TEST_F(ADcsmContentControl, reportsDataLinkedWithZeroContentIfContentGoneBeforeEvent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();

    // link data
    EXPECT_CALL(m_sceneControlMock, linkData(SceneId1, providerId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));

    // remove provider content
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID::Invalid(), providerId, m_contentID2, consumerId, true));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID::Invalid(), providerId, m_contentID2, consumerId, false));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);

    // remove consumer content
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId2, RendererSceneState::Available));
    m_dcsmHandler.forceContentOfferStopped(m_contentID2);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID2));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID::Invalid(), providerId, ContentID::Invalid(), consumerId, true));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID::Invalid(), providerId, ContentID::Invalid(), consumerId, false));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);
}

TEST_F(ADcsmContentControl, failsToUnlinkDataUnknownConsumerContent)
{
    constexpr dataConsumerId_t consumerId{ 13 };
    EXPECT_NE(StatusOK, m_dcsmContentControl.unlinkData(m_contentID1, consumerId));
}

TEST_F(ADcsmContentControl, failsToUnlinkDataConsumerContentWithUnknownScene)
{
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.unlinkData(m_contentID1, consumerId));
}

TEST_F(ADcsmContentControl, requestsDataUnlink)
{
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_sceneControlMock, unlinkData(SceneId1, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.unlinkData(m_contentID1, consumerId));
}

TEST_F(ADcsmContentControl, reportsDataUnlinkedWithCorrectContents)
{
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_eventHandlerMock, dataUnlinked(m_contentID1, consumerId, true));
    m_sceneControlHandler.dataUnlinked(SceneId1, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataUnlinked(m_contentID1, consumerId, false));
    m_sceneControlHandler.dataUnlinked(SceneId1, consumerId, false);
}

TEST_F(ADcsmContentControl, reportsDataUnlinkedWithZeroContentIfContentGoneBeforeEvent)
{
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // unlink data with content known
    EXPECT_CALL(m_sceneControlMock, unlinkData(SceneId1, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.unlinkData(m_contentID1, consumerId));

    // remove content
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report data unlinked
    EXPECT_CALL(m_eventHandlerMock, dataUnlinked(ContentID::Invalid(), consumerId, true));
    m_sceneControlHandler.dataUnlinked(SceneId1, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataUnlinked(ContentID::Invalid(), consumerId, false));
    m_sceneControlHandler.dataUnlinked(SceneId1, consumerId, false);
}

TEST_F(ADcsmContentControl, failsToHandlePickEventUnknownContent)
{
    EXPECT_NE(StatusOK, m_dcsmContentControl.handlePickEvent(m_contentID2, 1.0f, 1.0f));
}

TEST_F(ADcsmContentControl, failsToHandlePickEventContentWithUnknownScene)
{
    constexpr float x_coord = 1.0f;
    constexpr float y_coord = 2.0f;

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate2, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID2, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID2));

    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.handlePickEvent(m_contentID1, x_coord, y_coord));
}

TEST_F(ADcsmContentControl, requestsHandlePickEvent)
{
    constexpr float x_coord = 1.0f;
    constexpr float y_coord = 2.0f;

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_sceneControlMock, handlePickEvent(SceneId1, x_coord, y_coord));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.handlePickEvent(m_contentID1, x_coord, y_coord));
}

TEST_F(ADcsmContentControl, reportsObjectsPickedWithCorrectContents)
{
    constexpr std::array<pickableObjectId_t, 2> pickableObjects1{ pickableObjectId_t{567u}, pickableObjectId_t{578u} };
    constexpr std::array<pickableObjectId_t, 2> pickableObjects2{ pickableObjectId_t{111u}, pickableObjectId_t{222u} };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_eventHandlerMock, objectsPicked(m_contentID1, _, static_cast<uint32_t>(pickableObjects1.size()))).WillOnce(Invoke([&](auto, auto pickedObjects, auto pickedObjectsCount)
    {
        ASSERT_EQ(static_cast<uint32_t>(pickableObjects1.size()), pickedObjectsCount);
        for (uint32_t i = 0u; i < pickedObjectsCount; ++i)
            EXPECT_EQ(pickableObjects1[i], pickedObjects[i]);
    }));
    m_sceneControlHandler.objectsPicked(SceneId1, pickableObjects1.data(), static_cast<uint32_t>(pickableObjects1.size()));
    update();

    // different content with different scene
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
        });
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    update();
    m_dcsmHandler.contentDescription(m_contentID2, TechnicalContentDescriptor{ SceneId2.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Available);
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId2, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId2, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID2, 0));
    update();

    EXPECT_CALL(m_eventHandlerMock, objectsPicked(m_contentID2, _, static_cast<uint32_t>(pickableObjects2.size()))).WillOnce(Invoke([&](auto, auto pickedObjects, auto pickedObjectsCount)
    {
        ASSERT_EQ(static_cast<uint32_t>(pickableObjects2.size()), pickedObjectsCount);
        for (uint32_t i = 0u; i < pickedObjectsCount; ++i)
            EXPECT_EQ(pickableObjects2[i], pickedObjects[i]);
    }));
    m_sceneControlHandler.objectsPicked(SceneId2, pickableObjects2.data(), static_cast<uint32_t>(pickableObjects2.size()));
    update();
}

TEST_F(ADcsmContentControl, reportsObjectsPickedWithZeroContentIfContentGoneBeforeEvent)
{
    constexpr  pickableObjectId_t pickable1{ 567u };
    constexpr  pickableObjectId_t pickable2{ 578u };
    constexpr std::array<pickableObjectId_t, 2> pickableObjects{ pickable1, pickable2 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // handle Pick Event
    EXPECT_CALL(m_sceneControlMock, handlePickEvent(SceneId1, 1.0f, 1.0f));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.handlePickEvent(m_contentID1, 1.0f, 1.0f));

    // remove content with scene to be picked
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report objects picked
    EXPECT_CALL(m_eventHandlerMock, objectsPicked(ContentID::Invalid(), _, static_cast<uint32_t>(pickableObjects.size()))).WillOnce(Invoke([&](auto, auto pickedObjects, auto pickedObjectsCount)
    {
        ASSERT_EQ(static_cast<uint32_t>(pickableObjects.size()), pickedObjectsCount);
        for (uint32_t i = 0u; i < pickedObjectsCount; ++i)
            EXPECT_EQ(pickableObjects[i], pickedObjects[i]);
    }));
    m_sceneControlHandler.objectsPicked(SceneId1, pickableObjects.data(), static_cast<uint32_t>(pickableObjects.size()));
    update();
}

TEST_F(ADcsmContentControl, handlesDcsmMetadataUpdate)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();

    DcsmMetadataUpdate mdp(*new DcsmMetadataUpdateImpl);
    ramses_internal::DcsmMetadata md;
    md.setPreviewDescription(U"foobar");
    mdp.impl.setMetadata(md);
    m_dcsmHandler.contentMetadataUpdated(m_contentID1, mdp);

    EXPECT_CALL(m_eventHandlerMock, contentMetadataUpdated(m_contentID1, _)).
        WillOnce(Invoke([&](auto, auto& prov) { EXPECT_EQ(md, prov.impl.getMetadata()); }));
    update();
}

TEST_F(ADcsmContentControl, ignoresDcsmMetadataUpdateForUnknownContent)
{
    DcsmMetadataUpdate mdp(*new DcsmMetadataUpdateImpl);
    ramses_internal::DcsmMetadata md;
    md.setPreviewDescription(U"foobar");
    mdp.impl.setMetadata(md);
    m_dcsmHandler.contentMetadataUpdated(m_contentID1, mdp);

    update();
}

TEST_F(ADcsmContentControl, canAssignContentToDisplayBuffer)
{
    // in order to assign content to buffer, it must be at least DCSM ready
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, displayBuffer, 11));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID1, displayBuffer, 11));
}

TEST_F(ADcsmContentControl, failsToAssignUnknownContentToDisplayBuffer)
{
    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_NE(StatusOK, m_dcsmContentControl.assignContentToDisplayBuffer(ContentID{ 666u }, displayBuffer, 11));
}

TEST_F(ADcsmContentControl, failsToAssignNotReadyContentToDisplayBuffer)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();
    // content not reported as ready yet, therefore its scene is not known and mapping info was not set yet

    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_NE(StatusOK, m_dcsmContentControl.assignContentToDisplayBuffer(ContentID{ 666u }, displayBuffer, 11));
}

TEST_F(ADcsmContentControl, failsToAssignContentToDisplayBufferIfInternalRequestFails)
{
    // in order to assign content to buffer, it must be at least DCSM ready
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1);

    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, displayBuffer, 11)).WillOnce(Return(status_t{666}));
    EXPECT_NE(StatusOK, m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID1, displayBuffer, 11));
}

TEST_F(ADcsmContentControl, failsToOffscreenBufferLinkUnknownConsumerContent)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };
    EXPECT_NE(StatusOK, m_dcsmContentControl.linkOffscreenBuffer(obId, m_contentID1, consumerId));
}

TEST_F(ADcsmContentControl, failsToOffscreenBufferLinkConsumerContentWithUnknownScene)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::RamsesSceneID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkOffscreenBuffer(obId, m_contentID1, consumerId));
}


TEST_F(ADcsmContentControl, linkContentWithoutReadyGivesError)
{
    //make consumer ready but not actual content
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();
    //fail because content not ready
    EXPECT_NE(StatusOK, m_dcsmContentControl.linkContentToTextureConsumer(m_contentID1, m_contentID2, dataConsumerId_t{13}));
}

TEST_F(ADcsmContentControl, linkContentWithoutConsumerReadyGivesError)
{
    // make content ready but not consumer
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
    // fail because consumer not ready
    EXPECT_NE(StatusOK, m_dcsmContentControl.linkContentToTextureConsumer(m_contentID1, m_contentID2, dataConsumerId_t{13}));
}

TEST_F(ADcsmContentControl, linkContentWithoutOBAssignmentGivesErrorforRamsesContent)
{
    constexpr dataConsumerId_t consumerId{ 13 };

    //make consumer ready
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();
    // make provider scene ready but not assigned to OB
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkContentToTextureConsumer(m_contentID1, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, linkContentLinksOBInternallyForRamsesType)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };
    constexpr int renderOrder = 123;
    auto& contentid = m_contentID1;
    auto& consumerContentID = m_contentID2;

    //make content and consumer ready
    makeDcsmContentReady_ramses(contentid, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(consumerContentID, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(contentid, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(consumerContentID, DcsmContentControlEventResult::OK));
    update();
    // mandatory assign to OB
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, obId, renderOrder));
    m_dcsmContentControl.assignContentToDisplayBuffer(contentid, obId, renderOrder);

    // calls offscreenbuffer linking targeting previously assigned display buffer
    EXPECT_CALL(m_sceneControlMock, linkOffscreenBuffer(obId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkContentToTextureConsumer(contentid, consumerContentID, consumerId));
}

TEST_F(ADcsmContentControl, mustReassignToDisplayBufferAfterReleaseForlinkContent)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };
    constexpr int renderOrder = 123;

    //make everything ready
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();
    //assign
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, obId, renderOrder));
    m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID1, obId, renderOrder);

    //sucessfully link
    EXPECT_CALL(m_sceneControlMock, linkOffscreenBuffer(obId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkContentToTextureConsumer(m_contentID1, m_contentID2, consumerId));

    // ramp down content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, _));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    m_dcsmContentControl.releaseContent(m_contentID1, {});
    update();
    // renderer ramps down as reaction
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    Mock::VerifyAndClearExpectations(&m_eventHandlerMock);

    // make/request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, _));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(m_lastUpdateTS);
    m_dcsmHandler.contentReady(m_contentID1);
    update(m_lastUpdateTS);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    // must reassign first, therefore error
    EXPECT_NE(StatusOK, m_dcsmContentControl.linkContentToTextureConsumer(m_contentID1, m_contentID2, consumerId));

    // assign again
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, obId, renderOrder));
    m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID1, obId, renderOrder);

    // successfully linkContent
    EXPECT_CALL(m_sceneControlMock, linkOffscreenBuffer(obId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkContentToTextureConsumer(m_contentID1, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, assignTwoContentsWithSameUnderlyingScene_sameOB)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr int renderOrder = 123;

    //make everything ready
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId1, true);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    //m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();
    //assign
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, obId, renderOrder)).Times(2);
    m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID1, obId, renderOrder);
    m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID2, obId, renderOrder);
}

TEST_F(ADcsmContentControl, assignTwoContentsWithSameUnderlyingScene_separateOBs_lastAssignWins)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr displayBufferId_t obIdB{ 13 };
    constexpr int renderOrder = 123;

    //make everything ready
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId1, true);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();
    //assign
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, obId, renderOrder));
    m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID1, obId, renderOrder);
    EXPECT_CALL(m_sceneControlMock, setSceneDisplayBufferAssignment(SceneId1, obIdB, renderOrder));
    m_dcsmContentControl.assignContentToDisplayBuffer(m_contentID2, obIdB, renderOrder);
}

TEST_F(ADcsmContentControl, requestsOffscreenBufferLink)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_sceneControlMock, linkOffscreenBuffer(obId, SceneId1, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkOffscreenBuffer(obId, m_contentID1, consumerId));
}

TEST_F(ADcsmContentControl, reportsOffscreenBufferLinkedWithCorrectContents)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, m_contentID1, consumerId, true));
    m_sceneControlHandler.offscreenBufferLinked(obId, SceneId1, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, m_contentID1, consumerId, false));
    m_sceneControlHandler.offscreenBufferLinked(obId, SceneId1, consumerId, false);
}

TEST_F(ADcsmContentControl, reportsOffscreenBufferLinkedWithZeroContentIfContentGoneBeforeEvent)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // link data
    EXPECT_CALL(m_sceneControlMock, linkOffscreenBuffer(obId, SceneId1, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkOffscreenBuffer(obId, m_contentID1, consumerId));

    // remove provider content
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, ContentID::Invalid(), consumerId, true));
    m_sceneControlHandler.offscreenBufferLinked(obId, SceneId1, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, ContentID::Invalid(), consumerId, false));
    m_sceneControlHandler.offscreenBufferLinked(obId, SceneId1, consumerId, false);
}

TEST_F(ADcsmContentControl, reportsContentFlushed)
{
    constexpr sceneVersionTag_t version1{ 12 };
    constexpr sceneVersionTag_t version2{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID1, version1));
    m_sceneControlHandler.sceneFlushed(SceneId1, version1);

    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID2, version2));
    m_sceneControlHandler.sceneFlushed(SceneId2, version2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentFlushed_SceneAssociatedWithMultipleContents)
{
    constexpr sceneVersionTag_t version1{ 12 };
    constexpr sceneVersionTag_t version2{ 13 };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId1, true);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID1, version1));
    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID2, version1));
    m_sceneControlHandler.sceneFlushed(SceneId1, version1);

    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID1, version2));
    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID2, version2));
    m_sceneControlHandler.sceneFlushed(SceneId1, version2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentExpirationMonitoringEnabledDisabled)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringEnabled(m_contentID1));
    m_sceneControlHandler.sceneExpirationMonitoringEnabled(SceneId1);

    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringEnabled(m_contentID2));
    m_sceneControlHandler.sceneExpirationMonitoringEnabled(SceneId2);

    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringDisabled(m_contentID1));
    m_sceneControlHandler.sceneExpirationMonitoringDisabled(SceneId1);

    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringDisabled(m_contentID2));
    m_sceneControlHandler.sceneExpirationMonitoringDisabled(SceneId2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentExpirationMonitoringEnabledDisabled_SceneAssociatedWithMultipleContents)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId1, true);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringEnabled(m_contentID1));
    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringEnabled(m_contentID2));
    m_sceneControlHandler.sceneExpirationMonitoringEnabled(SceneId1);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringDisabled(m_contentID1));
    EXPECT_CALL(m_eventHandlerMock, contentExpirationMonitoringDisabled(m_contentID2));
    m_sceneControlHandler.sceneExpirationMonitoringDisabled(SceneId1);
    update();
}

TEST_F(ADcsmContentControl, reportsContentExpired)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID1));
    m_sceneControlHandler.sceneExpired(SceneId1);

    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID2));
    m_sceneControlHandler.sceneExpired(SceneId2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentExpired_SceneAssociatedWithMultipleContents)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId1, true);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID1));
    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID2));
    m_sceneControlHandler.sceneExpired(SceneId1);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID1));
    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID2));
    m_sceneControlHandler.sceneExpired(SceneId1);
    update();
}

TEST_F(ADcsmContentControl, reportsContentRecoveredFromExpiration)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID1));
    m_sceneControlHandler.sceneRecoveredFromExpiration(SceneId1);

    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID2));
    m_sceneControlHandler.sceneRecoveredFromExpiration(SceneId2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentRecoveredFromExpiration_SceneAssociatedWithMultipleContents)
{
    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId1, true);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID1));
    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID2));
    m_sceneControlHandler.sceneRecoveredFromExpiration(SceneId1);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID1));
    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID2));
    m_sceneControlHandler.sceneRecoveredFromExpiration(SceneId1);
    update();
}

TEST_F(ADcsmContentControl, reportsStreamAvailabilityChange)
{
    constexpr waylandIviSurfaceId_t streamId1{ 123 };
    constexpr waylandIviSurfaceId_t streamId2{ 124 };

    EXPECT_CALL(m_eventHandlerMock, streamAvailabilityChanged(streamId1, true));
    m_sceneControlHandler.streamAvailabilityChanged(streamId1, true);

    EXPECT_CALL(m_eventHandlerMock, streamAvailabilityChanged(streamId1, false));
    m_sceneControlHandler.streamAvailabilityChanged(streamId1, false);

    update();

    EXPECT_CALL(m_eventHandlerMock, streamAvailabilityChanged(streamId2, true));
    m_sceneControlHandler.streamAvailabilityChanged(streamId2, true);

    EXPECT_CALL(m_eventHandlerMock, streamAvailabilityChanged(streamId2, false));
    m_sceneControlHandler.streamAvailabilityChanged(streamId2, false);

    update();
}

TEST_F(ADcsmContentControl, reportsDataSlotEvent)
{
    constexpr dataProviderId_t dataProviderId1{ 1u };
    constexpr dataProviderId_t dataProviderId2{ 2u };
    constexpr dataConsumerId_t dataConsumerId1{ 3u };
    constexpr dataConsumerId_t dataConsumerId2{ 4u };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId2);
    update();

    EXPECT_CALL(m_eventHandlerMock, dataProviderCreated(m_contentID1, dataProviderId1));
    m_sceneControlHandler.dataProviderCreated(SceneId1, dataProviderId1);

    EXPECT_CALL(m_eventHandlerMock, dataProviderDestroyed(m_contentID2, dataProviderId2));
    m_sceneControlHandler.dataProviderDestroyed(SceneId2, dataProviderId2);

    EXPECT_CALL(m_eventHandlerMock, dataConsumerCreated(m_contentID1, dataConsumerId1));
    m_sceneControlHandler.dataConsumerCreated(SceneId1, dataConsumerId1);

    EXPECT_CALL(m_eventHandlerMock, dataConsumerDestroyed(m_contentID2, dataConsumerId2));
    m_sceneControlHandler.dataConsumerDestroyed(SceneId2, dataConsumerId2);

    update();
}

TEST_F(ADcsmContentControl, reportsDataSlotEvent_SceneAssociatedWithMultipleContents)
{
    constexpr dataProviderId_t dataProviderId{ 1u };
    constexpr dataConsumerId_t dataConsumerId{ 2u };

    makeDcsmContentReady_ramses(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady_ramses(m_contentID2, m_categoryID1, SceneId1, true);
    update();

    EXPECT_CALL(m_eventHandlerMock, dataProviderCreated(m_contentID1, dataProviderId));
    EXPECT_CALL(m_eventHandlerMock, dataProviderCreated(m_contentID2, dataProviderId));
    m_sceneControlHandler.dataProviderCreated(SceneId1, dataProviderId);

    EXPECT_CALL(m_eventHandlerMock, dataProviderDestroyed(m_contentID1, dataProviderId));
    EXPECT_CALL(m_eventHandlerMock, dataProviderDestroyed(m_contentID2, dataProviderId));
    m_sceneControlHandler.dataProviderDestroyed(SceneId1, dataProviderId);

    EXPECT_CALL(m_eventHandlerMock, dataConsumerCreated(m_contentID1, dataConsumerId));
    EXPECT_CALL(m_eventHandlerMock, dataConsumerCreated(m_contentID2, dataConsumerId));
    m_sceneControlHandler.dataConsumerCreated(SceneId1, dataConsumerId);

    EXPECT_CALL(m_eventHandlerMock, dataConsumerDestroyed(m_contentID1, dataConsumerId));
    EXPECT_CALL(m_eventHandlerMock, dataConsumerDestroyed(m_contentID2, dataConsumerId));
    m_sceneControlHandler.dataConsumerDestroyed(SceneId1, dataConsumerId);

    update();
}

TEST_F(ADcsmContentControl, noSupportForWaylandIviSurfaceIdContentButOfferAndRequestsHandledGracefully)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, _)).WillOnce([&](const auto&, const auto& infoupdate) {
        EXPECT_EQ(m_CategoryInfoUpdate1, infoupdate);
        return StatusOK;
    });
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1, ETechnicalContentType::WaylandIviSurfaceID);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    //content description with type WaylandIviSurfaceID does not cause contentAvailable call to the user (m_eventHandlerMock is StrictMock)
    m_dcsmHandler.contentDescription(m_contentID1, TechnicalContentDescriptor{543});

    // cannot request ready
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();

    // cannot request show
    EXPECT_NE(StatusOK, m_dcsmContentControl.showContent(m_contentID1, AnimationInformation{}));
    update();

    // cannot request hide
    EXPECT_NE(StatusOK, m_dcsmContentControl.hideContent(m_contentID1, AnimationInformation{}));
    update();
}
