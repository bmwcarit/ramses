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
    enum ETestForkerApplicationMessage
    {
        ETestForkerApplicationMessage_StopForkerApplication = 0,
        ETestForkerApplicationMessage_ForkTestApplication,
        ETestForkerApplicationMessage_WaitForTestApplicationExit,
        ETestForkerApplicationMessage_KillTestApplication
    };

    enum ETestWaylandApplicationMessage
    {
        ETestWaylandApplicationMessage_StopApplication = 0,
        ETestWaylandApplicationMessage_CreateSurface,
        ETestWaylandApplicationMessage_CreateShellSurface,
        ETestWaylandApplicationMessage_DestroyShellSurface,
        ETestWaylandApplicationMessage_SetShellSurfaceTitle,
        ETestWaylandApplicationMessage_SetShellSurfaceDummyValues,
        ETestWaylandApplicationMessage_DestroySurface,
        ETestWaylandApplicationMessage_DestroyIVISurface,
        ETestWaylandApplicationMessage_CreateIVISurface,
        ETestWaylandApplicationMessage_RenderOneFrame,
        ETestWaylandApplicationMessage_AttachBuffer,
        ETestWaylandApplicationMessage_DestroyBuffers,
        ETestWaylandApplicationMessage_SetSurfaceSize,
        ETestWaylandApplicationMessage_SetTriangleColor,
        ETestWaylandApplicationMessage_AdditionalConnectToEmbeddedCompositor,
        ETestWaylandApplicationMessage_DetachBufferFromSurface,
        ETestWaylandApplicationMessage_GetNumberOfAllocatedSHMBuffer,
        ETestWaylandApplicationMessage_RenderOneFrameToTwoSurfaces,
        ETestWaylandApplicationMessage_GetIsBufferFree,
        ETestWaylandApplicationMessage_GetOutputValues
    };
}

#endif
