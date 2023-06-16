//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererMate.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "RendererLib/RendererCommands.h"
#include "RamsesRendererImpl.h"
#include "Utils/ThreadBarrier.h"
#include "RendererCommandVisitorMock.h"
#include "PlatformFactoryMock.h"

using namespace ramses_internal;
using namespace testing;

class ARendererMate : public Test
{
public:
    ARendererMate()
        : renderer(*ramsesFramework.createRenderer(ramses::RendererConfig()))
        , rendererMate(renderer.m_impl, ramsesFramework.m_impl)
        , rendererMateRendererEventHandler(rendererMate)
        , rendererMateEventHandler(rendererMate)
    {
        renderer.m_impl.getDisplayDispatcher().injectPlatformFactory(std::make_unique<ramses_internal::PlatformFactoryNiceMock>());

        displayId = renderer.createDisplay({});
        rendererMate.setSceneMapping(sceneId, displayId);
        StrictMock<RendererCommandVisitorMock> visitor;
        EXPECT_CALL(visitor, createDisplayContext(_, _, _));
        visitor.visit(renderer.m_impl.getPendingCommands());
        clearCommands();
    }

protected:
    void expectLogConfirmationCommand()
    {
        StrictMock<RendererCommandVisitorMock> visitor;
        EXPECT_CALL(visitor, handleConfirmationEcho(DisplayHandle{ displayId.getValue() }, _));
        visitor.visit(renderer.m_impl.getPendingCommands());
        clearCommands();
    }

    void clearCommands()
    {
        const_cast<ramses_internal::RendererCommands&>(renderer.m_impl.getPendingCommands()).clear();
    }

    void publishAndExpectToGetToState(ramses::RendererSceneState state, bool expectConfirmation = false)
    {
        assert(state != ramses::RendererSceneState::Available);
        EXPECT_FALSE(isSceneShown());

        rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Available);
        if (state != ramses::RendererSceneState::Unavailable)
            rendererMateEventHandler.sceneStateChanged(sceneId, state);

        EXPECT_EQ(state, rendererMate.getLastReportedSceneState(sceneId));
        if (expectConfirmation)
            expectLogConfirmationCommand();
        clearCommands();
    }

    [[nodiscard]] bool isSceneShown() const
    {
        return rendererMate.getLastReportedSceneState(sceneId) == ramses::RendererSceneState::Rendered;
    }

private:
    ramses::RamsesFramework ramsesFramework{ ramses::RamsesFrameworkConfig{ramses::EFeatureLevel_Latest} };
    ramses::RamsesRenderer& renderer; // restrict access to renderer, acts only as dummy for RendererMate and to create (dummy) displays

protected:
    ramses::RendererMate rendererMate;
    ramses::IRendererEventHandler& rendererMateRendererEventHandler;
    ramses::IRendererSceneControlEventHandler& rendererMateEventHandler;

    const ramses::sceneId_t sceneId{ 33 };
    ramses::displayId_t displayId;
    const ramses::displayBufferId_t offscreenBufferId{ 2 };
};

TEST_F(ARendererMate, willShowSceneWhenPublished)
{
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    publishAndExpectToGetToState(ramses::RendererSceneState::Rendered);
}

TEST_F(ARendererMate, willShowSceneWhenPublishedAndLogsConfirmation)
{
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered, "dummy msg");
    publishAndExpectToGetToState(ramses::RendererSceneState::Rendered, true);
}

TEST_F(ARendererMate, willLogConfirmationEvenIfSceneAlreadyRendered)
{
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered, ""); // no confirmation
    publishAndExpectToGetToState(ramses::RendererSceneState::Rendered);

    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered, "confirmation");
    expectLogConfirmationCommand();
}

TEST_F(ARendererMate, reportsCorrectSceneShowState)
{
    // show first time
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    publishAndExpectToGetToState(ramses::RendererSceneState::Rendered);

    // explicitly hide
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Ready);
    EXPECT_TRUE(isSceneShown()); // hide command not processed yet
    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Ready);
    EXPECT_FALSE(isSceneShown());

    // show again
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    EXPECT_FALSE(isSceneShown()); // show command not processed yet
    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Rendered);
    EXPECT_TRUE(isSceneShown());

    // unexpected unpublish!!
    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Unavailable);
    EXPECT_FALSE(isSceneShown());
}

TEST_F(ARendererMate, continuesToShowSceneAndLogsConfirmationAfterReconnect)
{
    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Available);
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered, "scene is shown now");

    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Available);

    // simulate disconnect, renderer framework sends unpublish for all its scenes
    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Unavailable);

    publishAndExpectToGetToState(ramses::RendererSceneState::Rendered, true); // expect confirmation
}

TEST_F(ARendererMate, canBeCalledFromAnotherThread)
{
    // this test simulates 2 threads executing commands (e.g. ramsh commands)
    // and a thread dispatching and checking some states (e.g. standalone renderer)

    ThreadBarrier initBarrier(3);
    ThreadBarrier finishedBarrier(3);

    constexpr ramses::sceneId_t sceneId1{ 10123 };
    constexpr ramses::sceneId_t sceneId2{ 10124 };
    rendererMateEventHandler.sceneStateChanged(sceneId1, ramses::RendererSceneState::Available);
    rendererMateEventHandler.sceneStateChanged(sceneId2, ramses::RendererSceneState::Available);

    auto executeMethods = [&](ramses::sceneId_t sId)
    {
        initBarrier.wait();
        EXPECT_TRUE(rendererMate.setSceneState(sId, ramses::RendererSceneState::Available));
        EXPECT_TRUE(rendererMate.setSceneMapping(sId, displayId));
        EXPECT_TRUE(rendererMate.setSceneDisplayBufferAssignment(sId, offscreenBufferId));
        EXPECT_TRUE(rendererMate.setSceneState(sId, ramses::RendererSceneState::Ready));
        rendererMate.linkOffscreenBuffer(offscreenBufferId, sId, ramses::dataConsumerId_t{ 123 });
        rendererMate.linkData(sId, ramses::dataProviderId_t{ 123 }, ramses::sceneId_t{ sId.getValue() + 10 }, ramses::dataConsumerId_t{ 123 });
        rendererMate.processConfirmationEchoCommand(displayId, "foo");
        finishedBarrier.wait();
    };

    std::thread t1(executeMethods, sceneId1);
    std::thread t2(executeMethods, sceneId2);

    std::thread t3([&]
    {
        ramses::RendererSceneControlEventHandlerEmpty handler;
        initBarrier.wait();
        for (int i = 0; i < 10; ++i)
        {
            rendererMate.dispatchAndFlush(handler);
            EXPECT_TRUE(rendererMate.isRunning());
            rendererMate.getLastReportedSceneState(sceneId1);
            rendererMate.getLastReportedSceneState(sceneId2);
            rendererMate.enableKeysHandling();
        }
        finishedBarrier.wait();
    });

    t1.join();
    t2.join();
    t3.join();
}
