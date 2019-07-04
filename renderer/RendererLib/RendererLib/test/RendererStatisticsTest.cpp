//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/RendererStatistics.h"
#include "Utils/LogMacros.h"

using namespace testing;
using namespace ramses_internal;

class ARendererStatistics : public ::testing::Test
{
public:
    bool logOutputContains(const String& str) const
    {
        StringOutputStream strstr;
        stats.writeStatsToStream(strstr);
        return strstr.release().find(str) >= 0;
    }

protected:
    RendererStatistics stats;
    const SceneId sceneId1{ 11 };
    const SceneId sceneId2{ 22 };
    const DisplayHandle disp1{ 1 };
    const DisplayHandle disp2{ 2 };
    const DeviceResourceHandle ob1{ 11 };
    const DeviceResourceHandle ob2{ 22 };
    const DeviceResourceHandle ob3{ 33 };
};

TEST_F(ARendererStatistics, tracksDrawCallsPerFrame)
{
    stats.frameFinished(1u);
    stats.frameFinished(2u);
    stats.frameFinished(3u);
    stats.frameFinished(4u);
    EXPECT_EQ(2u, stats.getDrawCallsPerFrame());

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

    EXPECT_TRUE(logOutputContains("numFrames 3"));
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

    EXPECT_TRUE(logOutputContains("Scene 11: rendered 3"));
    EXPECT_TRUE(logOutputContains("Scene 22: rendered 3"));
}

TEST_F(ARendererStatistics, untracksScene)
{
    stats.sceneRendered(sceneId1);
    stats.frameFinished(0u);
    stats.sceneRendered(sceneId1);
    stats.sceneRendered(sceneId2);
    stats.frameFinished(0u);

    stats.untrackScene(sceneId1);

    EXPECT_FALSE(logOutputContains("Scene 11"));
    EXPECT_TRUE(logOutputContains("Scene 22: rendered 1"));
}

TEST_F(ARendererStatistics, tracksSceneArrivedFlushesIndependentlyFromFrames)
{
    stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4);
    stats.trackArrivedFlush(sceneId2, 1, 2, 3, 4);
    stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4);
    stats.trackArrivedFlush(sceneId2, 1, 2, 3, 4);
    stats.frameFinished(0u);
    stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4);

    EXPECT_TRUE(logOutputContains("Scene 11:"));
    EXPECT_TRUE(logOutputContains("FArrived 3"));
    EXPECT_TRUE(logOutputContains("Scene 22:"));
    EXPECT_TRUE(logOutputContains("FArrived 2"));
}

TEST_F(ARendererStatistics, tracksFramesWhereSceneFlushArrived)
{
    stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4);
    stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4);
    stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4);
    stats.frameFinished(0u);
    stats.trackArrivedFlush(sceneId1, 1, 2, 3, 4);
    stats.frameFinished(0u);
    stats.frameFinished(0u);
    stats.frameFinished(0u);

    EXPECT_TRUE(logOutputContains("numFrames 4"));
    EXPECT_TRUE(logOutputContains("framesFArrived 2"));
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

    EXPECT_TRUE(logOutputContains("numFrames 4"));
    EXPECT_TRUE(logOutputContains("framesFApplied 2"));
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

    EXPECT_TRUE(logOutputContains("numFrames 4"));
    EXPECT_TRUE(logOutputContains("framesFBlocked 2"));
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

    EXPECT_TRUE(logOutputContains("numFrames 6"));
    EXPECT_TRUE(logOutputContains("FApplied 6"));
    EXPECT_TRUE(logOutputContains("framesFApplied 3"));
    EXPECT_TRUE(logOutputContains("maxFramesWithNoFApplied 2"));
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

    EXPECT_TRUE(logOutputContains("numFrames 6"));
    EXPECT_TRUE(logOutputContains("framesFBlocked 5"));
    EXPECT_TRUE(logOutputContains("maxFramesFBlocked 3"));
}

TEST_F(ARendererStatistics, tracksFramebufferAndOffscreenBufferSwapCounts)
{
    stats.framebufferSwapped(disp1);
    stats.offscreenBufferSwapped(disp1,  ob1, false);
    stats.framebufferSwapped(disp2);
    stats.offscreenBufferSwapped(disp2, ob2, false);
    stats.offscreenBufferSwapped(disp2, ob3, false);
    stats.frameFinished(0u);

    stats.framebufferSwapped(disp1);
    stats.offscreenBufferSwapped(disp1, ob1, false);
    stats.frameFinished(0u);

    EXPECT_TRUE(logOutputContains("FB1: 2; OB11: 2"));
    EXPECT_TRUE(logOutputContains("FB2: 1; OB22: 1; OB33: 1"));
}

