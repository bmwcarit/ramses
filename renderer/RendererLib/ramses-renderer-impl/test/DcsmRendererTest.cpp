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
#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "DcsmMetadataUpdateImpl.h"
#include "DcsmConsumerMock.h"
#include "DisplayManagerMock.h"
#include <memory.h>

using namespace ramses;
using namespace testing;

class DcsmRendererEventHandlerMock : public IDcsmRendererEventHandler
{
public:
    MOCK_METHOD2(contentAvailable, void(ContentID, Category));
    MOCK_METHOD2(contentReady, void(ContentID, DcsmRendererEventResult));
    MOCK_METHOD1(contentShown, void(ContentID));
    MOCK_METHOD1(contentHidden, void(ContentID));
    MOCK_METHOD1(contentReleased, void(ContentID));
    MOCK_METHOD1(contentFocusRequested, void(ContentID));
    MOCK_METHOD1(contentStopOfferRequested, void(ContentID));
    MOCK_METHOD1(contentNotAvailable, void(ContentID));
    MOCK_METHOD2(contentMetadataUpdated, void(ContentID, const DcsmMetadataUpdate&));
    MOCK_METHOD4(offscreenBufferLinked, void(ramses::displayBufferId_t, ContentID, ramses::dataConsumerId_t, bool));
    MOCK_METHOD5(dataLinked, void(ContentID, ramses::dataProviderId_t, ContentID, ramses::dataConsumerId_t, bool));
};

class ADcsmRenderer : public Test
{
public:
    ADcsmRenderer()
        : m_displayManagerMock(*new StrictMock<DisplayManagerMock>)
        , m_dcsmRenderer(DcsmRendererConfig{ { {m_categoryID1, m_categoryInfo1}, {m_categoryID2, m_categoryInfo2} } }, m_dcsmConsumerMock, std::unique_ptr<ramses_internal::IDisplayManager>(&m_displayManagerMock))
        , m_dcsmHandler(m_dcsmRenderer)
        , m_displayManagerHandler(m_dcsmRenderer)
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
        EXPECT_CALL(m_displayManagerMock, dispatchAndFlush(_, nullptr));
        EXPECT_EQ(StatusOK, m_dcsmRenderer.update(timeStamp, m_eventHandlerMock, nullptr));
        m_lastUpdateTS = timeStamp;
    }

    void makeDcsmContentReady(ContentID contentID, Category categoryID, sceneId_t sceneId = SceneId1)
    {
        // offered
        EXPECT_CALL(m_eventHandlerMock, contentAvailable(contentID, categoryID));
        EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(contentID, m_categoryInfo1.size));
        m_dcsmHandler.contentOffered(contentID, categoryID);
        update(m_lastUpdateTS);

        // request ready
        EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(contentID, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
        EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(contentID, 0));
        update(m_lastUpdateTS);

        EXPECT_CALL(m_displayManagerMock, setSceneMapping(sceneId, m_displayId));
        EXPECT_CALL(m_displayManagerMock, setSceneState(sceneId, ramses_internal::SceneState::Ready, _));
        m_dcsmHandler.contentReady(contentID, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ sceneId.getValue() });
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
    const DcsmRendererConfig::CategoryInfo m_categoryInfo1{ {16,16}, m_displayId };
    const DcsmRendererConfig::CategoryInfo m_categoryInfo2{ {32,32}, m_displayId };

    StrictMock<DcsmConsumerMock> m_dcsmConsumerMock;
    StrictMock<DisplayManagerMock>& m_displayManagerMock;
    DcsmRendererImpl m_dcsmRenderer;
    IDcsmConsumerEventHandler& m_dcsmHandler;
    ramses_internal::IEventHandler& m_displayManagerHandler;
    StrictMock<DcsmRendererEventHandlerMock> m_eventHandlerMock;
    uint64_t m_lastUpdateTS = 0;
};

constexpr sceneId_t ADcsmRenderer::SceneId1;
constexpr sceneId_t ADcsmRenderer::SceneId2;


