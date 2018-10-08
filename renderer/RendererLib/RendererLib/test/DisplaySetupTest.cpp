//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/DisplaySetup.h"

using namespace testing;
using namespace ramses_internal;

class ADisplaySetup : public ::testing::Test
{
protected:
    void expectMappedScenesInOrder(DeviceResourceHandle buffer, std::initializer_list<MappedSceneInfo> mappedScenes)
    {
        for (const auto& mappedScene : mappedScenes)
            EXPECT_EQ(buffer, displaySetup.findDisplayBufferSceneIsMappedTo(mappedScene.sceneId));

        const MappedScenes& bufferScenes = displaySetup.getDisplayBuffer(buffer).mappedScenes;
        ASSERT_EQ(mappedScenes.size(), bufferScenes.size());
        const bool areEqual = std::equal(mappedScenes.begin(), mappedScenes.end(), bufferScenes.cbegin(), [](const MappedSceneInfo& a, const MappedSceneInfo& b)
        {
            return a.sceneId == b.sceneId
                && a.globalSceneOrder == b.globalSceneOrder
                && a.shown == b.shown;
        });
        EXPECT_TRUE(areEqual);
    }

    DisplaySetup displaySetup;

    const Viewport viewport{ 1,2,3,4 };
    const Vector4 clearColor{ 10,20,30,40 };
};

TEST_F(ADisplaySetup, hasNoBuffersInitially)
{
    EXPECT_TRUE(displaySetup.getDisplayBuffers().empty());
    EXPECT_TRUE(displaySetup.getNonInterruptibleOffscreenBuffersToRender().empty());
    EXPECT_TRUE(displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()).empty());
}

TEST_F(ADisplaySetup, canRegisterFramebufferInfoWithInitialValues)
{
    const DeviceResourceHandle bufferHandle(33u);
    displaySetup.registerDisplayBuffer(bufferHandle, viewport, clearColor, false, false);

    const auto& bufferInfo = displaySetup.getDisplayBuffer(bufferHandle);
    EXPECT_EQ(viewport, bufferInfo.viewport);
    EXPECT_EQ(clearColor, bufferInfo.clearColor);
    EXPECT_TRUE(bufferInfo.mappedScenes.empty());
    EXPECT_FALSE(bufferInfo.isInterruptible);
    EXPECT_TRUE(bufferInfo.needsRerender);

    EXPECT_TRUE(displaySetup.getNonInterruptibleOffscreenBuffersToRender().empty());
    EXPECT_TRUE(displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()).empty());
}

TEST_F(ADisplaySetup, canRegisterNonInterruptibleOBInfoWithInitialValues)
{
    const DeviceResourceHandle bufferHandle(33u);
    displaySetup.registerDisplayBuffer(bufferHandle, viewport, clearColor, true, false);

    const auto& bufferInfo = displaySetup.getDisplayBuffer(bufferHandle);
    EXPECT_EQ(viewport, bufferInfo.viewport);
    EXPECT_EQ(clearColor, bufferInfo.clearColor);
    EXPECT_TRUE(bufferInfo.mappedScenes.empty());
    EXPECT_FALSE(bufferInfo.isInterruptible);
    EXPECT_TRUE(bufferInfo.needsRerender);

    EXPECT_EQ(DeviceHandleVector{ bufferHandle }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());

    EXPECT_TRUE(displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()).empty());
}

TEST_F(ADisplaySetup, canRegisterInterruptibleOBInfoWithInitialValues)
{
    const DeviceResourceHandle bufferHandle(33u);
    displaySetup.registerDisplayBuffer(bufferHandle, viewport, clearColor, true, true);

    const auto& bufferInfo = displaySetup.getDisplayBuffer(bufferHandle);
    EXPECT_EQ(viewport, bufferInfo.viewport);
    EXPECT_EQ(clearColor, bufferInfo.clearColor);
    EXPECT_TRUE(bufferInfo.mappedScenes.empty());
    EXPECT_TRUE(bufferInfo.isInterruptible);
    EXPECT_TRUE(bufferInfo.needsRerender);

    EXPECT_EQ(DeviceHandleVector{ bufferHandle }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));

    EXPECT_TRUE(displaySetup.getNonInterruptibleOffscreenBuffersToRender().empty());
}

