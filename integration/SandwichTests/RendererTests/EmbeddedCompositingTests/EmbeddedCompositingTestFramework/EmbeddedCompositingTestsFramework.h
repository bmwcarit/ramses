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
    class RendererLogContext;

    class EmbeddedCompositingTestsFramework: public RendererTestsFramework
    {
    public:
        EmbeddedCompositingTestsFramework(bool generateScreenshots, TestForkingController& testForkingController, const ramses::RamsesFrameworkConfig& config);

        //control test app lifecycle
        void                            startTestApplication();
        void                            startTestApplicationAndWaitUntilConnected();
        void                            stopTestApplicationAndWaitUntilDisconnected();
        void                            killTestApplication();
        void                            setEnvironmentVariableWaylandSocket();
        void                            setEnvironmentVariableWaylandDisplay();

        //wait for events
        void                            waitForContentOnStreamTexture(StreamTextureSourceId sourceId);
        void                            waitForUnavailablilityOfContentOnStreamTexture(StreamTextureSourceId sourceId);
        void                            waitForSurfaceAvailableForStreamTexture(StreamTextureSourceId sourceId);
        void                            waitForSurfaceUnavailableForStreamTexture(StreamTextureSourceId sourceId);
        bool                            waitForStreamSurfaceAvailabilityChange(StreamTextureSourceId streamSource, bool available);
        Bool                            waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId waylandSurfaceId, UInt64 numberOfComittedBuffers, UInt32 timeoutMilliseconds = std::numeric_limits<ramses_internal::UInt32>::max());
        void                            waitForBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void                            waitForNoBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void                            waitUntilNumberOfCompositorConnections(UInt32 numberOfConnections, bool doResourceUpdate = false);

        //send message to test app
        TestApplicationSurfaceId        sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval);
        TestApplicationShellSurfaceId   sendCreateShellSurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendSetShellSurfaceTitleToTestApplication(TestApplicationShellSurfaceId, const String& title);
        void                            sendSetShellSurfaceDummyValuesToTestApplication(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId);
        void                            sendDestroyShellSurfaceToTestApplication(TestApplicationShellSurfaceId shellSurfaceId);
        TestApplicationSurfaceId        sendCreateSharedMemorySurfaceToTestApplication(UInt32 width, UInt32 height);
        void                            sendDestroySurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendCreateIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId, WaylandIviSurfaceId newSurfaceIviId);
        void                            sendDestroyIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendRenderOneFrameToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendAttachBufferToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendDestroyBuffersToTestApplication();
        void                            sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2);
        void                            sendAdditionalConnectToEmbeddedCompositorToTestApplication();
        void                            sendDetachBufferFromSurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendSetSurfaceSizeToTestApplicaton(TestApplicationSurfaceId surfaceId, UInt32 width, UInt32 height);
        void                            sendSetTriangleColorToTestApplication(ETriangleColor color);
        bool                            sendStartRamsesRendererAndRunRenderingTest();

        //get message from test app
        UInt32                          getNumberOfAllocatedSHMBufferFromTestApplication();
        Bool                            getIsBufferFreeFromTestApplication(UInt32 buffer);

        //local renderer
        void                            renderOneFrame();
        void                            setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility);
        String                          getTitleOfIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void                            logEmbeddedCompositor(RendererLogContext& logContext);

    protected:
        TestApplicationSurfaceId sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval, Bool useEGL);
        void sendStopToTestApplication();

        IEmbeddedCompositingManager& getEmbeddedCompositorManager();
        IEmbeddedCompositor& getEmbeddedCompositor();

        TestForkingController& m_testForkingController;
        TestApplicationSurfaceId m_nextSurfaceId = TestApplicationSurfaceId(0);
        TestApplicationShellSurfaceId m_nextShellSurfaceId = TestApplicationShellSurfaceId(0);
    };
}

#endif
