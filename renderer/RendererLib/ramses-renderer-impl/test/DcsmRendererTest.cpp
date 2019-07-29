//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "DcsmRendererImpl.h"
#include "ramses-renderer-api/IDcsmRendererEventHandler.h"
#include "ramses-renderer-api/DcsmRendererConfig.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "DcsmConsumerMock.h"
#include "DisplayManagerMock.h"
#include <unordered_set>
#include <unordered_map>
#include <memory.h>

using namespace ramses;
using namespace testing;

class DcsmRendererContentStateTracker : public IDcsmRendererEventHandler
{
public:
    virtual void contentAvailable(ContentID contentID, Category categoryID) override
    {
        m_contentStates[contentID].lastState = ContentState::Available;
        m_contentStates[contentID].category = categoryID;
        ++m_numEvents;
    }
    virtual void contentReady(ContentID contentID, DcsmRendererEventResult result) override
    {
        if (result == DcsmRendererEventResult::OK)
            m_contentStates[contentID].lastState = ContentState::Ready;
        ++m_numEvents;
    }
    virtual void contentShown(ContentID contentID) override
    {
        m_contentStates[contentID].lastState = ContentState::Shown;
        ++m_numEvents;
    }
    virtual void contentHidden(ContentID contentID) override
    {
        m_contentStates[contentID].lastState = ContentState::Ready;
        ++m_numEvents;
    }
    virtual void contentReleased(ContentID contentID) override
    {
        m_contentStates[contentID].lastState = ContentState::Available;
        ++m_numEvents;
    }
    virtual void contentFocusRequested(ContentID contentID) override
    {
        m_contentStates[contentID].focusRequested = true;
        ++m_numEvents;
    }
    virtual void contentStopOfferRequested(ContentID contentID) override
    {
        m_contentStates[contentID].stopOfferRequested = true;
        ++m_numEvents;
    }
    virtual void contentNotAvailable(ContentID contentID) override
    {
        m_contentStates[contentID].notAvailableReported = true;
        m_contentStates[contentID].lastState = ContentState::Invalid;
        ++m_numEvents;
    }

    struct ContentTrackedState
    {
        ContentState lastState = ContentState::Invalid;
        Category category{ 0 };
        bool focusRequested = false;
        bool stopOfferRequested = false;
        bool notAvailableReported = false;
    };

    const ContentTrackedState& operator[](ContentID contentID)
    {
        return m_contentStates[contentID];
    }
    void resetLastState(ContentID contentID)
    {
        m_contentStates.erase(contentID);
    }

    unsigned m_numEvents = 0;

private:
    std::unordered_map<ContentID, ContentTrackedState> m_contentStates;
};

class ADcsmRenderer : public Test
{
public:
    ADcsmRenderer()
        : m_displayManagerMock(*new StrictMock<DisplayManagerMock>)
        , m_dcsmRenderer(DcsmRendererConfig{ { {m_categoryID1, m_categoryInfo1}, {m_categoryID2, m_categoryInfo2} } }, m_dcsmConsumerMock, std::unique_ptr<ramses_display_manager::IDisplayManager>(&m_displayManagerMock))
        , m_dcsmHandler(m_dcsmRenderer)
        , m_displayManagerHandler(m_dcsmRenderer)
    {
    }

    virtual void TearDown() override
    {
        // empty tracker instance to only count remaining events -> expect none at end of each test case
        DcsmRendererContentStateTracker eventCounter;
        EXPECT_CALL(m_dcsmConsumerMock, dispatchEvents(_));
        EXPECT_CALL(m_displayManagerMock, dispatchAndFlush(_, nullptr));
        EXPECT_EQ(StatusOK, m_dcsmRenderer.update(std::numeric_limits<uint64_t>::max(), eventCounter, nullptr));
        EXPECT_EQ(0u, eventCounter.m_numEvents);
    }

    void update(uint64_t timeStamp = 0)
    {
        EXPECT_CALL(m_dcsmConsumerMock, dispatchEvents(_));
        EXPECT_CALL(m_displayManagerMock, dispatchAndFlush(_, nullptr));
        EXPECT_EQ(StatusOK, m_dcsmRenderer.update(timeStamp, m_dcsmRendererContentStateTracker, nullptr));
    }