TEST_F(ARendererStatistics, tracksInterruptibleOffscreenBuffer)
{
    stats.offscreenBufferInterrupted(disp1, ob1);
    stats.frameFinished(0u);
    stats.offscreenBufferSwapped(disp1, ob1, true);
    stats.frameFinished(0u);
    stats.offscreenBufferInterrupted(disp1, ob1);
    stats.frameFinished(0u);
    stats.offscreenBufferSwapped(disp1, ob1, true);
    stats.frameFinished(0u);

    EXPECT_TRUE(logOutputContains("OB11: 2 (intr: 2)"));
}

TEST_F(ARendererStatistics, untracksOffscreenBuffer)
{
    stats.offscreenBufferSwapped(disp1, ob1, false);
    stats.offscreenBufferSwapped(disp2, ob2, false);
    stats.offscreenBufferSwapped(disp2, ob3, false);
    stats.frameFinished(0u);
    stats.untrackOffscreenBuffer(disp2, ob2);

    EXPECT_FALSE(logOutputContains("OB22"));
}

TEST_F(ARendererStatistics, tracksStreamTextureSource)
{
    const StreamTextureSourceId src{ 99u };
    stats.streamTextureUpdated(src, 2u);
    stats.frameFinished(0u);
    stats.frameFinished(0u);
    stats.frameFinished(0u);
    stats.streamTextureUpdated(src, 9u);
    stats.frameFinished(0u);
    stats.streamTextureUpdated(src, 1u);
    stats.frameFinished(0u);

    EXPECT_TRUE(logOutputContains("numFrames 5"));
    EXPECT_TRUE(logOutputContains("SourceId 99: upd 12, framesUpd 3, maxUpdInFrame 9, maxFramesWithNoUpd 2"));
}

TEST_F(ARendererStatistics, logsValidNumbersWhenStreamTextureInactive)
{
    const StreamTextureSourceId src{ 99u };
    stats.streamTextureUpdated(src, 2u); // will register source
    stats.reset();
    stats.frameFinished(0u);
    stats.frameFinished(0u);
    stats.frameFinished(0u);

    EXPECT_TRUE(logOutputContains("SourceId 99: upd 0, framesUpd 0, maxUpdInFrame 0, maxFramesWithNoUpd 3"));
}

TEST_F(ARendererStatistics, untracksStreamTextureSource)
{
    const StreamTextureSourceId src{ 99u };
    stats.streamTextureUpdated(src, 2u);
    stats.frameFinished(0u);

    stats.untrackStreamTexture(src);

    EXPECT_FALSE(logOutputContains("SourceId 99"));
}

TEST_F(ARendererStatistics, tracksInterruptedFlushes)
{
    stats.flushApplyInterrupted(sceneId1);
    stats.flushApplyInterrupted(sceneId1);
    stats.flushApplyInterrupted(sceneId1);
    stats.frameFinished(0u);

    EXPECT_TRUE(logOutputContains("FApplyInterrupted 3"));
}

TEST_F(ARendererStatistics, doesNotLogInterruptedFlushesIfThereAreNone)
{
    stats.flushApplyInterrupted(sceneId1);
    stats.frameFinished(0u);
    EXPECT_TRUE(logOutputContains("FApplyInterrupted"));

    stats.reset();
    EXPECT_FALSE(logOutputContains("FApplyInterrupted"));
}

TEST_F(ARendererStatistics, tracksClientResourceUploads)
{
    stats.clientResourceUploaded(2u);
    stats.frameFinished(0u);
    EXPECT_TRUE(logOutputContains("clientResUploaded 1 (2 B)"));

    stats.reset();
    EXPECT_FALSE(logOutputContains("clientResUploaded"));

    stats.clientResourceUploaded(2u);
    stats.clientResourceUploaded(77u);
    stats.clientResourceUploaded(100u);
    stats.frameFinished(0u);
    EXPECT_TRUE(logOutputContains("clientResUploaded 3 (179 B)"));

    stats.reset();
    EXPECT_FALSE(logOutputContains("clientResUploaded"));
}

TEST_F(ARendererStatistics, tracksSceneResourceUploads)
{
    stats.sceneResourceUploaded(sceneId1, 2u);
    stats.frameFinished(0u);
    EXPECT_TRUE(logOutputContains("RSUploaded 1 (2 B)"));

    stats.reset();
    EXPECT_FALSE(logOutputContains("RSUploaded"));

    stats.sceneResourceUploaded(sceneId1, 2u);
    stats.sceneResourceUploaded(sceneId1, 77u);
    stats.sceneResourceUploaded(sceneId2, 100u);
    stats.frameFinished(0u);
    EXPECT_TRUE(logOutputContains("RSUploaded 2 (79 B)")); //scene1
    EXPECT_TRUE(logOutputContains("RSUploaded 1 (100 B)")); //scene2

    stats.reset();
    EXPECT_FALSE(logOutputContains("RSUploaded"));
}

