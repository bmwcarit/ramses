//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "DcsmContentControlImpl.h"
#include "ramses-renderer-api/IDcsmContentControlEventHandler.h"
#include "ramses-renderer-api/DcsmContentControlConfig.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "RendererSceneControlImpl.h"
#include "DcsmMetadataUpdateImpl.h"
#include "DcsmConsumerMock.h"
#include <memory.h>

using namespace ramses;
using namespace testing;

class DcsmContentControlEventHandlerMock : public IDcsmContentControlEventHandler
{
public:
    MOCK_METHOD2(contentAvailable, void(ContentID, Category));
    MOCK_METHOD2(contentReady, void(ContentID, DcsmContentControlEventResult));
    MOCK_METHOD1(contentShown, void(ContentID));
    MOCK_METHOD1(contentFocusRequested, void(ContentID));
    MOCK_METHOD1(contentStopOfferRequested, void(ContentID));
    MOCK_METHOD1(contentNotAvailable, void(ContentID));
    MOCK_METHOD2(contentMetadataUpdated, void(ContentID, const DcsmMetadataUpdate&));
    MOCK_METHOD4(offscreenBufferLinked, void(displayBufferId_t, ContentID, dataConsumerId_t, bool));
    MOCK_METHOD5(dataLinked, void(ContentID, dataProviderId_t, ContentID, dataConsumerId_t, bool));
    MOCK_METHOD2(contentFlushed, void(ContentID, sceneVersionTag_t));
    MOCK_METHOD1(contentExpired, void(ContentID));
    MOCK_METHOD1(contentRecoveredFromExpiration, void(ContentID));
    MOCK_METHOD2(streamAvailabilityChanged, void(streamSource_t, bool));
};

class RendererSceneControlMock : public IRendererSceneControl
{
public:
    MOCK_METHOD2(setSceneState, status_t(sceneId_t, RendererSceneState));
    MOCK_METHOD2(setSceneMapping, status_t(sceneId_t, displayId_t));
    MOCK_METHOD3(setSceneDisplayBufferAssignment, status_t(sceneId_t, displayBufferId_t, int32_t));
    MOCK_METHOD6(setDisplayBufferClearColor, status_t(displayId_t, displayBufferId_t, float, float, float, float));
    MOCK_METHOD3(linkOffscreenBuffer, status_t(displayBufferId_t, sceneId_t, dataConsumerId_t));
    MOCK_METHOD4(linkData, status_t(sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t));
    MOCK_METHOD2(unlinkData, status_t(sceneId_t, dataConsumerId_t));
    MOCK_METHOD0(flush, status_t());
    MOCK_METHOD1(dispatchEvents, status_t(IRendererSceneControlEventHandler&));
};

class ADcsmContentControl : public Test
{
public:
    ADcsmContentControl()
        : m_dcsmContentControl(DcsmContentControlConfig{ { {m_categoryID1, m_categoryInfo1}, {m_categoryID2, m_categoryInfo2} } }, m_dcsmConsumerMock, m_sceneControlMock)
        , m_dcsmHandler(m_dcsmContentControl)
        , m_sceneControlHandler(m_dcsmContentControl)
    {
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
        EXPECT_CALL(m_sceneControlMock, flush());
        EXPECT_EQ(StatusOK, m_dcsmContentControl.update(timeStamp, m_eventHandlerMock));
        m_lastUpdateTS = timeStamp;
    }

    void makeDcsmContentReady(ContentID contentID, Category categoryID, sceneId_t sceneId = SceneId1, bool alreadyAssignedToOtherContent = false)
    {
        // offered
        EXPECT_CALL(m_eventHandlerMock, contentAvailable(contentID, categoryID));
        EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(contentID, m_categoryInfo1.size));
        m_dcsmHandler.contentOffered(contentID, categoryID);
        update(m_lastUpdateTS);

