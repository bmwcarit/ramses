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
#include "RendererLib/RendererCommandContainer.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/RendererCommandTypes.h"
#include "RamsesRendererImpl.h"

using namespace ramses_internal;
using namespace testing;

class ARendererMate : public Test
{
public:
    ARendererMate()
        : renderer(*ramsesFramework.createRenderer(ramses::RendererConfig()))
        , rendererMate(renderer.impl, ramsesFramework.impl)
        , rendererMateRendererEventHandler(rendererMate)
        , rendererMateEventHandler(rendererMate)
    {
        rendererMate.setSceneMapping(sceneId, displayId);
    }

protected:
    void expectLogConfirmationCommand()
    {
        const ramses_internal::RendererCommandContainer& cmds = renderer.impl.getPendingCommands().getCommands();
        bool found = false;
        for (uint32_t i = 0; i < cmds.getTotalCommandCount(); ++i)
            if (cmds.getCommandType(i) == ERendererCommand_ConfirmationEcho)
                found = true;
        EXPECT_TRUE(found) << "expected confirmation echo command";
        clearCommands();
    }

    void clearCommands()
    {
        const_cast<ramses_internal::RendererCommands&>(renderer.impl.getPendingCommands()).clear();
    }

    void publishAndExpectToGetToState(ramses::RendererSceneState state, bool expectConfirmation = false)
    {
        EXPECT_FALSE(isSceneShown());

        rendererMateEventHandler.scenePublished(sceneId);
        if (state != ramses::RendererSceneState::Unavailable)
            rendererMateEventHandler.sceneStateChanged(sceneId, state);

        EXPECT_EQ(state, rendererMate.getLastReportedSceneState(sceneId));
        if (expectConfirmation)
            expectLogConfirmationCommand();
        clearCommands();
    }

    bool isSceneShown() const
    {
        return rendererMate.getLastReportedSceneState(sceneId) == ramses::RendererSceneState::Rendered;
    }

    void doRenderLoop()
    {
        renderer.doOneLoop();
    }

private:
    ramses::RamsesFramework ramsesFramework;
    ramses::RamsesRenderer& renderer; // restrict access to renderer, acts only as dummy for RendererMate and to create (dummy) displays

protected:
    ramses::RendererMate rendererMate;
    ramses::IRendererEventHandler& rendererMateRendererEventHandler;
    ramses::IRendererSceneControlEventHandler& rendererMateEventHandler;

    const ramses::sceneId_t sceneId{ 33 };
    const ramses::displayId_t displayId{ 1 };
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
    rendererMateEventHandler.scenePublished(sceneId);
    rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered, "scene is shown now");

    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Available);

    // simulate disconnect, renderer framework sends unpublish for all its scenes
    rendererMateEventHandler.sceneStateChanged(sceneId, ramses::RendererSceneState::Unavailable);

    publishAndExpectToGetToState(ramses::RendererSceneState::Rendered, true); // expect confirmation
}