    void makeDcsmContentReady(ContentID contentID, Category categoryID)
    {
        // this is always new cycle for content, make sure there is no tracked state from last cycle
        m_dcsmRendererContentStateTracker.resetLastState(contentID);

        // offered
        EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(contentID, m_categoryInfo1.size));
        m_dcsmHandler.contentOffered(contentID, categoryID);
        update();
        EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[contentID].lastState);
        EXPECT_EQ(categoryID, m_dcsmRendererContentStateTracker[contentID].category);

        // request ready
        EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(contentID, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
        EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(contentID, 0));
        update();
        EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[contentID].lastState);

        EXPECT_CALL(m_displayManagerMock, setSceneMapping(m_sceneId, m_displayId, 0));
        EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
        m_dcsmHandler.contentReady(contentID, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
        update();
        EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[contentID].lastState);
    }

protected:
    const Category m_categoryID1{ 6 };
    const Category m_categoryID2{ 7 };
    const ContentID m_contentID1{ 321 };
    const ContentID m_contentID2{ 322 };
    const ContentID m_contentID3{ 323 };
    const sceneId_t m_sceneId{ 33 };
    const displayId_t m_displayId{ 0 };
    const DcsmRendererConfig::CategoryInfo m_categoryInfo1{ {16,16}, m_displayId };
    const DcsmRendererConfig::CategoryInfo m_categoryInfo2{ {32,32}, m_displayId };

    StrictMock<DcsmConsumerMock> m_dcsmConsumerMock;
    StrictMock<DisplayManagerMock>& m_displayManagerMock;
    DcsmRendererImpl m_dcsmRenderer;
    IDcsmConsumerEventHandler& m_dcsmHandler;
    ramses_display_manager::IEventHandler& m_displayManagerHandler;
    DcsmRendererContentStateTracker m_dcsmRendererContentStateTracker;
};

TEST_F(ADcsmRenderer, noEventsInitially)
{
    update();

    EXPECT_NE(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_NE(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_NE(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_FALSE(m_dcsmRendererContentStateTracker[m_contentID1].focusRequested);
    EXPECT_FALSE(m_dcsmRendererContentStateTracker[m_contentID1].stopOfferRequested);
    EXPECT_FALSE(m_dcsmRendererContentStateTracker[m_contentID1].notAvailableReported);
    EXPECT_EQ(0u, m_dcsmRendererContentStateTracker.m_numEvents);
}

TEST_F(ADcsmRenderer, handlesDcsmOffered)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(m_categoryID1, m_dcsmRendererContentStateTracker[m_contentID1].category);
}

TEST_F(ADcsmRenderer, doesNotReportOfferedWhenDcsmOfferedContentForUnregisteredCategory)
{
    m_dcsmHandler.contentOffered(m_contentID1, Category{ 666 });
    update();
    EXPECT_NE(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_NE(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_NE(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, handlesDcsmOfferedForMultipleContentsAndCategories)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    // content3 uses another category
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID3].lastState);
    EXPECT_EQ(m_categoryID1, m_dcsmRendererContentStateTracker[m_contentID1].category);
    EXPECT_EQ(m_categoryID1, m_dcsmRendererContentStateTracker[m_contentID2].category);
    EXPECT_EQ(m_categoryID2, m_dcsmRendererContentStateTracker[m_contentID3].category);
}

TEST_F(ADcsmRenderer, requestsContentReady)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, handlesContentAndSceneReady)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, handlesContentAndSceneReadyForMultipleContentsSharingSingleScene)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    makeDcsmContentReady(m_contentID2, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // now request content3 ready
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID3].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID3, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID3, 0));
    m_dcsmHandler.contentReady(m_contentID3, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID3].lastState);
}

