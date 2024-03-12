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
        void                            startTestApplication(uint32_t testAppIdx = 0u);
        void                            initializeTestApplication(EConnectionMode connectionMode = EConnectionMode::DisplayName, uint32_t testAppIdx = 0u);
        void                            startTestApplicationAndWaitUntilConnected(EConnectionMode connectionMode = EConnectionMode::DisplayName, uint32_t displayIdx = 0u, uint32_t testAppIdx = 0u);
        void                            stopTestApplicationAndWaitUntilDisconnected(uint32_t displayIdx = 0u, uint32_t testAppIdx = 0u);
        void                            killTestApplication(uint32_t testAppIdx = 0u);

        //wait for events
        void                            waitForContentOnStreamTexture(WaylandIviSurfaceId iviSurface, uint32_t displayIdx = 0u);
        void                            waitForUnavailablilityOfContentOnStreamTexture(WaylandIviSurfaceId iviSurface);
        void                            waitForSurfaceAvailableForStreamTexture(WaylandIviSurfaceId iviSurface);
        void                            waitForSurfaceUnavailableForStreamTexture(WaylandIviSurfaceId iviSurface);
        bool                            waitForStreamSurfaceAvailabilityChange(WaylandIviSurfaceId iviSurface, bool available);
        Bool                            waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId iviSurface, UInt64 numberOfComittedBuffers, UInt32 timeoutMilliseconds = std::numeric_limits<ramses_internal::UInt32>::max());
        void                            waitForBufferAttachedToIviSurface(WaylandIviSurfaceId iviSurface);
        void                            waitForNoBufferAttachedToIviSurface(WaylandIviSurfaceId iviSurface);
        void                            waitUntilNumberOfCompositorConnections(UInt32 numberOfConnections, bool doResourceUpdate = false, uint32_t displayIdx = 0u);

        //send message to test app
        TestApplicationSurfaceId        sendCreateSurfaceWithEGLContextToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval, uint32_t testAppIdx = 0u);
        TestApplicationShellSurfaceId   sendCreateShellSurfaceToTestApplication(TestApplicationSurfaceId surfaceId, uint32_t testAppIdx = 0u);
        void                            sendSetShellSurfaceTitleToTestApplication(TestApplicationShellSurfaceId, const String& title, uint32_t testAppIdx = 0u);
        void                            sendSetShellSurfaceDummyValuesToTestApplication(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId, uint32_t testAppIdx = 0u);
        void                            sendDestroyShellSurfaceToTestApplication(TestApplicationShellSurfaceId shellSurfaceId, uint32_t testAppIdx = 0u);
        TestApplicationSurfaceId        sendCreateSurfaceWithoutEGLContextToTestApplication(UInt32 width, UInt32 height, uint32_t testAppIdx = 0u);
        void                            sendDestroySurfaceToTestApplication(TestApplicationSurfaceId surfaceId, uint32_t testAppIdx = 0u);
        void                            sendCreateIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId, WaylandIviSurfaceId iviSurface, uint32_t testAppIdx = 0u);
        void                            sendDestroyIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId, uint32_t testAppIdx = 0u);
        void                            sendRenderOneFrameToEGLBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool waitOnFramecallback = false, uint32_t testAppIdx = 0u);
        void                            sendRenderOneFrameToSharedMemoryBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool waitOnFramecallback = false, uint32_t testAppIdx = 0u);
        void                            sendAttachBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool commit = false, uint32_t testAppIdx = 0u);
        void                            sendReAttachBufferToTestApplication(TestApplicationSurfaceId surfaceId, uint32_t count, uint32_t testAppIdx = 0u);
        void                            sendDestroyBuffersToTestApplication(uint32_t testAppIdx = 0u);
        void                            sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2, uint32_t testAppIdx = 0u);
        void                            sendAdditionalConnectToEmbeddedCompositorToTestApplication(uint32_t testAppIdx = 0u);
        void                            sendDetachBufferFromSurfaceToTestApplication(TestApplicationSurfaceId surfaceId, uint32_t testAppIdx = 0u);
        void                            sendSetSurfaceSizeToTestApplicaton(TestApplicationSurfaceId surfaceId, UInt32 width, UInt32 height, uint32_t testAppIdx = 0u);
        void                            sendSetTriangleColorToTestApplication(ETriangleColor color, uint32_t testAppIdx = 0u);
        bool                            sendStartRamsesRendererAndRunRenderingTest(uint32_t testAppIdx = 0u);
        void                            sendSetRequiredWaylandOutputVersion(uint32_t protocolVersion, uint32_t testAppIdx = 0u);

        //get message from test app
        UInt32                          getNumberOfAllocatedSHMBufferFromTestApplication(uint32_t testAppIdx = 0u);
        Bool                            getIsBufferFreeFromTestApplication(UInt32 buffer, uint32_t testAppIdx = 0u);
        Bool                            getWaylandOutputParamsFromTestApplication(WaylandOutputTestParams& resultWaylandOutputParams, uint32_t testAppIdx = 0u);

        //local renderer
        void                            renderOneFrame();
        void                            setSurfaceVisibility(WaylandIviSurfaceId iviSurface, bool visibility);
        String                          getTitleOfIviSurface(WaylandIviSurfaceId iviSurface);
        void                            logEmbeddedCompositor(RendererLogContext& logContext);

        // This is needed due to the conflict resulting from mandating the possibility to set EC config on both RendererConfig
        // and DisplayConfig, as well as parsing EC config from cmd line to RendererConfig
        const String&                   getEmbeddedCompositingSocketGroupName() const;

        const static String             TestEmbeddedCompositingDisplayName;
        const static String             TestAlternateEmbeddedCompositingDisplayName;

    protected:
        TestApplicationSurfaceId sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval, Bool useEGL, uint32_t testAppIdx);
        void sendStopToTestApplication(uint32_t testAppIdx);

        IEmbeddedCompositingManager& getEmbeddedCompositorManager(uint32_t displayIdx = 0u);
        IEmbeddedCompositor& getEmbeddedCompositor(uint32_t displayIdx = 0u);

        TestForkingController& m_testForkingController;
        TestApplicationSurfaceId m_nextSurfaceId = TestApplicationSurfaceId(0);
        TestApplicationShellSurfaceId m_nextShellSurfaceId = TestApplicationShellSurfaceId(0);
        const String m_embeddedCompositingSocketGroupName;
    };
}

#endif