TEST_F(ADcsmRenderer, handlesDcsmOffered)
{
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
}

TEST_F(ADcsmRenderer, doesNotReportOfferedWhenDcsmOfferedContentForUnregisteredCategory)
{
    m_dcsmHandler.contentOffered(m_contentID1, Category{ 666 });
    update();
}

TEST_F(ADcsmRenderer, handlesDcsmOfferedForMultipleContentsAndCategories)
{
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    // content3 uses another category
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID2));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();
}

TEST_F(ADcsmRenderer, requestsContentReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
}

TEST_F(ADcsmRenderer, handlesContentAndSceneReady)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();
}

TEST_F(ADcsmRenderer, handlesContentAndSceneReadyForMultipleContentsSharingSingleScene)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    makeDcsmContentReady(m_contentID2, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmRendererEventResult::OK));
    update();

    // now request content3 ready
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID2));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID3, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID3, 0));
    m_dcsmHandler.contentReady(m_contentID3, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID3, DcsmRendererEventResult::OK));
    update();
}

TEST_F(ADcsmRenderer, failsUpdateIfTimeNotContinuous)
{
    EXPECT_CALL(m_dcsmConsumerMock, dispatchEvents(_)).Times(4);
    EXPECT_CALL(m_displayManagerMock, dispatchAndFlush(_, nullptr)).Times(4);

    EXPECT_EQ(StatusOK, m_dcsmRenderer.update(0, m_eventHandlerMock, nullptr));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.update(10, m_eventHandlerMock, nullptr));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.update(20, m_eventHandlerMock, nullptr));
    EXPECT_NE(StatusOK, m_dcsmRenderer.update(19, m_eventHandlerMock, nullptr));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.update(100, m_eventHandlerMock, nullptr));
    EXPECT_NE(StatusOK, m_dcsmRenderer.update(99, m_eventHandlerMock, nullptr));
}

TEST_F(ADcsmRenderer, canGetContentRenderedAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // start time not reached -> scene not shown -> combined state still ready
    update(10);

    // scene show triggered but not confirmed yet
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update(20);
    update(30);

    // now report scene rendered
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(40);

    // stays rendered...
    update(50);
    update(60);
}

TEST_F(ADcsmRenderer, canGetContentRenderedAndThenHiddenAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update(20);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(20);

    // announce hide at another time
    const AnimationInformation timing2{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing2));

    update(30);
    // state still rendered because scene is still rendered...

    //...till finish time
    update(100);
    update(120);

    // at finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    update(150);

    // only after scene really hidden content state changes
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentHidden(m_contentID1));
    update(200);
}

TEST_F(ADcsmRenderer, hideContentAtGivenTimeAndThenOverrideWithEarlierTiming)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // announce show at a time
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, { 0, 0 }));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update(0);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(0);

    // announce hide at a time
    const AnimationInformation timing{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing));

    update(110);
    // still rendering to allow fade out animation

    // now request hiding at earlier point
    const AnimationInformation timing2{ 100, 120 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing2));

    // still no change
    update(115);

    // at updated finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    update(120);

    // only after scene really hidden content state changes
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentHidden(m_contentID1));
    update(121);
}

TEST_F(ADcsmRenderer, hideContentAtGivenTimeAndThenOverrideWithLaterTiming)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // announce show at a time
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, { 0, 0 }));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update(0);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(0);

    // announce hide at a time
    const AnimationInformation timing{ 100, 150 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing));

    update(110);
    // still rendering to allow fade out animation

    // now request hiding at later point
    const AnimationInformation timing2{ 100, 180 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, timing2));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.hideContent(m_contentID1, timing2));

    // still no change
    update(115);
    update(160);

    // at updated finish time scene is requested to be hidden(ready)
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    update(180);

    // only after scene really hidden content state changes
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentHidden(m_contentID1));
    update(181);
}

