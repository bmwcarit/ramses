//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "RendererLib/SceneDisplayTracker.h"
#include "RendererLib/RendererCommands.h"

namespace ramses_internal
{
    class ASceneDisplayTracker : public ::testing::Test
    {
    protected:
        SceneDisplayTracker tracker;
    };

    TEST_F(ASceneDisplayTracker, noOwnershipForUnknownScene)
    {
        constexpr SceneId sceneId{ 123 };
        EXPECT_FALSE(tracker.getSceneOwnership(sceneId).isValid());
    }

    TEST_F(ASceneDisplayTracker, tracksSceneOwnership)
    {
        constexpr SceneId sceneId1{ 123 };
        constexpr SceneId sceneId2{ 124 };
        constexpr DisplayHandle display1{ 2u };
        constexpr DisplayHandle display2{ 3u };

        EXPECT_FALSE(tracker.getSceneOwnership(sceneId1).isValid());
        EXPECT_FALSE(tracker.getSceneOwnership(sceneId2).isValid());

        tracker.setSceneOwnership(sceneId1, display1);
        EXPECT_EQ(display1, tracker.getSceneOwnership(sceneId1));
        EXPECT_FALSE(tracker.getSceneOwnership(sceneId2).isValid());

        tracker.setSceneOwnership(sceneId2, display2);
        EXPECT_EQ(display1, tracker.getSceneOwnership(sceneId1));
        EXPECT_EQ(display2, tracker.getSceneOwnership(sceneId2));

        tracker.setSceneOwnership(sceneId1, display2);
        EXPECT_EQ(display2, tracker.getSceneOwnership(sceneId1));
        EXPECT_EQ(display2, tracker.getSceneOwnership(sceneId2));

        tracker.setSceneOwnership(sceneId2, display1);
        EXPECT_EQ(display2, tracker.getSceneOwnership(sceneId1));
        EXPECT_EQ(display1, tracker.getSceneOwnership(sceneId2));
    }

    TEST_F(ASceneDisplayTracker, determinesDisplayFromCommandOrFromOwnership)
    {
        constexpr SceneId sceneId{ 123 };
        constexpr DisplayHandle sceneDisplay{ 2u };
        constexpr DisplayHandle cmdDisplay{ 3u };

        tracker.setSceneOwnership(sceneId, sceneDisplay);

        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::ReceiveScene{ SceneInfo{ sceneId } }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::UpdateScene{ sceneId, {} }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::SetSceneState{ sceneId, RendererSceneState::Ready }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::SetSceneMapping{ sceneId, cmdDisplay }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::SetSceneDisplayBufferAssignment{ sceneId, {}, {} }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::LinkData{ {}, {}, sceneId, {} }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::LinkOffscreenBuffer{ {}, sceneId, {} }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::LinkStreamBuffer{ {}, sceneId, {} }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::UnlinkData{ sceneId, {} }));
        EXPECT_EQ(sceneDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::PickEvent{ sceneId, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::CreateDisplay{ cmdDisplay, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::DestroyDisplay{ cmdDisplay }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::CreateOffscreenBuffer{ cmdDisplay, {}, {}, {}, {}, {}, ERenderBufferType_DepthStencilBuffer }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::CreateDmaOffscreenBuffer{ cmdDisplay, {}, {}, {}, {}, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::DestroyOffscreenBuffer{ cmdDisplay, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::CreateStreamBuffer{ cmdDisplay, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::DestroyStreamBuffer{ cmdDisplay, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::SetStreamBufferState{ cmdDisplay, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::SetClearFlags{ cmdDisplay, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::SetClearColor{ cmdDisplay, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::SetExterallyOwnedWindowSize{ cmdDisplay, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::UpdateWarpingData{ cmdDisplay, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::ReadPixels{ cmdDisplay, {}, {}, {}, {}, {}, {}, {}, {} }));
        EXPECT_EQ(cmdDisplay, tracker.determineDisplayFromRendererCommand(RendererCommand::ConfirmationEcho{ cmdDisplay, {} }));
        // broadcast commands
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::ScenePublished{ sceneId, EScenePublicationMode_LocalOnly }));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SceneUnpublished{ sceneId }));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SetSkippingOfUnmodifiedBuffers{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::LogStatistics{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::LogInfo{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCListIviSurfaces{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCSetIviSurfaceVisibility{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCSetIviSurfaceOpacity{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCSetIviSurfaceDestRectangle{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCScreenshot{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCAddIviSurfaceToIviLayer{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCSetIviLayerVisibility{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCRemoveIviSurfaceFromIviLayer{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SCDestroyIviSurface{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SetLimits_FrameBudgets{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SetLimits_FlushesForceApply{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::SetLimits_FlushesForceUnsubscribe{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::FrameProfiler_Toggle{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::FrameProfiler_TimingGraphHeight{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::FrameProfiler_CounterGraphHeight{}));
        EXPECT_FALSE(tracker.determineDisplayFromRendererCommand(RendererCommand::FrameProfiler_RegionFilterFlags{}));
    }

    TEST_F(ASceneDisplayTracker, returnsInvalidDisplayIfNoOwnership)
    {
        constexpr SceneId sceneId{ 123 };
        const auto disp = tracker.determineDisplayFromRendererCommand(RendererCommand::ReceiveScene{ SceneInfo{ sceneId } });
        ASSERT_TRUE(disp.has_value());
        EXPECT_FALSE(disp->isValid());
    }

    TEST_F(ASceneDisplayTracker, returnsInvalidDisplayIfNoOwnerDisplayUnregistered)
    {
        constexpr SceneId sceneId1{ 123 };
        constexpr SceneId sceneId2{ 124 };
        constexpr DisplayHandle display1{ 2u };
        constexpr DisplayHandle display2{ 3u };

        tracker.setSceneOwnership(sceneId1, display1);
        tracker.setSceneOwnership(sceneId2, display2);
        EXPECT_EQ(display1, tracker.getSceneOwnership(sceneId1));
        EXPECT_EQ(display2, tracker.getSceneOwnership(sceneId2));

        tracker.unregisterDisplay(display2);
        EXPECT_EQ(display1, tracker.getSceneOwnership(sceneId1));
        EXPECT_FALSE(tracker.getSceneOwnership(sceneId2).isValid());

        tracker.unregisterDisplay(display1);
        EXPECT_FALSE(tracker.getSceneOwnership(sceneId1).isValid());
        EXPECT_FALSE(tracker.getSceneOwnership(sceneId2).isValid());
    }
}