TEST_F(ADcsmRenderer, canGetContentRenderedAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // start time not reached -> scene not shown -> combined state still ready
    update(10);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // scene show triggered but not confirmed yet
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update(20);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    update(30);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // now report scene rendered
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update(40);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // stays rendered...
    update(50);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    update(60);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, canGetContentRenderedAndThenHiddenAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update(20);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update(20);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce hide at another time
    const AnimationInformation timing2{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing2));

    update(30);
    // state still rendered because scene is still rendered...
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    //...till finish time
    update(100);
    update(120);

    // at finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    update(150);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // only after scene really hidden content state changes
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update(200);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, hideContentAtGivenTimeAndThenOverrideWithEarlierTiming)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce show at a time
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, { 0, 0 }));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update(0);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update(0);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce hide at a time
    const AnimationInformation timing{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing));

    update(110);
    // still rendering to allow fade out animation
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // now request hiding at earlier point
    const AnimationInformation timing2{ 100, 120 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing2));

    // still no change
    update(115);

    // at updated finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    update(120);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // only after scene really hidden content state changes
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update(121);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, hideContentAtGivenTimeAndThenOverrideWithLaterTiming)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce show at a time
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, { 0, 0 }));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update(0);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update(0);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce hide at a time
    const AnimationInformation timing{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing));

    update(110);
    // still rendering to allow fade out animation
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // now request hiding at later point
    const AnimationInformation timing2{ 100, 180 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing2));

    // still no change
    update(115);
    update(160);

    // at updated finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    update(180);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // only after scene really hidden content state changes
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update(181);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // content state unchanged till scene really shown
    update(10);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update(20);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update(20);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    //////////////////////////
    // new content offered, initiate content switch
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update(30);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // first request new content ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID2, 0));
    update(30);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // announce original content not needed at 1000 and start provider transition animation (if any) immediately
    // scene is still shown within this period
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 1000 }));

    // bit later new content should become ready
    const sceneId_t otherScene{ m_sceneId + 10 };
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(otherScene, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(otherScene, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID2, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ otherScene });
    update(100);
    // new content is not combined ready because its scene is not ready
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
    // handle new scene ready
    m_displayManagerHandler.sceneStateChanged(otherScene, ramses_display_manager::SceneState::Ready, m_displayId);
    update(100);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // announce show at 1000, starting provider side transition (if any) immediately
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Shown, AnimationInformation{ 0, 1000 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(otherScene, ramses_display_manager::SceneState::Rendered, _));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID2, AnimationInformation{ 0, 1000 }));
    update(110);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
    // simulate scene shown
    m_displayManagerHandler.sceneStateChanged(otherScene, ramses_display_manager::SceneState::Rendered, m_displayId);
    update(120);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    ///////

    // now reaching time 1000 original content's scene can be safely hidden - desired state was offered
    // content states already reached their final states
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Available, _));
    update(1000);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // only after scene 'downgraded' to available original content's state changes
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Available, m_displayId);
    update(1100);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
}

TEST_F(ADcsmRenderer, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime_sharingScene)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update(20);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update(20);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    //////////////////////////
    // new content offered, initiate content switch
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update(30);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // first request new content ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID2, 0));
    update(30);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // announce original content not needed at 1000 and start provider transition animation (if any) immediately
    // scene is still shown within this period
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // bit later new content should become ready
    m_dcsmHandler.contentReady(m_contentID2, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
    update(100);
    // original content state still rendered
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    // new content is fully ready because scene is already mapped (used by original content)
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // announce show at 1000, starting provider side transition (if any) immediately
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Shown, AnimationInformation{ 200, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID2, AnimationInformation{ 200, 1000 }));

    update(110);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    update(210);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    // as scene is already shown (used by original content) new content is in rendered state right away
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    ///////

    // actual scene state is kept rendered but scheduled state change came into effect and original content state reached its target
    update(1000);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
    update(1100);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
}

TEST_F(ADcsmRenderer, forcedStopOfferIsPropagatedAsEventAndMakesContentInvalid)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].notAvailableReported);

    // content invalidated, cannot set state
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmRenderer, forcedStopOfferedContentCanBecomeOfferedAgain)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].notAvailableReported);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Unavailable, m_displayId);

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, forcedStopOfferForUnknownContentIsIgnored)
{
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    update();
    EXPECT_FALSE(m_dcsmRendererContentStateTracker[m_contentID1].notAvailableReported);
}

TEST_F(ADcsmRenderer, propagatesFocusChangeRequestAsEvent)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    m_dcsmHandler.contentFocusRequest(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].focusRequested);
}

TEST_F(ADcsmRenderer, focusChangeRequestForUnknownContentIsIgnored)
{
    m_dcsmHandler.contentFocusRequest(m_contentID1);
    update();
    EXPECT_FALSE(m_dcsmRendererContentStateTracker[m_contentID1].focusRequested);
}

TEST_F(ADcsmRenderer, stopOfferRequestForUnknownContentIsAcceptedRightAway)
{
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
}