TEST_F(ADcsmRenderer, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // content state unchanged till scene really shown
    update(10);

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update(20);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(20);

    //////////////////////////
    // new content offered, initiate content switch
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update(30);

    // first request new content ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID2, 0));
    update(30);

    // announce original content not needed at 1000 and start provider transition animation (if any) immediately
    // scene is still shown within this period
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 1000 }));

    // bit later new content should become ready
    const sceneId_t otherScene{ SceneId1.getValue() + 10 };
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(otherScene, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(otherScene, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID2, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ otherScene.getValue() });
    update(100);
    // new content is not combined ready because its scene is not ready
    // handle new scene ready
    m_displayManagerHandler.sceneStateChanged(otherScene, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmRendererEventResult::OK));
    update(100);

    // announce show at 1000, starting provider side transition (if any) immediately
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Shown, AnimationInformation{ 0, 1000 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(otherScene, ramses_internal::SceneState::Rendered, _));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID2, AnimationInformation{ 0, 1000 }));
    update(110);
    // simulate scene shown
    m_displayManagerHandler.sceneStateChanged(otherScene, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID2));
    update(120);

    ///////

    // now reaching time 1000 original content's scene can be safely hidden - desired state was offered
    // content states already reached their final states
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Available, _));
    update(1000);

    // only after scene 'downgraded' to available original content's state changes
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Available, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReleased(m_contentID1));
    update(1100);
}

TEST_F(ADcsmRenderer, canGetContentRenderedAndThenSwitchToAnotherContentAtGivenTime_sharingScene)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // announce show at a time
    const AnimationInformation timing{ 20, 50 };
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, timing));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, timing));

    // scene shown
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update(20);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update(20);

    //////////////////////////
    // new content offered, initiate content switch
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    update(30);

    // first request new content ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID2, 0));
    update(30);

    // announce original content not needed at 1000 and start provider transition animation (if any) immediately
    // scene is still shown within this period
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 1000 }));

    // bit later new content should become ready
    m_dcsmHandler.contentReady(m_contentID2, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmRendererEventResult::OK));
    update(100);
    // original content state still rendered
    // new content is fully ready because scene is already mapped (used by original content)

    // announce show at 1000, starting provider side transition (if any) immediately
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID2, EDcsmState::Shown, AnimationInformation{ 200, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID2, AnimationInformation{ 200, 1000 }));

    update(110);

    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID2));
    update(210);
    // as scene is already shown (used by original content) new content is in rendered state right away

    ///////

    // actual scene state is kept rendered but scheduled state change came into effect and original content state reached its target
    EXPECT_CALL(m_eventHandlerMock, contentReleased(m_contentID1));
    update(1000);
    update(1100);
}

TEST_F(ADcsmRenderer, forcedStopOfferIsPropagatedAsEventAndMakesContentInvalid)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // content invalidated, cannot set state
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmRenderer, forcedStopOfferedContentCanBecomeOfferedAgain)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Unavailable, m_displayId);

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();
}

TEST_F(ADcsmRenderer, forcedStopOfferForUnknownContentIsIgnored)
{
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    update();
}

TEST_F(ADcsmRenderer, propagatesFocusChangeRequestAsEvent)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    m_dcsmHandler.contentFocusRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentFocusRequested(m_contentID1));
    update();
}

TEST_F(ADcsmRenderer, focusChangeRequestForUnknownContentIsIgnored)
{
    m_dcsmHandler.contentFocusRequest(m_contentID1);
    update();
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
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    update(15);

    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Unavailable, _));
    update(100);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmRenderer, scheduleContentHideBeforeAcceptingStopOfferRequest)
{
    // show content
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update();
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update();

    // request stop offer
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    // schedule release of content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 20, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 20, 100 }));
    // and accept stop offer with same timing
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 20, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 20, 100 }));

    update(10);
    update(50);

    // 2 scene state change requests were scheduled - one for hide request, one as result of accepted stop offer
    // the latter overrides the first
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Unavailable, _));
    update(100);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Unavailable, m_displayId);
    update(110);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update(200);
}