TEST_F(ARendererStatistics, tracksShaderCompilationAndTimes)
{
    stats.shaderCompiled(2u);
    stats.frameFinished(0u);
    EXPECT_TRUE(logOutputContains("shadersCompiled 1"));
    EXPECT_TRUE(logOutputContains("for total ms:0"));

    stats.reset();
    EXPECT_FALSE(logOutputContains("shadersCompiled"));

    stats.shaderCompiled(3000u);
    stats.shaderCompiled(5000u);
    stats.shaderCompiled(7000u);
    stats.frameFinished(0u);
    EXPECT_TRUE(logOutputContains("shadersCompiled 3"));
    EXPECT_TRUE(logOutputContains("for total ms:15"));

    stats.reset();
    EXPECT_FALSE(logOutputContains("shadersCompiled"));
}

TEST_F(ARendererStatistics, confidenceTest_fullLogOutput)
{
    for (size_t period = 0u; period < 2u; ++period)
    {
        stats.trackArrivedFlush(sceneId1, 123, 5, 3, 4);
        stats.trackArrivedFlush(sceneId2, 6, 7, 8, 9);
        stats.flushBlocked(sceneId1);
        stats.flushApplied(sceneId2);
        stats.framebufferSwapped(disp1);
        stats.frameFinished(0);

        stats.clientResourceUploaded(2u);
        stats.clientResourceUploaded(77u);
        stats.shaderCompiled(0u);
        stats.shaderCompiled(1000u);

        stats.trackArrivedFlush(sceneId1, 123, 5, 3, 4);
        stats.flushBlocked(sceneId1);
        stats.sceneRendered(sceneId2);
        stats.framebufferSwapped(disp2);
        stats.frameFinished(100);

        stats.trackArrivedFlush(sceneId1, 123, 5, 3, 4);
        stats.trackArrivedFlush(sceneId2, 6, 7, 8, 9);
        stats.flushApplied(sceneId1);
        stats.flushBlocked(sceneId2);
        stats.sceneRendered(sceneId1);
        stats.sceneRendered(sceneId2);
        stats.framebufferSwapped(disp1);
        stats.offscreenBufferInterrupted(disp1, ob1);
        stats.framebufferSwapped(disp2);
        stats.frameFinished(0);

        stats.sceneResourceUploaded(sceneId1, 3u);
        stats.sceneResourceUploaded(sceneId1, 77u);
        stats.sceneResourceUploaded(sceneId2, 200u);

        stats.sceneRendered(sceneId1);
        stats.framebufferSwapped(disp1);
        stats.offscreenBufferSwapped(disp1, ob1, true);
        stats.frameFinished(0);

        EXPECT_TRUE(logOutputContains("Avg framerate: "));
        EXPECT_TRUE(logOutputContains("FPS [minFrameTime "));
        EXPECT_TRUE(logOutputContains("us, maxFrameTime "));
        EXPECT_TRUE(logOutputContains("], drawcallsPerFrame 25, numFrames 4"));
        EXPECT_TRUE(logOutputContains("clientResUploaded 2 (79 B)"));
        EXPECT_TRUE(logOutputContains("shadersCompiled 2"));
        EXPECT_TRUE(logOutputContains("for total ms:1"));
        EXPECT_TRUE(logOutputContains("FB1: 3; OB11: 1 (intr: 1)"));
        EXPECT_TRUE(logOutputContains("FB2: 2"));
        EXPECT_TRUE(logOutputContains("Scene 11: rendered 2, framesFArrived 3, framesFApplied 1, framesFBlocked 2, maxFramesWithNoFApplied 2, maxFramesFBlocked 2, FArrived 3, FApplied 1, actions/F (123/123/123.000000), RC+/F (5/5/5.000000), RC-/F (3/3/3.000000), RS/F (4/4/4.000000), RSUploaded 2 (80 B)"));
        EXPECT_TRUE(logOutputContains("Scene 22: rendered 2, framesFArrived 2, framesFApplied 1, framesFBlocked 1, maxFramesWithNoFApplied 3, maxFramesFBlocked 1, FArrived 2, FApplied 1, actions/F (6/6/6.000000), RC+/F (7/7/7.000000), RC-/F (8/8/8.000000), RS/F (9/9/9.000000), RSUploaded 1 (200 B)"));

        stats.reset();
    }
}