TEST_F(ADisplaySetup, canBeQueriedForAnyRegisteredTypeOfBufferViaItsHandle)
{
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    EXPECT_FALSE(displaySetup.getDisplayBuffer(bufferHandleFB).isOffscreenBuffer);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).isOffscreenBuffer);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).isInterruptible);
}

TEST_F(ADisplaySetup, canUnregisterOB)
{
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    displaySetup.unregisterDisplayBuffer(bufferHandleOB);
    displaySetup.unregisterDisplayBuffer(bufferHandleOBint);

    EXPECT_TRUE(displaySetup.getDisplayBuffers().empty());
    EXPECT_TRUE(displaySetup.getNonInterruptibleOffscreenBuffersToRender().empty());
    EXPECT_TRUE(displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()).empty());
}

TEST_F(ADisplaySetup, marksNewlyRegisteredBuffersToRerender)
{
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);

    EXPECT_EQ(DeviceHandleVector{ bufferHandleOB }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOBint }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));
}

TEST_F(ADisplaySetup, canSetRerenderFlagOfABuffer)
{
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    displaySetup.setDisplayBufferToBeRerendered(bufferHandleFB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOBint, false);

    EXPECT_FALSE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_FALSE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_FALSE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);

    EXPECT_TRUE(displaySetup.getNonInterruptibleOffscreenBuffersToRender().empty());
    EXPECT_TRUE(displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()).empty());

    displaySetup.setDisplayBufferToBeRerendered(bufferHandleFB, true);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOB, true);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOBint, true);

    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);

    EXPECT_EQ(DeviceHandleVector{ bufferHandleOB }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOBint }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));
}

TEST_F(ADisplaySetup, canMapAndUnmapSceneToBuffer)
{
    const SceneId scene1(12u);
    const SceneId scene2(13u);
    const SceneId scene3(14u);
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    displaySetup.mapSceneToDisplayBuffer(scene1, bufferHandleFB, 0);
    displaySetup.mapSceneToDisplayBuffer(scene2, bufferHandleOB, 0);
    displaySetup.mapSceneToDisplayBuffer(scene3, bufferHandleOBint, 0);

    expectMappedScenesInOrder(bufferHandleFB, { { scene1, 0, false } });
    expectMappedScenesInOrder(bufferHandleOB, { { scene2, 0, false } });
    expectMappedScenesInOrder(bufferHandleOBint, { { scene3, 0, false } });

    displaySetup.unmapScene(scene1);
    displaySetup.unmapScene(scene2);
    displaySetup.unmapScene(scene3);

    expectMappedScenesInOrder(bufferHandleFB, {});
    expectMappedScenesInOrder(bufferHandleOB, {});
    expectMappedScenesInOrder(bufferHandleOBint, {});
    EXPECT_FALSE(displaySetup.findDisplayBufferSceneIsMappedTo(scene1).isValid());
    EXPECT_FALSE(displaySetup.findDisplayBufferSceneIsMappedTo(scene2).isValid());
    EXPECT_FALSE(displaySetup.findDisplayBufferSceneIsMappedTo(scene3).isValid());
}

TEST_F(ADisplaySetup, storesMappedScenesInRenderingOrder)
{
    const SceneId scene1(12u);
    const SceneId scene2(13u);
    const SceneId scene3(14u);
    const DeviceResourceHandle bufferHandleOB(34u);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);

    displaySetup.mapSceneToDisplayBuffer(scene2, bufferHandleOB, 10);
    expectMappedScenesInOrder(bufferHandleOB, { { scene2, 10, false } });

    displaySetup.mapSceneToDisplayBuffer(scene3, bufferHandleOB, -10);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene2, 10, false } });

    displaySetup.mapSceneToDisplayBuffer(scene1, bufferHandleOB, 0);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene1, 0, false },{ scene2, 10, false } });

    displaySetup.unmapScene(scene1);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene2, 10, false } });

    displaySetup.mapSceneToDisplayBuffer(scene1, bufferHandleOB, 666);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene2, 10, false },{ scene1, 666, false } });

    displaySetup.unmapScene(scene2);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene1, 666, false } });

    displaySetup.unmapScene(scene3);
    expectMappedScenesInOrder(bufferHandleOB, { { scene1, 666, false } });

    displaySetup.unmapScene(scene1);
    expectMappedScenesInOrder(bufferHandleOB, {});
}