TEST_F(ADcsmRenderer, contentCanBeReofferedAfterStopOfferRequestAccepted)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();

    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 10, 20 }));

    update(10);
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Unavailable, _));
    update(20);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Unavailable, m_displayId);
    update(21);

    // content is now unknown and cannot be used
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));

    // offer again and start new cycle
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    // make scene ready to reach ready state again
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update(200);
}

TEST_F(ADcsmRenderer, failsToSetSizeOfUnknownCategory)
{
    EXPECT_NE(StatusOK, m_dcsmRenderer.setCategorySize(Category{ 9999 }, SizeInfo{ 1, 1 }, AnimationInformation{}));
}

TEST_F(ADcsmRenderer, sendsCategorySizeChangeToAllContentsAssociatedWithIt)
{
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID2));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, m_categoryInfo2.size));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID2);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID2, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.setCategorySize(m_categoryID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID3, SizeInfo{ 666, 999 }, AnimationInformation{ 10, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.setCategorySize(m_categoryID2, SizeInfo{ 666, 999 }, AnimationInformation{ 10, 100 }));
}

TEST_F(ADcsmRenderer, sendsLatestCategorySizeToNewContentOfferedForItEvenIfNotReachedTheFinishTimeForTheSizeChange)
{
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    EXPECT_CALL(m_dcsmConsumerMock, contentSizeChange(m_contentID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.setCategorySize(m_categoryID1, SizeInfo{ 333, 111 }, AnimationInformation{ 500, 1000 }));

    // before start time
    update(10);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, SizeInfo{ 333, 111 }));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);

    // between start and end time
    update(700);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID3, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID3, SizeInfo{ 333, 111 }));
    m_dcsmHandler.contentOffered(m_contentID3, m_categoryID1);

    // after end time
    const ContentID contentID4{ m_contentID3.getValue() + 1 };
    update(1100);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(contentID4, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(contentID4, SizeInfo{ 333, 111 }));
    m_dcsmHandler.contentOffered(contentID4, m_categoryID1);

    update(2000);
}

TEST_F(ADcsmRenderer, failsToShowAfterReleasingContent)
{
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    // handle scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));

    // attempt to show content now fails
    EXPECT_NE(StatusOK, m_dcsmRenderer.showContent(m_contentID1, { 0, 0 }));

    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Available, _));
    update(100);
}

TEST_F(ADcsmRenderer, canRequestReadyAgainAfterReleasingContent)
{
    // show content
    makeDcsmContentReady(m_contentID1, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Shown, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.showContent(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Rendered, _));
    update();
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Rendered, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentShown(m_contentID1));
    update();

    // release content to some time point
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 100 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 100 }));
    update(10);

    // cannot request ready till content released at animation finish
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));

    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Available, _));
    update(100);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Available, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReleased(m_contentID1));
    update(110);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update(110);

    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update(120);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeReadyConfirmedFromDcsmProvider)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    // no call to DM to change state because it is already only available
    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    // stays available
    update(100);
    update(1000);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeReadyConfirmedFromDM)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();

    // dcsm ready
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate scene just became ready from DM, nothing expected
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    update();

    // no change reported but internally will set scene state to available because it got ready from DM
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Available, _));
    update(10);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Available, m_displayId);
    update(10);

    // stays available
    update(100);
    update(1000);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeDcsmReadyConfirmedAndThenMakeReadyAgain)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    // no call to DM to change state because it is already only available
    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    update(100);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update(100);
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update(100);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update(100);
}

TEST_F(ADcsmRenderer, canRequestReadyButReleaseBeforeDMReadyConfirmedAndThenMakeReadyAgain)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();

    // dcsm ready
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update();

    // release content
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 10 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.releaseContent(m_contentID1, AnimationInformation{ 0, 10 }));
    update();

    // simulate scene just became ready from DM, nothing expected
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    update();

    // no change reported but internally will set scene state to available because it got ready from DM
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Available, _));
    update(10);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Available, m_displayId);
    update(10);

    update(100);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update(100);
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update(100);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update(100);
}