TEST_F(ADcsmRenderer, stopOfferRequestCanBeAccepted)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].stopOfferRequested);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    update(15);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Unavailable, _));
    update(100);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmRenderer, scheduleContentHideBeforeAcceptingStopOfferRequest)
{
    // show content
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update();
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update();
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request stop offer
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].stopOfferRequested);

    // schedule release of content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 20, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 20, 100 }));
    // and accept stop offer with same timing
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 20, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 20, 100 }));

    update(10);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    update(50);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // 2 scene state change requests were scheduled - one for hide request, one as result of accepted stop offer
    // the latter overrides the first
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Unavailable, _));
    update(100);
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Unavailable, m_displayId);
    update(110);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update(200);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, contentCanBeReofferedAfterStopOfferRequestAccepted)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].stopOfferRequested);

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));

    update(10);
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Unavailable, _));
    update(20);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Unavailable, m_displayId);
    update(21);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update(200);
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToSetSizeOfUnknownCategory)
{
    EXPECT_NE(StatusOK, m_dcsmRenderer.setCategorySize(Category{ 9999 }, SizeInfo{}, AnimationInformation{}));
}

TEST_F(ADcsmRenderer, sendsCategorySizeChangeToAllContentsAssociatedWithIt)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID3].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID2, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.setCategorySize(m_categoryID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID3, SizeInfo{ 666, 999 }, AnimationInformation{ 10, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.setCategorySize(m_categoryID2, SizeInfo{ 666, 999 }, AnimationInformation{ 10, 100 }));
}

TEST_F(ADcsmRenderer, sendsLatestCategorySizeToNewContentOfferedForItEvenIfNotReachedTheFinishTimeForTheSizeChange)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.setCategorySize(m_categoryID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));

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
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID3].lastState);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[contentID4].lastState);
}

TEST_F(ADcsmRenderer, failsToShowAfterReleasingContent)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));

    // attempt to show content now fails
    EXPECT_NE(StatusOK, m_dcsmRenderer.showContent(m_contentID1, { 0, 0 }));

    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Available, _));
    update(100);
}

