//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITINGTESTMESSAGES_H
#define RAMSES_EMBEDDEDCOMPOSITINGTESTMESSAGES_H

namespace ramses_internal
{
    enum class ETestForkerApplicationMessage
    {
        StopForkerApplication,
        ForkTestApplication,
        WaitForTestApplicationExit,
        KillTestApplication,
    };

    enum class ETestWaylandApplicationMessage
    {
        InitializeTestApplication,
        StopApplication,
        CreateSurface,
        CreateShellSurface,
        DestroyShellSurface,
        SetShellSurfaceTitle,
        SetShellSurfaceDummyValues,
        DestroySurface,
        DestroyIVISurface,
        CreateIVISurface,
        RenderOneFrame_ToEGLBuffer,
        RenderOneFrame_ToSharedMemoryBuffer,
        AttachBuffer,
        ReAttachBuffer,
        DestroyBuffers,
        SetSurfaceSize,
        SetTriangleColor,
        AdditionalConnectToEmbeddedCompositor,
        DetachBufferFromSurface,
        GetNumberOfAllocatedSHMBuffer,
        RenderOneFrameToTwoSurfaces,
        GetIsBufferFree,
        GetWaylandOutputParams,
        SetRequiredWaylandOutputVersion,
        StartRamsesRendererAndRunRenderingTest,
    };
}

#endif