        // provider sends content description right after assign
        if (!alreadyAssignedToOtherContent)
            EXPECT_CALL(m_sceneControlMock, setSceneState(sceneId, RendererSceneState::Available));
        else
        {
            EXPECT_CALL(m_sceneControlMock, setSceneMapping(sceneId, m_displayId));
            EXPECT_CALL(m_sceneControlMock, setSceneState(sceneId, RendererSceneState::Ready));
        }
        m_dcsmHandler.contentDescription(contentID, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneId.getValue() });
        update(m_lastUpdateTS);
        if (!alreadyAssignedToOtherContent)
            m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);

        // request ready
        EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(contentID, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
        EXPECT_CALL(m_sceneControlMock, setSceneMapping(sceneId, m_displayId));
        EXPECT_CALL(m_sceneControlMock, setSceneState(sceneId, RendererSceneState::Ready));
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
    const ContentID m_contentID1{ 321 };
    const ContentID m_contentID2{ 322 };
    const ContentID m_contentID3{ 323 };
    const displayId_t m_displayId{ 0 };
    const DcsmContentControlConfig::CategoryInfo m_categoryInfo1{ {16,16}, m_displayId };
    const DcsmContentControlConfig::CategoryInfo m_categoryInfo2{ {32,32}, m_displayId };

    StrictMock<DcsmConsumerMock> m_dcsmConsumerMock;
    StrictMock<RendererSceneControlMock> m_sceneControlMock;
    DcsmContentControlImpl m_dcsmContentControl;
    IDcsmConsumerEventHandler& m_dcsmHandler;
    IRendererSceneControlEventHandler& m_sceneControlHandler;
    StrictMock<DcsmContentControlEventHandlerMock> m_eventHandlerMock;
    uint64_t m_lastUpdateTS = 0;
};

constexpr sceneId_t ADcsmContentControl::SceneId1;
constexpr sceneId_t ADcsmContentControl::SceneId2;


TEST_F(ADcsmContentControl, handlesDcsmOffered)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
}

TEST_F(ADcsmContentControl, handlesDcsmContentDescription)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);

    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    update();
}

TEST_F(ADcsmContentControl, doesNotAssignWhenDcsmOfferedContentForUnregisteredCategory)
{
    m_dcsmHandler.contentOffered(m_contentID1, Category{ 666 });
    update();
}

TEST_F(ADcsmContentControl, handlesDcsmOfferedForMultipleContentsAndCategories)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    // content3 uses another category
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID2));
    // there is no mechanism to track if same state for same scene already requested because it is not needed, therefore 3 times
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available)).Times(3);
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_dcsmHandler.contentDescription(m_contentID2, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    m_dcsmHandler.contentDescription(m_contentID3, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();
}

TEST_F(ADcsmContentControl, requestsContentReady)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update();
}

TEST_F(ADcsmContentControl, handlesContentAndSceneReady)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();
}

TEST_F(ADcsmContentControl, handlesContentAndSceneReadyForMultipleContentsSharingSingleScene)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId1, true);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();

    // now request content3 ready
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID2));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();

    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID3, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID3, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID3, 0));
    m_dcsmHandler.contentReady(m_contentID3);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID3, DcsmContentControlEventResult::OK));
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

TEST_F(ADcsmContentControl, canGetContentRenderedAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.showContent(m_contentID1, timing));

    // start time not reached -> scene not shown -> combined state still ready
    update(10);

    // scene show triggered but not confirmed yet
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Rendered));
    update(20);
    update(30);

    // now report scene rendered
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Rendered);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(40);

    // stays rendered...
    update(50);
    update(60);
}

TEST_F(ADcsmContentControl, canGetContentRenderedAndThenHiddenAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

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

    update(30);
    // state still rendered because scene is still rendered...

    //...till finish time
    update(100);
    update(120);

    // at finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    update(150);

    // only after scene really hidden content state changes
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, hideContentAtGivenTimeAndThenOverrideWithEarlierTiming)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

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

    update(110);
    // still rendering to allow fade out animation

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
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(121);
}

TEST_F(ADcsmContentControl, hideContentAtGivenTimeAndThenOverrideWithLaterTiming)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

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
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(181);
}

TEST_F(ADcsmContentControl, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update(30);
    // announce new content's description
    const sceneId_t otherScene{ SceneId1.getValue() + 10 };
    EXPECT_CALL(m_sceneControlMock, setSceneState(otherScene, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID2, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ otherScene.getValue() });
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
    // scene is still shown within this period
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
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    update(1000);

    // only after scene 'downgraded' to available original content's state changes
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    update(1100);
}

TEST_F(ADcsmContentControl, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime_sharingScene)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update(30);

    // provider sends content description right after assign
    m_dcsmHandler.contentDescription(m_contentID2, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update(30);

    // first request new content ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID2, 0));
    update(30);

    // announce original content not needed at 1000 and start provider transition animation (if any) immediately
    // scene is still shown within this period
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 1000 }));

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

    // actual scene state is kept rendered but scheduled state change came into effect and original content state reached its target
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    update(1000);
    update(1100);
}