TEST_F(ADisplaySetup, triggersRerenderForBufferWithMappedOrUnmappedScene)
{
    const SceneId scene1(12u);
    const SceneId scene2(13u);
    const SceneId scene3(14u);
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    // reset re-render state
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleFB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOBint, false);

    // trigger re-render by mapping
    displaySetup.mapSceneToDisplayBuffer(scene1, bufferHandleFB, 0);
    displaySetup.mapSceneToDisplayBuffer(scene2, bufferHandleOB, 0);
    displaySetup.mapSceneToDisplayBuffer(scene3, bufferHandleOBint, 0);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOB }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOBint }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));

    // reset re-render state
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleFB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOBint, false);

    // trigger re-render by unmapping
    displaySetup.unmapScene(scene1);
    displaySetup.unmapScene(scene2);
    displaySetup.unmapScene(scene3);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOB }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOBint }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));
}

TEST_F(ADisplaySetup, canSetSceneShown)
{
    const SceneId scene1(12u);
    const SceneId scene2(13u);
    const SceneId scene3(14u);
    const DeviceResourceHandle bufferHandleOB(34u);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);

    displaySetup.mapSceneToDisplayBuffer(scene1, bufferHandleOB, 0);
    displaySetup.mapSceneToDisplayBuffer(scene2, bufferHandleOB, 10);
    displaySetup.mapSceneToDisplayBuffer(scene3, bufferHandleOB, -10);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene1, 0, false },{ scene2, 10, false } });

    displaySetup.setSceneShown(scene1, true);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene1, 0, true },{ scene2, 10, false } });
    displaySetup.setSceneShown(scene2, true);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene1, 0, true },{ scene2, 10, true } });
    displaySetup.setSceneShown(scene3, true);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, true },{ scene1, 0, true },{ scene2, 10, true } });

    displaySetup.setSceneShown(scene1, false);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, true },{ scene1, 0, false },{ scene2, 10, true } });
    displaySetup.setSceneShown(scene2, false);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, true },{ scene1, 0, false },{ scene2, 10, false } });
    displaySetup.setSceneShown(scene3, false);
    expectMappedScenesInOrder(bufferHandleOB, { { scene3, -10, false },{ scene1, 0, false },{ scene2, 10, false } });
}

TEST_F(ADisplaySetup, triggersRerenderForBufferWithChangedSceneShowState)
{
    const SceneId scene1(12u);
    const SceneId scene2(13u);
    const SceneId scene3(14u);
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    displaySetup.mapSceneToDisplayBuffer(scene1, bufferHandleFB, 0);
    displaySetup.mapSceneToDisplayBuffer(scene2, bufferHandleOB, 0);
    displaySetup.mapSceneToDisplayBuffer(scene3, bufferHandleOBint, 0);

    // reset re-render state
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleFB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOBint, false);

    // trigger re-render by scene showing
    displaySetup.setSceneShown(scene1, true);
    displaySetup.setSceneShown(scene2, true);
    displaySetup.setSceneShown(scene3, true);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOB }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOBint }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));

    // reset re-render state
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleFB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOBint, false);

    // trigger re-render by scene hiding
    displaySetup.setSceneShown(scene1, false);
    displaySetup.setSceneShown(scene2, false);
    displaySetup.setSceneShown(scene3, false);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOB }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOBint }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));
}

TEST_F(ADisplaySetup, canSetClearColorForAnyRegisteredBuffer)
{
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    EXPECT_EQ(clearColor, displaySetup.getDisplayBuffer(bufferHandleFB).clearColor);
    EXPECT_EQ(clearColor, displaySetup.getDisplayBuffer(bufferHandleOB).clearColor);
    EXPECT_EQ(clearColor, displaySetup.getDisplayBuffer(bufferHandleOBint).clearColor);

    const Vector4 clr1{ 9, 8, 7, 6 };
    const Vector4 clr2{ 5, 4, 3, 2 };
    const Vector4 clr3{ 1, 0, -1, -2 };
    displaySetup.setClearColor(bufferHandleFB, clr1);
    displaySetup.setClearColor(bufferHandleOB, clr2);
    displaySetup.setClearColor(bufferHandleOBint, clr3);

    EXPECT_EQ(clr1, displaySetup.getDisplayBuffer(bufferHandleFB).clearColor);
    EXPECT_EQ(clr2, displaySetup.getDisplayBuffer(bufferHandleOB).clearColor);
    EXPECT_EQ(clr3, displaySetup.getDisplayBuffer(bufferHandleOBint).clearColor);
}

