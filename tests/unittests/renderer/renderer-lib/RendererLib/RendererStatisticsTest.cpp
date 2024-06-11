//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererStatistics.h"
#include "internal/RendererLib/FrameProfilerStatistics.h"
#include "internal/Core/Utils/LogMacros.h"
#include "gmock/gmock.h"

using namespace testing;

namespace ramses::internal
{
    class ARendererStatistics : public ::testing::Test
    {
    public:
        [[nodiscard]] std::string logOutput() const
        {
            StringOutputStream strstr;
            stats.writeStatsToStream(strstr);
            return strstr.release();
        }

    protected:
        RendererStatistics stats;
        const SceneId sceneId1{ 11 };
        const SceneId sceneId2{ 22 };
        const DeviceResourceHandle ob1{ 11 };
        const DeviceResourceHandle ob2{ 22 };
        const DeviceResourceHandle ob3{ 33 };
    };

    TEST_F(ARendererStatistics, VerifyStringsOfRegionNames)
    {
        EXPECT_EQ(RegionNames.size(), 15u);
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::ExecuteRendererCommands), "RendererCommands");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateClientResources), "UpdateClientResources");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::ApplySceneActions), "ApplySceneActions");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateSceneResources), "UpdateSceneResources");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateEmbeddedCompositingResources), "UpdateEmbeddedCompositingResources");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateStreamTextures), "UpdateStreamTextures");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateScenesToBeMapped), "UpdateScenesToBeMapped");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateResourceCache), "UpdateResourceCache");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateTransformations), "UpdateTransformations");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateDataLinks), "UpdateDataLinks");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::UpdateSemanticUniformBuffers), "UpdateSemanticUniformBuffers");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::HandleDisplayEvents), "HandleDisplayEvents");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::DrawScenes), "DrawScenes");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::SwapBuffersAndNotifyClients), "SwapBuffersNotifyClients");
        EXPECT_STREQ(EnumToString(FrameProfilerStatistics::ERegion::MaxFramerateSleep), "MaxFramerateSleep");
    }

    TEST_F(ARendererStatistics, tracksDrawCallsPerFrame)
    {
        stats.frameFinished(1u);
        stats.frameFinished(2u);
        stats.frameFinished(3u);
        stats.frameFinished(4u);
        EXPECT_EQ(3u, stats.getDrawCallsPerFrame());

        stats.reset();
        EXPECT_EQ(0u, stats.getDrawCallsPerFrame());

        stats.frameFinished(3u);
        stats.frameFinished(3u);
        EXPECT_EQ(3u, stats.getDrawCallsPerFrame());
    }

    TEST_F(ARendererStatistics, tracksFrameCount)
    {
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("numFrames 3"));
    }

    TEST_F(ARendererStatistics, tracksSceneRenderedCount)
    {
        stats.sceneRendered(sceneId1);
        stats.frameFinished(0u);
        stats.sceneRendered(sceneId1);
        stats.sceneRendered(sceneId2);
        stats.frameFinished(0u);
        stats.sceneRendered(sceneId1);
        stats.sceneRendered(sceneId2);
        stats.frameFinished(0u);
        stats.sceneRendered(sceneId2);

        EXPECT_THAT(logOutput(), HasSubstr("Scene 11: rendered 3"));
        EXPECT_THAT(logOutput(), HasSubstr("Scene 22: rendered 3"));
    }

    TEST_F(ARendererStatistics, untracksScene)
    {
        stats.sceneRendered(sceneId1);
        stats.frameFinished(0u);
        stats.sceneRendered(sceneId1);
        stats.sceneRendered(sceneId2);
        stats.frameFinished(0u);

        stats.untrackScene(sceneId1);

        EXPECT_THAT(logOutput(), Not(HasSubstr("Scene 11")));
        EXPECT_THAT(logOutput(), HasSubstr("Scene 22: rendered 1"));
    }

    TEST_F(ARendererStatistics, tracksSceneArrivedFlushesIndependentlyFromFrames)
    {
        stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.trackArrivedFlush(sceneId2, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.trackArrivedFlush(sceneId2, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.frameFinished(0u);
        stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4, std::chrono::milliseconds{0});

        EXPECT_THAT(logOutput(), HasSubstr("Scene 11:"));
        EXPECT_THAT(logOutput(), HasSubstr("FArrived 3"));
        EXPECT_THAT(logOutput(), HasSubstr("Scene 22:"));
        EXPECT_THAT(logOutput(), HasSubstr("FArrived 2"));
    }

    TEST_F(ARendererStatistics, tracksFramesWhereSceneFlushArrived)
    {
        stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.frameFinished(0u);
        stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4, std::chrono::milliseconds{0});
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("numFrames 4"));
        EXPECT_THAT(logOutput(), HasSubstr("framesFArrived 2"));
    }

    TEST_F(ARendererStatistics, tracksFramesWhereSceneFlushApplied)
    {
        stats.flushApplied(sceneId1);
        stats.flushApplied(sceneId1);
        stats.flushApplied(sceneId1);
        stats.frameFinished(0u);
        stats.flushApplied(sceneId1);
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("numFrames 4"));
        EXPECT_THAT(logOutput(), HasSubstr("framesFApplied 2"));
    }

    TEST_F(ARendererStatistics, tracksFramesWhereSceneFlushBlocked)
    {
        stats.flushBlocked(sceneId1);
        stats.flushBlocked(sceneId1);
        stats.flushBlocked(sceneId1);
        stats.frameFinished(0u);
        stats.flushBlocked(sceneId1);
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("numFrames 4"));
        EXPECT_THAT(logOutput(), HasSubstr("framesFBlocked 2"));
    }

    TEST_F(ARendererStatistics, tracksMaximumConsecutiveFramesWhereSceneFlushNotApplied)
    {
        stats.flushApplied(sceneId1);
        stats.flushApplied(sceneId1);
        stats.flushApplied(sceneId1);
        stats.frameFinished(0u);
        stats.flushApplied(sceneId1);
        stats.frameFinished(0u);
        stats.frameFinished(0u); //x
        stats.frameFinished(0u); //x
        stats.flushApplied(sceneId1);
        stats.flushApplied(sceneId1);
        stats.frameFinished(0u);
        stats.frameFinished(0u); //x

        EXPECT_THAT(logOutput(), HasSubstr("numFrames 6"));
        EXPECT_THAT(logOutput(), HasSubstr("FApplied 6"));
        EXPECT_THAT(logOutput(), HasSubstr("framesFApplied 3"));
        EXPECT_THAT(logOutput(), HasSubstr("maxFramesWithNoFApplied 2"));
    }

    TEST_F(ARendererStatistics, tracksMaximumConsecutiveFramesWhereSceneFlushBlocked)
    {
        stats.flushBlocked(sceneId1);
        stats.frameFinished(0u);
        stats.flushBlocked(sceneId1);
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.flushBlocked(sceneId1);
        stats.frameFinished(0u);
        stats.flushBlocked(sceneId1);
        stats.frameFinished(0u);
        stats.flushBlocked(sceneId1);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("numFrames 6"));
        EXPECT_THAT(logOutput(), HasSubstr("framesFBlocked 5"));
        EXPECT_THAT(logOutput(), HasSubstr("maxFramesFBlocked 3"));
    }

    TEST_F(ARendererStatistics, tracksFramebufferAndOffscreenBufferSwapCounts)
    {
        stats.framebufferSwapped();
        stats.offscreenBufferSwapped(ob1, false);
        stats.framebufferSwapped();
        stats.offscreenBufferSwapped(ob2, false);
        stats.offscreenBufferSwapped(ob3, false);
        stats.frameFinished(0u);

        stats.framebufferSwapped();
        stats.offscreenBufferSwapped(ob1, false);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("FB: 3; OB11: 2; OB22: 1; OB33: 1"));
    }

    TEST_F(ARendererStatistics, tracksInterruptibleOffscreenBuffer)
    {
        stats.offscreenBufferInterrupted(ob1);
        stats.frameFinished(0u);
        stats.offscreenBufferSwapped(ob1, true);
        stats.frameFinished(0u);
        stats.offscreenBufferInterrupted(ob1);
        stats.frameFinished(0u);
        stats.offscreenBufferSwapped(ob1, true);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("OB11: 2 (intr: 2)"));
    }

    TEST_F(ARendererStatistics, untracksOffscreenBuffer)
    {
        stats.offscreenBufferSwapped(ob1, false);
        stats.offscreenBufferSwapped(ob2, false);
        stats.offscreenBufferSwapped(ob3, false);
        stats.frameFinished(0u);
        stats.untrackOffscreenBuffer(ob2);

        EXPECT_THAT(logOutput(), Not(HasSubstr("OB22")));
    }

    TEST_F(ARendererStatistics, tracksStreamTextureSource)
    {
        const WaylandIviSurfaceId src{ 99u };
        stats.streamTextureUpdated(src, 2u);
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.streamTextureUpdated(src, 9u);
        stats.frameFinished(0u);
        stats.streamTextureUpdated(src, 1u);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("numFrames 5"));
        EXPECT_THAT(logOutput(), HasSubstr("SourceId ivi-surface:99: upd 12, framesUpd 3, maxUpdInFrame 9, maxFramesWithNoUpd 2"));
}

    TEST_F(ARendererStatistics, logsValidNumbersWhenStreamTextureInactive)
    {
        const WaylandIviSurfaceId src{ 99u };
        stats.streamTextureUpdated(src, 2u); // will register source
        stats.reset();
        stats.frameFinished(0u);
        stats.frameFinished(0u);
        stats.frameFinished(0u);

        EXPECT_THAT(logOutput(), HasSubstr("SourceId ivi-surface:99: upd 0, framesUpd 0, maxUpdInFrame 0, maxFramesWithNoUpd 3"));
}

    TEST_F(ARendererStatistics, untracksStreamTextureSource)
    {
        const WaylandIviSurfaceId src{ 99u };
        stats.streamTextureUpdated(src, 2u);
        stats.frameFinished(0u);

        stats.untrackStreamTexture(src);

        EXPECT_THAT(logOutput(), Not(HasSubstr("SourceId ivi-surface:99")));
}

    TEST_F(ARendererStatistics, tracksResourceUploads)
    {
        stats.resourceUploaded(2u);
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("resUploaded 1 (2 B)"));

        stats.reset();
        EXPECT_THAT(logOutput(), Not(HasSubstr("resUploaded")));

        stats.resourceUploaded(2u);
        stats.resourceUploaded(77u);
        stats.resourceUploaded(100u);
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("resUploaded 3 (179 B)"));

        stats.reset();
        EXPECT_THAT(logOutput(), Not(HasSubstr("resUploaded")));
    }

    TEST_F(ARendererStatistics, tracksSceneResourceUploads)
    {
        stats.sceneResourceUploaded(sceneId1, 2u);
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("RSUploaded 1 (2 B)"));

        stats.reset();
        EXPECT_THAT(logOutput(), Not(HasSubstr("RSUploaded")));

        stats.sceneResourceUploaded(sceneId1, 2u);
        stats.sceneResourceUploaded(sceneId1, 77u);
        stats.sceneResourceUploaded(sceneId2, 100u);
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("RSUploaded 2 (79 B)")); //scene1
        EXPECT_THAT(logOutput(), HasSubstr("RSUploaded 1 (100 B)")); //scene2

        stats.reset();
        EXPECT_THAT(logOutput(), Not(HasSubstr("RSUploaded")));
    }

    TEST_F(ARendererStatistics, tracksShaderCompilationAndTimes)
    {
        stats.shaderCompiled(std::chrono::microseconds(2u), "some effect", SceneId(123));
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("shadersCompiled 1"));
        EXPECT_THAT(logOutput(), HasSubstr("for total ms:0"));

        stats.reset();
        EXPECT_THAT(logOutput(), Not(HasSubstr("shadersCompiled")));

        stats.shaderCompiled(std::chrono::microseconds(3000u), "some effect name", SceneId(123));
        stats.shaderCompiled(std::chrono::microseconds(5000u), "some effect name", SceneId(124));
        stats.shaderCompiled(std::chrono::microseconds(7000u), "longest effect name", SceneId(125));
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("shadersCompiled 3"));
        EXPECT_THAT(logOutput(), HasSubstr("for total ms:15"));
        EXPECT_THAT(logOutput(), HasSubstr("longest: longest effect name"));
        EXPECT_THAT(logOutput(), HasSubstr("from scene:125"));
        EXPECT_THAT(logOutput(), HasSubstr("ms:7"));

        stats.reset();
        EXPECT_THAT(logOutput(), Not(HasSubstr("shadersCompiled")));
    }


    TEST_F(ARendererStatistics, tracksExpirationOffsets)
    {
        stats.addExpirationOffset(sceneId1, -100);
        stats.addExpirationOffset(sceneId1, -80);
        stats.addExpirationOffset(sceneId1, -120);
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("Exp (0/3:-120/-80/-100)"));
        stats.reset();

        stats.addExpirationOffset(sceneId1, -30);
        stats.addExpirationOffset(sceneId1, 10);
        stats.addExpirationOffset(sceneId1, -10);
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("Exp (1/3:-30/10/-10)"));
        stats.reset();

        stats.addExpirationOffset(sceneId1, -30);
        stats.addExpirationOffset(sceneId2, 30);
        stats.addExpirationOffset(sceneId1, -20);
        stats.addExpirationOffset(sceneId2, 20);
        stats.addExpirationOffset(sceneId1, -10);
        stats.addExpirationOffset(sceneId2, 10);
        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), HasSubstr("Exp (0/3:-30/-10/-20)"));
        EXPECT_THAT(logOutput(), HasSubstr("Exp (3/3:10/30/20)"));
        stats.reset();

        stats.frameFinished(0u);
        EXPECT_THAT(logOutput(), Not(HasSubstr("Exp (")));
    }


    TEST_F(ARendererStatistics, confidenceTest_fullLogOutput)
    {
        for (size_t period = 0u; period < 2u; ++period)
        {
            stats.trackArrivedFlush(sceneId1, 123, 5, 3, 4, std::chrono::milliseconds{2});
            stats.trackArrivedFlush(sceneId2, 6, 7, 8, 9, std::chrono::milliseconds{5});
            stats.flushBlocked(sceneId1);
            stats.flushApplied(sceneId2);
            stats.framebufferSwapped();
            stats.frameFinished(0);

            stats.resourceUploaded(2u);
            stats.resourceUploaded(77u);
            stats.shaderCompiled(std::chrono::microseconds(0u), "", SceneId(54321));
            stats.shaderCompiled(std::chrono::microseconds(1000u), "slow effect", SceneId(12345));

            stats.trackArrivedFlush(sceneId1, 123, 5, 3, 4, std::chrono::milliseconds{3});
            stats.flushBlocked(sceneId1);
            stats.sceneRendered(sceneId2);
            stats.framebufferSwapped();
            stats.frameFinished(100);

            stats.trackArrivedFlush(sceneId1, 123, 5, 3, 4, std::chrono::milliseconds{6});
            stats.trackArrivedFlush(sceneId2, 6, 7, 8, 9, std::chrono::milliseconds{11});
            stats.flushApplied(sceneId1);
            stats.flushBlocked(sceneId2);
            stats.sceneRendered(sceneId1);
            stats.sceneRendered(sceneId2);
            stats.framebufferSwapped();
            stats.offscreenBufferInterrupted(ob1);
            stats.framebufferSwapped();
            stats.frameFinished(0);

            stats.sceneResourceUploaded(sceneId1, 3u);
            stats.sceneResourceUploaded(sceneId1, 77u);
            stats.sceneResourceUploaded(sceneId2, 200u);

            stats.sceneRendered(sceneId1);
            stats.framebufferSwapped();
            stats.offscreenBufferSwapped(ob1, true);
            stats.frameFinished(0);

            EXPECT_THAT(logOutput(), HasSubstr("Avg framerate: "));
            EXPECT_THAT(logOutput(), HasSubstr("FPS [minFrameTime "));
            EXPECT_THAT(logOutput(), HasSubstr("us, maxFrameTime "));
            EXPECT_THAT(logOutput(), HasSubstr("], drawCalls (0/100/25), numFrames 4"));
            EXPECT_THAT(logOutput(), HasSubstr("resUploaded 2 (79 B)"));
            EXPECT_THAT(logOutput(), HasSubstr("shadersCompiled 2"));
            EXPECT_THAT(logOutput(), HasSubstr("for total ms:1"));
            EXPECT_THAT(logOutput(), HasSubstr("FB: 5; OB11: 1 (intr: 1)"));
            EXPECT_THAT(logOutput(), HasSubstr("Scene 11: rendered 2, framesFArrived 3, framesFApplied 1, framesFBlocked 2, maxFramesWithNoFApplied 2, maxFramesFBlocked 2, FArrived 3, FApplied 1, actions/F (123/123/123), dt/F (2/6/3.6666667), RC+/F (5/5/5), RC-/F (3/3/3), RS/F (4/4/4), RSUploaded 2 (80 B)"));
            EXPECT_THAT(logOutput(), HasSubstr("Scene 22: rendered 2, framesFArrived 2, framesFApplied 1, framesFBlocked 1, maxFramesWithNoFApplied 3, maxFramesFBlocked 1, FArrived 2, FApplied 1, actions/F (6/6/6), dt/F (5/11/8), RC+/F (7/7/7), RC-/F (8/8/8), RS/F (9/9/9), RSUploaded 1 (200 B)"));
            EXPECT_THAT(logOutput(), HasSubstr("slow effect"));
            stats.reset();
        }
    }
}