TEST_F(ADcsmContentControl, forcedStopOfferIsPropagatedAsEventAndMakesContentInvalid)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // content invalidated, cannot set state
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmContentControl, forcedStopOfferedContentCanBecomeOfferedAgain)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Unavailable);

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
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

TEST_F(ADcsmContentControl, propagatesFocusChangeRequestAsEvent)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.contentFocusRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentFocusRequested(m_contentID1));
    update();
}

TEST_F(ADcsmContentControl, focusChangeRequestForUnknownContentIsIgnored)
{
    m_dcsmHandler.contentFocusRequest(m_contentID1);
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
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    update(15);

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Unavailable));
    update(100);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmContentControl, scheduleContentHideBeforeAcceptingStopOfferRequest)
{
    // show content
    makeDcsmContentReady(m_contentID1, m_categoryID1);
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

    update(10);
    update(50);

    // 2 scene state change requests were scheduled - one for hide request, one as result of accepted stop offer
    // the latter overrides the first
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Unavailable));
    update(100);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Unavailable);
    update(110);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, contentCanBeReofferedAfterStopOfferRequestAccepted)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));

    update(10);
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Unavailable));
    update(20);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Unavailable);
    update(21);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update(200);
}

TEST_F(ADcsmContentControl, failsToSetSizeOfUnknownCategory)
{
    EXPECT_NE(StatusOK, m_dcsmContentControl.setCategorySize(Category{ 9999 }, SizeInfo{ 1, 1 }, AnimationInformation{}));
}

TEST_F(ADcsmContentControl, sendsCategorySizeChangeToAllContentsAssociatedWithIt)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID2, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.setCategorySize(m_categoryID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID3, SizeInfo{ 666, 999 }, AnimationInformation{ 10, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.setCategorySize(m_categoryID2, SizeInfo{ 666, 999 }, AnimationInformation{ 10, 100 }));
}

TEST_F(ADcsmContentControl, sendsLatestCategorySizeToNewContentOfferedForItEvenIfNotReachedTheFinishTimeForTheSizeChange)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.setCategorySize(m_categoryID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));

    // before start time
    update(10);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, SizeInfo{ 333, 111 }));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);

    // between start and end time
    update(700);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, SizeInfo{ 333, 111 }));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID1);

    // after end time
    const ContentID contentID4{ m_contentID3.getValue() + 1 };
    update(1100);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(contentID4, SizeInfo{ 333, 111 }));
    m_dcsmHandler.contentOffered(contentID4, m_categoryID1);

    update(2000);
}

TEST_F(ADcsmContentControl, failsToShowAfterReleasingContent)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));

    // attempt to show content now fails
    EXPECT_NE(StatusOK, m_dcsmContentControl.showContent(m_contentID1, { 0, 0 }));

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    update(100);
}

TEST_F(ADcsmContentControl, canRequestReadyAgainAfterReleasingContent)
{
    // show content
    makeDcsmContentReady(m_contentID1, m_categoryID1);
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
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));
    update(10);

    // cannot request ready till content released at animation finish
    EXPECT_NE(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));

    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    update(100);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Available);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, _));
    update(110);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_sceneControlMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Ready));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.requestContentReady(m_contentID1, 0));
    update(110);

    m_dcsmHandler.contentReady(m_contentID1);
    update(120);
}

TEST_F(ADcsmContentControl, canRequestReadyButReleaseBeforeReadyConfirmedFromDcsmProvider)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    // provider sends content description right after assign
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate scene just became ready from renderer, nothing expected
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    // no change reported but internally will set scene state to available because it got ready from renderer
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    // provider sends content description right after assign
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

    // no call to renderer to change state because it is already only available
    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1);

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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    // provider sends content description right after assign
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    EXPECT_EQ(StatusOK, m_dcsmContentControl.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate scene just became ready from renderer, nothing expected
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    update();

    // no change reported but internally will set scene state to available because it got ready from renderer
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    // provider sends content description right after assign
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    // provider sends content description right after assign
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Unavailable));
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

TEST_F(ADcsmContentControl, handlesTimeOutWhenRequestReadyButNotReachedDcsmReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update(100); // set initial time
    // provider sends content description right after assign
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    // will not set anything on renderer side because nothing was requested from renderer yet
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available)).Times(0);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::TimedOut));
    update(151);

    // simulate race - dcsm ready arrives right after timeout before dcsm set state back to assigned
    m_dcsmHandler.contentReady(m_contentID1);
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

