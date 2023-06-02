//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SINGLESTREAMTEXTURETESTS_H
#define RAMSES_SINGLESTREAMTEXTURETESTS_H

#include "IEmbeddedCompositingTest.h"

#include <string>

namespace ramses_internal
{
    class SingleStreamTextureTests : public IEmbeddedCompositingTest
    {
    public:
        SingleStreamTextureTests();
        void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            ShowFallbackTexture = 0,
            ShowStreamTexture,
            StreamTextureWithDifferentSizeFromFallbackTexture,
            ShowStreamTextureWithTexCoordsOffset,
            ShowFallbackTextureAfterSurfaceIsDestroyed,
            StreamTextureContentAvailableBeforeSceneCreated,
            TestCorrectNumberOfCommitedFrames,
            ShowStreamTextureAfterChangingSurfaceSize,
            ShowStreamTextureAfterRecreatingSurfaceWithDifferentSize,
            ShowStreamTextureAfterRecreatingScene,
            ShowFallbackTextureWhenNoSurfaceUpdate,
            EachFrameOfClientIsSynchronouslyDisplayedByRenderer,
            SwapIntervalZeroDoesNotBlockClient,
            SwapIntervalOneBlocksClient,
            ShowStreamTextureAfterClientRecreated,
            ShowStreamTextureAfterSurfaceRecreated,
            ClientReceivesFrameCallback,
            ClientCanBindMultipleTimesToEmbeddedCompositor,
            ShowFallbackTextureWhenClientIsKilled,
            ClientCreatesTwoSurfacesWithSameIVIIdSendsErrorToClient,
            ClientUsesShellSurface,
            ShowFallbackTextureWhenIVISurfaceDestroyed,
            ClientCanNotCreateTwoIVISurfacesWithSameIdForSameSurface,
            ClientCanNotCreateTwoIVISurfacesWithDifferentIdsForSameSurface,
            ClientRecreatesIVISurfaceWithSameId,
            ShowStreamTextureWhenIVISurfaceIsCreatedAfterUpdate,
            ClientCanNotUseShellSurfaceWhenSurfaceHasBeenDeleted,
            ClientCanNotCreateTwoShellSurfacesForSameSurface,
            SurfaceHasNoTitleWhenShellSurfaceDestroyed,
            CompositorLogsInfo,
        };

        bool recreateSurfaceRenderAndCheckOneFrame(EmbeddedCompositingTestsFramework& testFramework, TestApplicationSurfaceId& testSurfaceIdOut, UInt32 surfaceWidth, UInt32 surfaceHeight, WaylandIviSurfaceId waylandSurfaceIviId, const std::string& expectedImageName);
        bool resizeSurfaceRenderAndCheckOneFrame(EmbeddedCompositingTestsFramework& testFramework, TestApplicationSurfaceId testSurfaceId, UInt32 surfaceWidth, UInt32 surfaceHeight, WaylandIviSurfaceId waylandSurfaceIviId, UInt32 frameCount, const std::string& expectedImageName);
        bool renderFramesOnTestApplicationAndTakeScreenshots(EmbeddedCompositingTestsFramework& testFramework, TestApplicationSurfaceId testSurfaceId, WaylandIviSurfaceId waylandSurfaceIviId, const UInt32 frameCountToRender, const ETriangleColor triangleColors[], const std::string screenshotFiles[], const UInt32 triangleColorCount);
    };
}

#endif
