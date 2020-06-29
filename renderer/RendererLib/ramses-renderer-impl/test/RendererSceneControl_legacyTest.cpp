//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererSceneControl_legacy.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler_legacy.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "RendererLib/RendererCommands.h"
#include "RendererSceneControlImpl_legacy.h"

using namespace testing;

class ARendererSceneControl_legacy : public ::testing::Test
{
protected:
    ARendererSceneControl_legacy()
        : m_renderer(*m_framework.createRenderer({}))
        , m_sceneControlAPI(*m_renderer.getSceneControlAPI_legacy())
        , m_commandBuffer(m_sceneControlAPI.impl.getPendingCommands())
        , m_displayId(m_renderer.createDisplay(ramses::DisplayConfig{}))
    {
    }

    void expectCommand(uint32_t index, ramses_internal::ERendererCommand commandType)
    {
        ASSERT_LT(index, m_commandBuffer.getCommands().getTotalCommandCount());
        EXPECT_EQ(commandType, m_commandBuffer.getCommands().getCommandType(index));
    }

    void expectCommandCount(uint32_t count)
    {
        EXPECT_EQ(count, m_commandBuffer.getCommands().getTotalCommandCount());
    }

protected:
    ramses::RamsesFramework m_framework;
    ramses::RamsesRenderer& m_renderer;
    ramses::RendererSceneControl_legacy& m_sceneControlAPI;
    const ramses_internal::RendererCommands& m_commandBuffer;
    const ramses::displayId_t m_displayId;
};

TEST_F(ARendererSceneControl_legacy, hasNoCommandsOnStartUp)
{
    expectCommandCount(0u);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForBufferClearColorSet)
{
    const ramses::displayBufferId_t ob{ 123u };
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.setDisplayBufferClearColor(m_displayId, ob, 0.1f, 0.2f, 0.3f, 0.4f));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_SetClearColor);
    const auto& cmd = m_commandBuffer.getCommands().getCommandData<ramses_internal::SetClearColorCommand>(0u);
    EXPECT_EQ(ob.getValue(), cmd.obHandle.asMemoryHandle());
    EXPECT_EQ(ramses_internal::Vector4(0.1f, 0.2f, 0.3f, 0.4f), cmd.clearColor);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForFramebufferClearColorSetUsingDisplayBufferId)
{
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.setDisplayBufferClearColor(m_displayId, m_renderer.getDisplayFramebuffer(m_displayId), 0.1f, 0.2f, 0.3f, 0.4f));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_SetClearColor);

    const auto& cmd = m_commandBuffer.getCommands().getCommandData<ramses_internal::SetClearColorCommand>(0u);
    EXPECT_FALSE(cmd.obHandle.isValid());
    EXPECT_EQ(ramses_internal::Vector4(0.1f, 0.2f, 0.3f, 0.4f), cmd.clearColor);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForFramebufferClearColorSetUsingDefaultArg)
{
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.setDisplayBufferClearColor(m_displayId));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_SetClearColor);

    const auto& cmd = m_commandBuffer.getCommands().getCommandData<ramses_internal::SetClearColorCommand>(0u);
    EXPECT_FALSE(cmd.obHandle.isValid());
    EXPECT_EQ(ramses_internal::Vector4(0.f, 0.f, 0.f, 1.f), cmd.clearColor);
}

TEST_F(ARendererSceneControl_legacy, clearsAllPendingCommandsWhenCallingFlush)
{
    m_sceneControlAPI.setDisplayBufferClearColor(m_displayId);
    expectCommandCount(1u);
    m_sceneControlAPI.flush();
    expectCommandCount(0u);
}

TEST_F(ARendererSceneControl_legacy, createsCommandSceneSubscription)
{
    const ramses::sceneId_t scene(1u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.subscribeScene(scene));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_SubscribeScene);
}

TEST_F(ARendererSceneControl_legacy, createsCommandSceneUnsubscription)
{
    const ramses::sceneId_t scene(23u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unsubscribeScene(scene));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_UnsubscribeScene);
}

TEST_F(ARendererSceneControl_legacy, createsCommandSceneShow)
{
    const ramses::sceneId_t scene(23u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.showScene(scene));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_ShowScene);
}

TEST_F(ARendererSceneControl_legacy, createsCommandSceneHide)
{
    const ramses::sceneId_t scene(23u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.hideScene(scene));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_HideScene);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForSceneMapping)
{
    const ramses::sceneId_t scene(43u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.mapScene(m_displayId, scene));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_MapSceneToDisplay);
}

TEST_F(ARendererSceneControl_legacy, createsCommandSceneUnmapping)
{
    const ramses::sceneId_t scene(23u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unmapScene(scene));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_UnmapSceneFromDisplays);
}