TEST_F(ADcsmRenderer, canRequestReadyAgainAfterReleasingContent)
{
    // show content
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Rendered, _));
    update();
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Rendered, m_displayId);
    update();
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));
    update(10);

    // cannot request ready till content released at animation finish
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));

    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Available, _));
    update(100);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Available, m_displayId);
    update(110);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneMapping(m_sceneId, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, canGetContentReadyAndLinkedViaOB)
{
    // make consumer content ready first
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // request content to be linked ready
    const sceneId_t sceneToLink{ m_sceneId + 1 };
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneOffscreenBufferMapping(sceneToLink, m_displayId, 16, 32, m_sceneId, 123));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetContentReadyAndLinkedViaOBToItself)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID1, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetUnknownContentReadyAndLinkedViaOB)
{
    // make consumer content ready first
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // content1 is unknown
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Invalid, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetContentReadyAndLinkedViaOBToConsumerContentWhichIsNotReady)
{
    // make consumer content available only
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // request content to be linked ready
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetContentReadyAndLinkedViaOBWithZeroDimension)
{
    // make consumer content ready first
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // request content to be linked ready
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 0, 32, m_contentID2, 123, 0));
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 0, m_contentID2, 123, 0));
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 0, 0, m_contentID2, 123, 0));

    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, confidenceTest_canGetContentReadyAndLinkedViaOB_thenRemappedToDisplay_thenRemappedToOBAndLinkAgain)
{
    ////// 1. make ready and OB link
    // make consumer content ready first
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // request content to be linked ready
    const sceneId_t sceneToLink{ m_sceneId + 1 };
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneOffscreenBufferMapping(sceneToLink, m_displayId, 16, 32, m_sceneId, 123));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);


    ////// 2. release and make ready without OB
    // release first
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, { 0, 0 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Available, _));
    update();
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Available, m_displayId);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(sceneToLink, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);


    ////// 3. make ready and OB link
    // release first
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, { 0, 0 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Available, _));
    update();
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Available, m_displayId);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // consumer content is already ready from step 1
    // request content to be linked ready
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneOffscreenBufferMapping(sceneToLink, m_displayId, 16, 32, m_sceneId, 123));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetContentReadyIfItIsAlreadyReadyUsingOBLinking)
{
    // make consumer content ready first
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // request content to be linked ready
    const sceneId_t sceneToLink{ m_sceneId + 1 };
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneOffscreenBufferMapping(sceneToLink, m_displayId, 16, 32, m_sceneId, 123));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // fails request to make ready again
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetContentReadyAndLinkedViaOBIfItIsAlreadyReadyWithoutUsingOBLinking)
{
    // make content ready without OB
    const sceneId_t sceneToLink{ m_sceneId + 1 };
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(sceneToLink, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // make consumer content ready
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // fails request to make ready using OB
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetContentReadyAndLinkedViaOBIfItIsAlreadyReadyUsingDifferentOBParameters)
{
    // make consumer content ready first
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // request content to be linked ready
    const sceneId_t sceneToLink{ m_sceneId + 1 };
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneOffscreenBufferMapping(sceneToLink, m_displayId, 16, 32, m_sceneId, 123));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // fails request to make ready using different parameters
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 999, 32, m_contentID2, 123, 0));
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 999, m_contentID2, 123, 0));
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID3, 123, 0));
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 999, 0));
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, failsToGetContentReadyAndLinkedViaOBIfItIsAlreadyShown)
{
    // make consumer content ready first
    makeDcsmContentReady(m_contentID2, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID2].lastState);

    // request content to be linked ready
    const sceneId_t sceneToLink{ m_sceneId + 1 };
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    EXPECT_CALL(m_displayManagerMock, setSceneOffscreenBufferMapping(sceneToLink, m_displayId, 16, 32, m_sceneId, 123));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneToLink });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // show content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, { 0, 0 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(sceneToLink, ramses_display_manager::SceneState::Rendered, _));
    update();
    m_displayManagerHandler.sceneStateChanged(sceneToLink, ramses_display_manager::SceneState::Rendered, m_displayId);
    update();
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // fails request to make ready again
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReadyAndLinkedViaOffscreenBuffer(m_contentID1, 16, 32, m_contentID2, 123, 0));
    update();
    EXPECT_EQ(ContentState::Shown, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeReadyConfirmedFromDcsmProvider)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    // no call to DM to change state because it is already only available
    update(10);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    // stays available
    update(100);
    update(1000);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeReadyConfirmedFromDM)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // dcsm ready
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(m_sceneId, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate scene just became ready from DM, nothing expected
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // no change reported but internally will set scene state to available because it got ready from DM
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Available, _));
    update(10);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Available, m_displayId);
    update(10);

    // stays available
    update(100);
    update(1000);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeDcsmReadyConfirmedAndThenMakeReadyAgain)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    // no call to DM to change state because it is already only available
    update(10);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    update(100);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(m_sceneId, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeDMReadyConfirmedAndThenMakeReadyAgain)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // dcsm ready
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(m_sceneId, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate scene just became ready from DM, nothing expected
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // no change reported but internally will set scene state to available because it got ready from DM
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Available, _));
    update(10);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Available, m_displayId);
    update(10);

    update(100);
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(m_sceneId, m_displayId, 0));
    EXPECT_CALL(m_displayManagerMock, setSceneState(m_sceneId, ramses_display_manager::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
    m_displayManagerHandler.sceneStateChanged(m_sceneId, ramses_display_manager::SceneState::Ready, m_displayId);
    update();
    EXPECT_EQ(ContentState::Ready, m_dcsmRendererContentStateTracker[m_contentID1].lastState);
}

TEST_F(ADcsmRenderer, willNotTryToMapSceneIfReadyRequestedButForcedStopOfferedBeforeProviderReady)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate force stop offer
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].notAvailableReported);

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    update(10);
    EXPECT_EQ(ContentState::Invalid, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    update(100);
    EXPECT_EQ(ContentState::Invalid, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready fails as content not available
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmRenderer, willNotTryToMapSceneIfReadyRequestedButAcceptedStopOfferBeforeProviderReady)
{
    // offered
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    EXPECT_EQ(ContentState::Available, m_dcsmRendererContentStateTracker[m_contentID1].lastState);

    // simulate request stop offer
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    update();
    EXPECT_TRUE(m_dcsmRendererContentStateTracker[m_contentID1].stopOfferRequested);
    // accept right away
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ m_sceneId });

    update(100);

    // request ready fails as content not available
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}
