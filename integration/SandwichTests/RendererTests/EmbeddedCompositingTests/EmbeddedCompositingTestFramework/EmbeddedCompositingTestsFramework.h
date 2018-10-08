//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITINGTESTSFRAMEWORK_H
#define RAMSES_EMBEDDEDCOMPOSITINGTESTSFRAMEWORK_H

#include "RendererTestsFramework.h"
#include "RendererAPI/Types.h"
#include "ETriangleColor.h"
#include "TestApplicationSurfaceId.h"
#include "TestApplicationShellSurfaceId.h"

namespace ramses_internal
{
    class IEmbeddedCompositor;
    class IEmbeddedCompositingManager;
    class TestForkingController;

    class EmbeddedCompositingTestsFramework: public RendererTestsFramework
    {
    public:
        EmbeddedCompositingTestsFramework(bool generateScreenshots, TestForkingController& testForkingController, const ramses::RamsesFrameworkConfig& config);
        ~EmbeddedCompositingTestsFramework();

        void startTestApplicationAndWaitUntilConnected();
        void stopTestApplicationAndWaitUntilDisconnected();
        void showSceneAndWaitForShowing(ramses::sceneId_t sceneId);
        void waitForSceneUnmapped(ramses::sceneId_t sceneId);
        void waitForContentOnStreamTexture(StreamTextureSourceId sourceId);
        void waitForUnavailablilityOfContentOnStreamTexture(StreamTextureSourceId sourceId);
        void waitForSurfaceAvailableForStreamTexture(StreamTextureSourceId sourceId);
        void waitForSurfaceUnavailableForStreamTexture(StreamTextureSourceId sourceId);
        bool waitForStreamSurfaceAvailabilityChange(StreamTextureSourceId streamSource, bool available);
        void sendStopToTestApplication();
        TestApplicationSurfaceId sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval);
        TestApplicationShellSurfaceId sendCreateShellSurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void sendSetShellSurfaceTitleToTestApplication(TestApplicationShellSurfaceId, const String& title);
        void sendSetShellSurfaceDummyValuesToTestApplication(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId);
        void sendDestroyShellSurfaceToTestApplication(TestApplicationShellSurfaceId shellSurfaceId);
        TestApplicationSurfaceId sendCreateSharedMemorySurfaceToTestApplication(UInt32 width, UInt32 height);
        void sendDestroySurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void sendCreateIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId, WaylandIviSurfaceId newSurfaceIviId);
        void sendDestroyIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void sendRenderOneFrameToTestApplication(TestApplicationSurfaceId surfaceId);
        void sendAttachBufferToTestApplication(TestApplicationSurfaceId surfaceId);
        void sendDestroyBuffersToTestApplication();
        void sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId);
        void sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2);
        void sendAdditionalConnectToEmbeddedCompositorToTestApplication();
        void sendDetachBufferFromSurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        UInt32 getNumberOfAllocatedSHMBufferFromTestApplication();
        Bool getIsBufferFreeFromTestApplication(UInt32 buffer);
        void getOutputValuesFromTestApplication(Int32& width, Int32& height, Int32& scale, UInt32& doneCount);
        void setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility);
        Bool waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId waylandSurfaceId, UInt64 numberOfComittedBuffers, UInt32 timeoutMilliseconds = std::numeric_limits<ramses_internal::UInt32>::max());
        void waitForBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void waitForNoBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void waitUntilNumberOfCompositorConnections(UInt32 numberOfConnections, bool doResourceUpdate = false);
        void renderOneFrame();
        void sendSetSurfaceSizeToTestApplicaton(TestApplicationSurfaceId surfaceId, UInt32 width, UInt32 height);
        String getTitleOfIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void getOutputResolutionFromEmbeddedCompositor(Int32& width, Int32& height);
        void sendSetTriangleColorToTestApplication(ETriangleColor color);

        void killTestApplication();
        IEmbeddedCompositor& getEmbeddedCompositor();

    protected:
        TestApplicationSurfaceId sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval, Bool useEGL);

        IEmbeddedCompositingManager& getEmbeddedCompositorManager();

        TestForkingController& m_testForkingController;
        TestApplicationSurfaceId m_nextSurfaceId = TestApplicationSurfaceId(0);
        TestApplicationShellSurfaceId m_nextShellSurfaceId = TestApplicationShellSurfaceId(0);
    };
}

#endif