TEST_F(ADcsmContentControl, handlesTimeOutWhenRequestReadyButNotReachedRendererReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update(100); // set initial time
    // provider sends content description right after assign
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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

TEST_F(ADcsmContentControl, failsToDataLinkUnknownProviderContent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, failsToDataLinkUnknownConsumerContent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, failsToDataLinkProviderContentWithUnknownScene)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, failsToDataLinkConsumerContentWithUnknownScene)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);

    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmContentControl, requestsDataLinkFromDisplayManager)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
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

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
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

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    m_sceneControlHandler.sceneStateChanged(SceneId2, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmContentControlEventResult::OK));
    update();

    // link data
    EXPECT_CALL(m_sceneControlMock, linkData(SceneId1, providerId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkData(m_contentID1, providerId, m_contentID2, consumerId));

    // remove provider content
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, m_contentID2, consumerId, true));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, m_contentID2, consumerId, false));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);

    // remove consumer content
    m_dcsmHandler.forceContentOfferStopped(m_contentID2);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID2));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, ContentID{ 0 }, consumerId, true));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, ContentID{ 0 }, consumerId, false));
    m_sceneControlHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);
}

TEST_F(ADcsmContentControl, handlesDcsmMetadataUpdate)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    makeDcsmContentReady(m_contentID1, m_categoryID1);

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
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_sceneControlMock, setSceneState(SceneId1, RendererSceneState::Available));
    m_dcsmHandler.contentDescription(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
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
    makeDcsmContentReady(m_contentID1, m_categoryID1);

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

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    EXPECT_NE(StatusOK, m_dcsmContentControl.linkOffscreenBuffer(obId, m_contentID1, consumerId));
}

TEST_F(ADcsmContentControl, requestsOffscreenBufferLinkFromDisplayManager)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
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

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
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

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    m_sceneControlHandler.sceneStateChanged(SceneId1, RendererSceneState::Ready);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmContentControlEventResult::OK));
    update();

    // link data
    EXPECT_CALL(m_sceneControlMock, linkOffscreenBuffer(obId, SceneId1, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.linkOffscreenBuffer(obId, m_contentID1, consumerId));

    // remove provider content
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, ContentID{ 0 }, consumerId, true));
    m_sceneControlHandler.offscreenBufferLinked(obId, SceneId1, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, ContentID{ 0 }, consumerId, false));
    m_sceneControlHandler.offscreenBufferLinked(obId, SceneId1, consumerId, false);
}

TEST_F(ADcsmContentControl, canSetClearColorOfDisplayBuffer)
{
    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_sceneControlMock, setDisplayBufferClearColor(m_displayId, displayBuffer, 1, 2, 3, 4));
    EXPECT_EQ(StatusOK, m_dcsmContentControl.setDisplayBufferClearColor(m_displayId, displayBuffer, 1, 2, 3, 4));
}

TEST_F(ADcsmContentControl, failsToSetClearColorIfInternalRequestFails)
{
    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_sceneControlMock, setDisplayBufferClearColor(m_displayId, displayBuffer, 1, 2, 3, 4)).WillOnce(Return(status_t{666}));
    EXPECT_NE(StatusOK, m_dcsmContentControl.setDisplayBufferClearColor(m_displayId, displayBuffer, 1, 2, 3, 4));
}

TEST_F(ADcsmContentControl, reportsContentFlushed)
{
    constexpr sceneVersionTag_t version1{ 12 };
    constexpr sceneVersionTag_t version2{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
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

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId1, true);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID1, version1));
    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID2, version1));
    m_sceneControlHandler.sceneFlushed(SceneId1, version1);

    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID1, version2));
    EXPECT_CALL(m_eventHandlerMock, contentFlushed(m_contentID2, version2));
    m_sceneControlHandler.sceneFlushed(SceneId1, version2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentExpired)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID1));
    m_sceneControlHandler.sceneExpired(SceneId1);

    EXPECT_CALL(m_eventHandlerMock, contentExpired(m_contentID2));
    m_sceneControlHandler.sceneExpired(SceneId2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentExpired_SceneAssociatedWithMultipleContents)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId1, true);
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
    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
    update();

    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID1));
    m_sceneControlHandler.sceneRecoveredFromExpiration(SceneId1);

    EXPECT_CALL(m_eventHandlerMock, contentRecoveredFromExpiration(m_contentID2));
    m_sceneControlHandler.sceneRecoveredFromExpiration(SceneId2);

    update();
}

TEST_F(ADcsmContentControl, reportsContentRecoveredFromExpiration_SceneAssociatedWithMultipleContents)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId1, true);
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
    constexpr streamSource_t streamId1{ 123 };
    constexpr streamSource_t streamId2{ 124 };

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