TEST_F(ADcsmRenderer, willNotTryToMapSceneIfReadyRequestedButForcedStopOfferedBeforeProviderReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();

    // simulate force stop offer
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    update(100);

    // request ready fails as content not available
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmRenderer, willNotTryToMapSceneIfReadyRequestedButAcceptedStopOfferBeforeProviderReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();

    // simulate request stop offer
    m_dcsmHandler.contentStopOfferRequest(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentStopOfferRequested(m_contentID1));
    update();
    // accept right away
    EXPECT_CALL(m_dcsmConsumerMock, acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.acceptStopOffer(m_contentID1, AnimationInformation{ 0, 0 }));
    update();

    // simulate Dcsm just became ready, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    update(10);

    // simulate Dcsm became ready again, nothing expected
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    update(100);

    // request ready fails as content not available
    EXPECT_NE(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
}

TEST_F(ADcsmRenderer, handlesTimeOutWhenRequestReadyButNotReachedDcsmReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update(100); // set initial time

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 50)); // give 50 time units for timeout
    update(111);

    // dcsm ready does not come
    update(121);
    update(131);
    update(141);

    // will explicitly request state back to DCSM assigned
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    // will not set anything on DM side because nothing was requested from DM yet
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Available, _)).Times(0);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::TimedOut));
    update(151);

    // simulate race - dcsm ready arrives right after timeout before dcsm set state back to assigned
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    // nothing expected

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update(200);
    // handle dcsm ready
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update(200);
    // handle new scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update(200);
}

TEST_F(ADcsmRenderer, handlesTimeOutWhenRequestReadyButNotReachedDMReady)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update(100); // set initial time

    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 50)); // give 50 time units for timeout
    update(111);

    // handle dcsm ready
    update(121);
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });

    // DM ready does not come
    update(131);
    update(141);

    // will explicitly request state back to DCSM assigned
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Assigned, AnimationInformation{ 0, 0 }));
    // will set scene state back to available on DM side
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Available, _));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::TimedOut));
    update(151);

    // simulate race - DM ready arrives right after timeout before DM set state back to available
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    // nothing expected
    update(200);
    // due to race DM actually reached ready state and will change state back to available which was requested when timed out
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Available, m_displayId);
    // nothing expected
    update(200);

    // request ready again
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update(200);
    // handle dcsm ready
    EXPECT_CALL(m_displayManagerMock, setSceneMapping(SceneId1, m_displayId));
    EXPECT_CALL(m_displayManagerMock, setSceneState(SceneId1, ramses_internal::SceneState::Ready, _));
    m_dcsmHandler.contentReady(m_contentID1, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{ SceneId1.getValue() });
    update(200);
    // handle new scene ready
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update(200);
}

TEST_F(ADcsmRenderer, failsToDataLinkUnknownProviderContent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmRenderer.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmRenderer, failsToDataLinkUnknownConsumerContent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmRenderer.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmRenderer, failsToDataLinkProviderContentWithUnknownScene)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmRenderer.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmRenderer, failsToDataLinkConsumerContentWithUnknownScene)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID2, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID2, m_categoryID1);

    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID2, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmRenderer.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmRenderer, requestsDataLinkFromDisplayManager)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    m_displayManagerHandler.sceneStateChanged(SceneId2, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmRendererEventResult::OK));
    update();

    // contents share same scene, so provider scene is same as consumer scene
    EXPECT_CALL(m_displayManagerMock, linkData(SceneId1, providerId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.linkData(m_contentID1, providerId, m_contentID2, consumerId));
}

TEST_F(ADcsmRenderer, reportsDataLinkedWithCorrectContents)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    m_displayManagerHandler.sceneStateChanged(SceneId2, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmRendererEventResult::OK));
    update();

    EXPECT_CALL(m_eventHandlerMock, dataLinked(m_contentID1, providerId, m_contentID2, consumerId, true));
    m_displayManagerHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(m_contentID1, providerId, m_contentID2, consumerId, false));
    m_displayManagerHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);
}