TEST_F(ARendererSceneControl_legacy, createsNoCommandForTransformDataLinkingWithinTheSameScene)
{
    const ramses::sceneId_t scene(1u);
    const ramses::dataProviderId_t providerId(3u);
    const ramses::dataConsumerId_t consumerId(4u);

    EXPECT_NE(ramses::StatusOK, m_sceneControlAPI.linkData(scene, providerId, scene, consumerId));
    expectCommandCount(0u);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForTransformDataLinkingBetweenDifferentScenes)
{
    const ramses::sceneId_t scene1(1u);
    const ramses::sceneId_t scene2(2u);
    const ramses::dataProviderId_t providerId(3u);
    const ramses::dataConsumerId_t consumerId(4u);

    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(scene1, providerId, scene2, consumerId));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_LinkSceneData);
}

TEST_F(ARendererSceneControl_legacy, createsComandForUnlinkTransformDataLink)
{
    const ramses::sceneId_t scene(1u);
    const ramses::dataConsumerId_t consumerId(4u);

    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unlinkData(scene, consumerId));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_UnlinkSceneData);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForAssignSceneToOffscreenBuffer)
{
    const ramses::sceneId_t scene(1u);
    const ramses::displayBufferId_t bufferId(123u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.assignSceneToDisplayBuffer(scene, bufferId, 11));

    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    const auto& cmd = m_commandBuffer.getCommands().getCommandData<ramses_internal::SceneMappingCommand>(0u);
    EXPECT_EQ(bufferId.getValue(), cmd.offscreenBuffer.asMemoryHandle());
    EXPECT_EQ(11, cmd.sceneRenderOrder);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForAssignSceneToFramebufferUsingDisplayBufferId)
{
    const ramses::sceneId_t scene(1u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.assignSceneToDisplayBuffer(scene, m_renderer.getDisplayFramebuffer(m_displayId), 11));

    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    const auto& cmd = m_commandBuffer.getCommands().getCommandData<ramses_internal::SceneMappingCommand>(0u);
    EXPECT_FALSE(cmd.offscreenBuffer.isValid());
    EXPECT_EQ(11, cmd.sceneRenderOrder);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForAssignSceneToFramebufferUsingInvalidBufferID)
{
    const ramses::sceneId_t scene(1u);
    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.assignSceneToDisplayBuffer(scene, ramses::displayBufferId_t::Invalid(), 11));

    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_AssignSceneToDisplayBuffer);
    const auto& cmd = m_commandBuffer.getCommands().getCommandData<ramses_internal::SceneMappingCommand>(0u);
    EXPECT_FALSE(cmd.offscreenBuffer.isValid());
    EXPECT_EQ(11, cmd.sceneRenderOrder);
}

TEST_F(ARendererSceneControl_legacy, createsCommandForOffscreenBufferLink)
{
    const ramses::displayBufferId_t bufferId(0u);
    const ramses::sceneId_t consumerScene(1u);
    const ramses::dataConsumerId_t dataConsumer(2u);

    EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkOffscreenBufferToSceneData(bufferId, consumerScene, dataConsumer));
    expectCommandCount(1u);
    expectCommand(0u, ramses_internal::ERendererCommand_LinkBufferToSceneData);
}

TEST_F(ARendererSceneControl_legacy, canRunRendererInItsOwnThreadAndCallSceneAPIMethods)
{
    EXPECT_EQ(ramses::StatusOK, m_renderer.startThread());

    // most of these will fail but the purpose is to create and submit renderer commands for renderer running in another thread
    // thread sanitizer or other analyzer would catch race conditions when running this test
    m_sceneControlAPI.subscribeScene(ramses::sceneId_t(0u));
    m_sceneControlAPI.linkData(ramses::sceneId_t(0u), ramses::dataProviderId_t(1u), ramses::sceneId_t(2u), ramses::dataConsumerId_t(3u));
    m_sceneControlAPI.unlinkData(ramses::sceneId_t(0u), ramses::dataConsumerId_t(1u));
    m_sceneControlAPI.mapScene(m_displayId, ramses::sceneId_t(1u));
    m_sceneControlAPI.unmapScene(ramses::sceneId_t(0u));
    m_sceneControlAPI.showScene(ramses::sceneId_t(0u));
    m_sceneControlAPI.hideScene(ramses::sceneId_t(0u));
    m_sceneControlAPI.flush();

    m_sceneControlAPI.assignSceneToDisplayBuffer(ramses::sceneId_t(0u), ramses::displayBufferId_t{12}, 11);
    m_sceneControlAPI.linkOffscreenBufferToSceneData(ramses::displayBufferId_t{12}, ramses::sceneId_t(0u), ramses::dataConsumerId_t(0u));
    m_sceneControlAPI.setDisplayBufferClearColor(m_displayId, ramses::displayBufferId_t{12});
    m_sceneControlAPI.flush();

    ramses::RendererSceneControlEventHandlerEmpty_legacy handler;
    m_sceneControlAPI.dispatchEvents(handler);

    EXPECT_EQ(ramses::StatusOK, m_renderer.stopThread());
}