TEST_F(ADisplaySetup, triggersRerenderForBufferWithChangedClearColor)
{
    const DeviceResourceHandle bufferHandleFB(33u);
    const DeviceResourceHandle bufferHandleOB(34u);
    const DeviceResourceHandle bufferHandleOBint(35u);
    displaySetup.registerDisplayBuffer(bufferHandleFB, viewport, clearColor, false, false);
    displaySetup.registerDisplayBuffer(bufferHandleOB, viewport, clearColor, true, false);
    displaySetup.registerDisplayBuffer(bufferHandleOBint, viewport, clearColor, true, true);

    // reset re-render state
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleFB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOB, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandleOBint, false);

    // trigger re-render by setting clear color
    displaySetup.setClearColor(bufferHandleFB, clearColor);
    displaySetup.setClearColor(bufferHandleOB, clearColor);
    displaySetup.setClearColor(bufferHandleOBint, clearColor);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleFB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOB).needsRerender);
    EXPECT_TRUE(displaySetup.getDisplayBuffer(bufferHandleOBint).needsRerender);
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOB }, displaySetup.getNonInterruptibleOffscreenBuffersToRender());
    EXPECT_EQ(DeviceHandleVector{ bufferHandleOBint }, displaySetup.getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle::Invalid()));
}

TEST_F(ADisplaySetup, getsListOfInterruptibleBuffersToRerenderStartingFromInterruptedBuffer)
{
    const DeviceResourceHandle bufferHandle1(33u);
    const DeviceResourceHandle bufferHandle2(34u);
    const DeviceResourceHandle bufferHandle3(35u);
    displaySetup.registerDisplayBuffer(bufferHandle1, viewport, clearColor, true, true);
    displaySetup.registerDisplayBuffer(bufferHandle2, viewport, clearColor, true, true);
    displaySetup.registerDisplayBuffer(bufferHandle3, viewport, clearColor, true, true);

    const DeviceHandleVector expectedBuffersToRerender{ bufferHandle2, bufferHandle3 };
    EXPECT_EQ(expectedBuffersToRerender, displaySetup.getInterruptibleOffscreenBuffersToRender(bufferHandle2));
}

TEST_F(ADisplaySetup, getsListOfInterruptibleBuffersToRerenderStartingFromInterruptedBufferAndOnlyThoseMarkedToRerender)
{
    const DeviceResourceHandle bufferHandle1(33u);
    const DeviceResourceHandle bufferHandle2(34u);
    const DeviceResourceHandle bufferHandle3(35u);
    const DeviceResourceHandle bufferHandle4(36u);
    const DeviceResourceHandle bufferHandle5(37u);
    displaySetup.registerDisplayBuffer(bufferHandle1, viewport, clearColor, true, true);
    displaySetup.registerDisplayBuffer(bufferHandle2, viewport, clearColor, true, true);
    displaySetup.registerDisplayBuffer(bufferHandle3, viewport, clearColor, true, true);
    displaySetup.registerDisplayBuffer(bufferHandle4, viewport, clearColor, true, true);
    displaySetup.registerDisplayBuffer(bufferHandle5, viewport, clearColor, true, true);

    displaySetup.setDisplayBufferToBeRerendered(bufferHandle2, false);
    displaySetup.setDisplayBufferToBeRerendered(bufferHandle4, false);

    // the interrupted buffer (2) is always put to list regardless of its re-render flag,
    // buffer1 is skipped because it is before interrupted buffer,
    // buffer4 is skipped because it is not marked to re-render
    const DeviceHandleVector expectedBuffersToRerender{ bufferHandle2, bufferHandle3, bufferHandle5 };
    EXPECT_EQ(expectedBuffersToRerender, displaySetup.getInterruptibleOffscreenBuffersToRender(bufferHandle2));
}