TEST_F(ADcsmRenderer, reportsDataLinkedWithZeroContentIfContentGoneBeforeEvent)
{
    constexpr dataProviderId_t providerId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    makeDcsmContentReady(m_contentID2, m_categoryID1, SceneId2);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    m_displayManagerHandler.sceneStateChanged(SceneId2, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID2, DcsmRendererEventResult::OK));
    update();

    // link data
    EXPECT_CALL(m_displayManagerMock, linkData(SceneId1, providerId, SceneId2, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.linkData(m_contentID1, providerId, m_contentID2, consumerId));

    // remove provider content
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, m_contentID2, consumerId, true));
    m_displayManagerHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, m_contentID2, consumerId, false));
    m_displayManagerHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);

    // remove consumer content
    m_dcsmHandler.forceContentOfferStopped(m_contentID2);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID2));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, ContentID{ 0 }, consumerId, true));
    m_displayManagerHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, dataLinked(ContentID{ 0 }, providerId, ContentID{ 0 }, consumerId, false));
    m_displayManagerHandler.dataLinked(SceneId1, providerId, SceneId2, consumerId, false);
}

TEST_F(ADcsmRenderer, handlesDcsmMetadataUpdate)
{
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);

    DcsmMetadataUpdate mdp(*new DcsmMetadataUpdateImpl);
    ramses_internal::DcsmMetadata md;
    md.setPreviewDescription(U"foobar");
    mdp.impl.setMetadata(md);
    m_dcsmHandler.contentMetadataUpdated(m_contentID1, mdp);

    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_eventHandlerMock, contentMetadataUpdated(m_contentID1, _)).
        WillOnce(Invoke([&](auto, auto& prov) { EXPECT_EQ(md, prov.impl.getMetadata()); }));
    update();
}

TEST_F(ADcsmRenderer, ignoresDcsmMetadataUpdateForUnknownContent)
{
    DcsmMetadataUpdate mdp(*new DcsmMetadataUpdateImpl);
    ramses_internal::DcsmMetadata md;
    md.setPreviewDescription(U"foobar");
    mdp.impl.setMetadata(md);
    m_dcsmHandler.contentMetadataUpdated(m_contentID1, mdp);

    update();
}

TEST_F(ADcsmRenderer, canAssignContentToDisplayBuffer)
{
    // in order to assign content to buffer, it must be at least DCSM ready
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_displayManagerMock, setSceneDisplayBufferAssignment(SceneId1, displayBuffer, 11));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.assignContentToDisplayBuffer(m_contentID1, displayBuffer, 11));
}

TEST_F(ADcsmRenderer, failsToAssignUnknownContentToDisplayBuffer)
{
    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_NE(StatusOK, m_dcsmRenderer.assignContentToDisplayBuffer(ContentID{ 666u }, displayBuffer, 11));
}

TEST_F(ADcsmRenderer, failsToAssignNotReadyContentToDisplayBuffer)
{
    // offered
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    update();
    // request ready
    EXPECT_CALL(m_dcsmConsumerMock, contentStateChange(m_contentID1, EDcsmState::Ready, AnimationInformation{ 0, 0 }));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.requestContentReady(m_contentID1, 0));
    update();
    // content not reported as ready yet, therefore its scene is not known and mapping info was not set yet

    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_NE(StatusOK, m_dcsmRenderer.assignContentToDisplayBuffer(ContentID{ 666u }, displayBuffer, 11));
}

TEST_F(ADcsmRenderer, failsToAssignContentToDisplayBufferIfInternalRequestFails)
{
    // in order to assign content to buffer, it must be at least DCSM ready
    makeDcsmContentReady(m_contentID1, m_categoryID1);

    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_displayManagerMock, setSceneDisplayBufferAssignment(SceneId1, displayBuffer, 11)).WillOnce(Return(false));
    EXPECT_NE(StatusOK, m_dcsmRenderer.assignContentToDisplayBuffer(m_contentID1, displayBuffer, 11));
}

