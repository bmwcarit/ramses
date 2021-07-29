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
#include "WaylandOutputTestParams.h"
#include "SceneAPI/WaylandIviSurfaceId.h"

namespace ramses_internal
{
    class IEmbeddedCompositor;
    class IEmbeddedCompositingManager;
    class TestForkingController;
    class RendererLogContext;

    enum class EConnectionMode
    {
        DisplayName,
        AlternateDisplayName,
        DisplayFD,
    };

    class EmbeddedCompositingTestsFramework: public RendererTestsFramework
    {
    public:
        EmbeddedCompositingTestsFramework(bool generateScreenshots, TestForkingController& testForkingController, const ramses::RamsesFrameworkConfig& config, const String& embeddedCompositingSocketGroupName);

        //control test app lifecycle
        void                            startTestApplication();
        void                            initializeTestApplication(EConnectionMode connectionMode = EConnectionMode::DisplayName);
        void                            startTestApplicationAndWaitUntilConnected(EConnectionMode connectionMode = EConnectionMode::DisplayName, uint32_t displayIdx = 0u);
        void                            stopTestApplicationAndWaitUntilDisconnected(uint32_t displayIdx = 0u);
        void                            killTestApplication();

        //wait for events
        void                            waitForContentOnStreamTexture(WaylandIviSurfaceId sourceId, uint32_t displayIdx = 0u);
        void                            waitForUnavailablilityOfContentOnStreamTexture(WaylandIviSurfaceId sourceId);
        void                            waitForSurfaceAvailableForStreamTexture(WaylandIviSurfaceId sourceId);
        void                            waitForSurfaceUnavailableForStreamTexture(WaylandIviSurfaceId sourceId);
        bool                            waitForStreamSurfaceAvailabilityChange(WaylandIviSurfaceId streamSource, bool available);
        Bool                            waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId waylandSurfaceId, UInt64 numberOfComittedBuffers, UInt32 timeoutMilliseconds = std::numeric_limits<ramses_internal::UInt32>::max());
        void                            waitForBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void                            waitForNoBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void                            waitUntilNumberOfCompositorConnections(UInt32 numberOfConnections, bool doResourceUpdate = false, uint32_t displayIdx = 0u);

        //send message to test app
        TestApplicationSurfaceId        sendCreateSurfaceWithEGLContextToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval);
        TestApplicationShellSurfaceId   sendCreateShellSurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendSetShellSurfaceTitleToTestApplication(TestApplicationShellSurfaceId, const String& title);
        void                            sendSetShellSurfaceDummyValuesToTestApplication(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId);
        void                            sendDestroyShellSurfaceToTestApplication(TestApplicationShellSurfaceId shellSurfaceId);
        TestApplicationSurfaceId        sendCreateSurfaceWithoutEGLContextToTestApplication(UInt32 width, UInt32 height);
        void                            sendDestroySurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendCreateIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId, WaylandIviSurfaceId newSurfaceIviId);
        void                            sendDestroyIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendRenderOneFrameToEGLBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool waitOnFramecallback = false);
        void                            sendRenderOneFrameToSharedMemoryBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool waitOnFramecallback = false);
        void                            sendAttachBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool commit = false);
        void                            sendDestroyBuffersToTestApplication();
        void                            sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2);
        void                            sendAdditionalConnectToEmbeddedCompositorToTestApplication();
        void                            sendDetachBufferFromSurfaceToTestApplication(TestApplicationSurfaceId surfaceId);
        void                            sendSetSurfaceSizeToTestApplicaton(TestApplicationSurfaceId surfaceId, UInt32 width, UInt32 height);
        void                            sendSetTriangleColorToTestApplication(ETriangleColor color);
        bool                            sendStartRamsesRendererAndRunRenderingTest();
        void                            sendSetRequiredWaylandOutputVersion(uint32_t protocolVersion);

        //get message from test app
        UInt32                          getNumberOfAllocatedSHMBufferFromTestApplication();
        Bool                            getIsBufferFreeFromTestApplication(UInt32 buffer);
        Bool                            getWaylandOutputParamsFromTestApplication(WaylandOutputTestParams& resultWaylandOutputParams);

        //local renderer
        void                            renderOneFrame();
        void                            setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility);
        String                          getTitleOfIviSurface(WaylandIviSurfaceId waylandSurfaceId);
        void                            logEmbeddedCompositor(RendererLogContext& logContext);

        // This is needed due to the conflict resulting from mandating the possibility to set EC config on both RendererConfig
        // and DisplayConfig, as well as parsing EC config from cmd line to RendererConfig
        const String&                   getEmbeddedCompositingSocketGroupName() const;

        const static String             TestEmbeddedCompositingDisplayName;
        const static String             TestAlternateEmbeddedCompositingDisplayName;

    protected:
        TestApplicationSurfaceId sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval, Bool useEGL);
        void sendStopToTestApplication();

        IEmbeddedCompositingManager& getEmbeddedCompositorManager(uint32_t displayIdx = 0u);
        IEmbeddedCompositor& getEmbeddedCompositor(uint32_t displayIdx = 0u);

        TestForkingController& m_testForkingController;
        TestApplicationSurfaceId m_nextSurfaceId = TestApplicationSurfaceId(0);
        TestApplicationShellSurfaceId m_nextShellSurfaceId = TestApplicationShellSurfaceId(0);
        const String m_embeddedCompositingSocketGroupName;
    };
}

#endif