TEST_F(ADcsmRenderer, failsToOffscreenBufferLinkUnknownConsumerContent)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };
    EXPECT_NE(StatusOK, m_dcsmRenderer.linkOffscreenBuffer(obId, m_contentID1, consumerId));
}

TEST_F(ADcsmRenderer, failsToOffscreenBufferLinkConsumerContentWithUnknownScene)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    EXPECT_CALL(m_dcsmConsumerMock, assignContentToConsumer(m_contentID1, m_categoryInfo1.size));
    m_dcsmHandler.contentOffered(m_contentID1, m_categoryID1);
    EXPECT_CALL(m_eventHandlerMock, contentAvailable(m_contentID1, m_categoryID1));
    update();

    EXPECT_NE(StatusOK, m_dcsmRenderer.linkOffscreenBuffer(obId, m_contentID1, consumerId));
}

TEST_F(ADcsmRenderer, requestsOffscreenBufferLinkFromDisplayManager)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    EXPECT_CALL(m_displayManagerMock, linkOffscreenBuffer(obId, SceneId1, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.linkOffscreenBuffer(obId, m_contentID1, consumerId));
}

TEST_F(ADcsmRenderer, reportsOffscreenBufferLinkedWithCorrectContents)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, m_contentID1, consumerId, true));
    m_displayManagerHandler.offscreenBufferLinked(obId, SceneId1, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, m_contentID1, consumerId, false));
    m_displayManagerHandler.offscreenBufferLinked(obId, SceneId1, consumerId, false);
}

TEST_F(ADcsmRenderer, reportsOffscreenBufferLinkedWithZeroContentIfContentGoneBeforeEvent)
{
    constexpr displayBufferId_t obId{ 12 };
    constexpr dataConsumerId_t consumerId{ 13 };

    makeDcsmContentReady(m_contentID1, m_categoryID1, SceneId1);
    m_displayManagerHandler.sceneStateChanged(SceneId1, ramses_internal::SceneState::Ready, m_displayId);
    EXPECT_CALL(m_eventHandlerMock, contentReady(m_contentID1, DcsmRendererEventResult::OK));
    update();

    // link data
    EXPECT_CALL(m_displayManagerMock, linkOffscreenBuffer(obId, SceneId1, consumerId));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.linkOffscreenBuffer(obId, m_contentID1, consumerId));

    // remove provider content
    m_dcsmHandler.forceContentOfferStopped(m_contentID1);
    EXPECT_CALL(m_eventHandlerMock, contentNotAvailable(m_contentID1));
    update();

    // report data linked
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, ContentID{ 0 }, consumerId, true));
    m_displayManagerHandler.offscreenBufferLinked(obId, SceneId1, consumerId, true);

    // failed case
    EXPECT_CALL(m_eventHandlerMock, offscreenBufferLinked(obId, ContentID{ 0 }, consumerId, false));
    m_displayManagerHandler.offscreenBufferLinked(obId, SceneId1, consumerId, false);
}

TEST_F(ADcsmRenderer, canSetClearColorOfDisplayBuffer)
{
    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_displayManagerMock, setDisplayBufferClearColor(displayBuffer, 1, 2, 3, 4));
    EXPECT_EQ(StatusOK, m_dcsmRenderer.setDisplayBufferClearColor(displayBuffer, 1, 2, 3, 4));
}

TEST_F(ADcsmRenderer, failsToSetClearColorIfInternalRequestFails)
{
    const displayBufferId_t displayBuffer{ 123u };
    EXPECT_CALL(m_displayManagerMock, setDisplayBufferClearColor(displayBuffer, 1, 2, 3, 4)).WillOnce(Return(false));
    EXPECT_NE(StatusOK, m_dcsmRenderer.setDisplayBufferClearColor(displayBuffer, 1, 2, 3, 4));
}
