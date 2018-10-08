//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositingTestsFramework.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RamsesRendererImpl.h"
#include "RendererTestEventHandler.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "EmbeddedCompositingTestMessages.h"
#include "TestSignalHandler.h"
#include "ETriangleColor.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "TestForkingController.h"

namespace ramses_internal
{
    EmbeddedCompositingTestsFramework::EmbeddedCompositingTestsFramework(bool generateScreenshots, TestForkingController& testForkingController, const ramses::RamsesFrameworkConfig& config)
        : RendererTestsFramework(generateScreenshots, config)
        , m_testForkingController(testForkingController)
    {
        TestSignalHandler::RegisterSignalHandlersForCurrentProcess("EmbeddedCompositingTestsFramework");
    }

    EmbeddedCompositingTestsFramework::~EmbeddedCompositingTestsFramework()
    {
        m_testForkingController.deinitialize();

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::~EmbeddedCompositingTestsFramework forking controller terminated");
    }

    void EmbeddedCompositingTestsFramework::startTestApplicationAndWaitUntilConnected()
    {
        m_testForkingController.startTestApplication();
        waitUntilNumberOfCompositorConnections(1);
    }

    void EmbeddedCompositingTestsFramework::stopTestApplicationAndWaitUntilDisconnected()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::stopTestApplicationAndWaitUntilDisconnected waiting until client test application has terminated ...");
        sendStopToTestApplication();
        waitUntilNumberOfCompositorConnections(0);

        // When TestWaylandApplication has disconnected from the EC, it can still run, so wait on the final answer, that
        // it has really reached it's end. For example, in the test case ClientCreatesTwoSurfaceWithSameIVIId the client
        // connection is already terminated by an error, before stopTestApplicationAndWaitUntilDisconnected is called.
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::stopTestApplicationAndWaitUntilDisconnected waiting on confirmation ... ");
        m_testForkingController.waitForTestApplicationExit();
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::stopTestApplicationAndWaitUntilDisconnected stop confirmation received");
    }

    void EmbeddedCompositingTestsFramework::showSceneAndWaitForShowing(ramses::sceneId_t sceneId)
    {
        getTestRenderer().showScene(sceneId);
    }

    void EmbeddedCompositingTestsFramework::waitForSceneUnmapped(ramses::sceneId_t sceneId)
    {
        getTestRenderer().waitForUnmapped(sceneId);
    }

    void EmbeddedCompositingTestsFramework::waitForContentOnStreamTexture(StreamTextureSourceId sourceId)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();
        IEmbeddedCompositingManager& embeddedCompositorManager = getEmbeddedCompositorManager();

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForContentOnStreamTexture(): waiting for content on stream source id :" << sourceId.getValue());

        while (!embeddedCompositor.isContentAvailableForStreamTexture(sourceId))
        {
            embeddedCompositorManager.processClientRequests();
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForContentOnStreamTexture(): content found on stream source id :" << sourceId.getValue());
    }

    void EmbeddedCompositingTestsFramework::waitForUnavailablilityOfContentOnStreamTexture(StreamTextureSourceId sourceId)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();
        IEmbeddedCompositingManager& embeddedCompositorManager = getEmbeddedCompositorManager();

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForUnavailablilityOfContentFromStreamTexture(): waiting for unavailability of content on stream source id :" << sourceId.getValue());

        while (embeddedCompositor.isContentAvailableForStreamTexture(sourceId))
        {
            embeddedCompositorManager.processClientRequests();
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForUnavailablilityOfContentFromStreamTexture(): no content on stream source id :" << sourceId.getValue());
    }

    Bool EmbeddedCompositingTestsFramework::waitUntilNumberOfCommitedFramesForIviSurface(WaylandIviSurfaceId waylandSurfaceId, UInt64 numberOfComittedBuffers, UInt32 timeoutMilliseconds)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();
        IEmbeddedCompositingManager& embeddedCompositingManager = getEmbeddedCompositorManager();

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitUntilNumberOfCommitedBuffersForIviSurface(): waiting for number of commited buffers for ivi surface " << waylandSurfaceId.getValue() << " reaching " << numberOfComittedBuffers);

        const UInt64 startTime = PlatformTime::GetMillisecondsMonotonic();
        while (embeddedCompositor.getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(waylandSurfaceId) != numberOfComittedBuffers)
        {
            embeddedCompositingManager.processClientRequests();

            const UInt32 timeElapsed = static_cast<UInt32>(PlatformTime::GetMillisecondsMonotonic() - startTime);
            if(timeElapsed > timeoutMilliseconds)
            {
                LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitUntilNumberOfCommitedFramesForIviSurfaceWithTimeout(): timed out after " << timeElapsed << " ms");
                return false;
            }
        }
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitUntilNumberOfCommitedBuffersForIviSurface(): number of commited buffers for ivi surface " << waylandSurfaceId.getValue() << " is " << embeddedCompositor.getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(waylandSurfaceId));
        return true;
    }

    String EmbeddedCompositingTestsFramework::getTitleOfIviSurface(WaylandIviSurfaceId waylandSurfaceId)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();
        return embeddedCompositor.getTitleOfWaylandIviSurface(waylandSurfaceId);
    }

    void EmbeddedCompositingTestsFramework::getOutputResolutionFromEmbeddedCompositor(Int32& width, Int32& height)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();
        embeddedCompositor.getOutputResolution(width, height);
    }

    void EmbeddedCompositingTestsFramework::waitUntilNumberOfCompositorConnections(uint32_t numberOfConnections, bool doResourceUpdate)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();
        IEmbeddedCompositingManager& embeddedCompositingManager = getEmbeddedCompositorManager();

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitUntilNumberOfCompositorConnections(): waiting for number of connections reaching " << numberOfConnections);

        while (embeddedCompositor.getNumberOfCompositorConnections() != numberOfConnections)
        {
            embeddedCompositingManager.processClientRequests();
            if (doResourceUpdate)
            {
                UpdatedSceneIdSet updatedScenes;
                StreamTextureBufferUpdates unused;
                embeddedCompositingManager.uploadResourcesAndGetUpdates(updatedScenes, unused);
                embeddedCompositingManager.notifyClients();
            }
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitUntilNumberOfCompositorConnections(): number of compositor connections is " << numberOfConnections);
    }

    void EmbeddedCompositingTestsFramework::waitForBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId)
    {
        const IEmbeddedCompositor&   embeddedCompositor         = getEmbeddedCompositor();
        IEmbeddedCompositingManager& embeddedCompositingManager = getEmbeddedCompositorManager();

        LOG_INFO(CONTEXT_RENDERER,
                 "EmbeddedCompositingTestsFramework::waitForBufferAttachedToIviSurface(): waiting for "
                 "buffer attached to ivi surface "
                     << waylandSurfaceId.getValue());

        while (!embeddedCompositor.isBufferAttachedToWaylandIviSurface(waylandSurfaceId))
        {
            embeddedCompositingManager.processClientRequests();
        }
        LOG_INFO(CONTEXT_RENDERER,
                 "EmbeddedCompositingTestsFramework::waitForBufferAttachedToIviSurface(): buffer is attached");
    }

    void EmbeddedCompositingTestsFramework::waitForNoBufferAttachedToIviSurface(WaylandIviSurfaceId waylandSurfaceId)
    {
        const IEmbeddedCompositor&   embeddedCompositor         = getEmbeddedCompositor();
        IEmbeddedCompositingManager& embeddedCompositingManager = getEmbeddedCompositorManager();

        LOG_INFO(CONTEXT_RENDERER,
                 "EmbeddedCompositingTestsFramework::waitForNoBufferAttachedToIviSurface(): waiting for "
                 "no buffer attached to ivi surface "
                     << waylandSurfaceId.getValue());

        while (embeddedCompositor.isBufferAttachedToWaylandIviSurface(waylandSurfaceId))
        {
            embeddedCompositingManager.processClientRequests();
        }
        LOG_INFO(CONTEXT_RENDERER,
                 "EmbeddedCompositingTestsFramework::waitForNoBufferAttachedToIviSurface(): no buffer is attached");
    }

    void EmbeddedCompositingTestsFramework::renderOneFrame()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::renderOneFrame() rendering frame");
        getTestRenderer().doOneLoop();
    }

    void EmbeddedCompositingTestsFramework::waitForSurfaceAvailableForStreamTexture(StreamTextureSourceId sourceId)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForSurfaceAvailableForStreamTexture(): waiting for surface available for stream source id :" << sourceId.getValue());

        while (!embeddedCompositor.hasSurfaceForStreamTexture(sourceId))
        {
            getTestRenderer().doOneLoop();
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForSurfaceAvailableForStreamTexture(): surface available for source id :" << sourceId.getValue());
    }

    void EmbeddedCompositingTestsFramework::waitForSurfaceUnavailableForStreamTexture(StreamTextureSourceId sourceId)
    {
        const IEmbeddedCompositor& embeddedCompositor = getEmbeddedCompositor();

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForSurfaceUnavailableForStreamTexture(): waiting for surface unavailable for stream source id :" << sourceId.getValue());

        while (embeddedCompositor.hasSurfaceForStreamTexture(sourceId))
        {
            getTestRenderer().doOneLoop();
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositingTestsFramework::waitForSurfaceUnavailableForStreamTexture(): surface unavailable for source id :" << sourceId.getValue());
    }

    bool EmbeddedCompositingTestsFramework::waitForStreamSurfaceAvailabilityChange(StreamTextureSourceId streamSource, bool available)
    {
        return getTestRenderer().waitForStreamSurfaceAvailabilityChange(ramses::streamSource_t(streamSource.getValue()), available);
    }

    void EmbeddedCompositingTestsFramework::sendStopToTestApplication()
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_StopApplication);
    }

    TestApplicationSurfaceId EmbeddedCompositingTestsFramework::sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval)
    {
        return sendCreateSurfaceToTestApplication(width, height, swapInterval, true);
    }

    TestApplicationSurfaceId EmbeddedCompositingTestsFramework::sendCreateSharedMemorySurfaceToTestApplication(UInt32 width, UInt32 height)
    {
        return sendCreateSurfaceToTestApplication(width, height, 0, false);
    }

    TestApplicationSurfaceId EmbeddedCompositingTestsFramework::sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval, Bool useEGL)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId;
            UInt32 width;
            UInt32 height;
            UInt32 swapInterval;
            Bool useEGL;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId = m_nextSurfaceId;
        params.width = width;
        params.height = height;
        params.swapInterval = swapInterval;
        params.useEGL = useEGL;
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_CreateSurface, params);

        m_nextSurfaceId = TestApplicationSurfaceId(m_nextSurfaceId.getValue() + 1);

        return params.surfaceId;
    }

    TestApplicationShellSurfaceId EmbeddedCompositingTestsFramework::sendCreateShellSurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId;
            TestApplicationShellSurfaceId shellSurfaceId;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId = surfaceId;
        params.shellSurfaceId = m_nextShellSurfaceId;
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_CreateShellSurface, params);

        m_nextShellSurfaceId = TestApplicationShellSurfaceId(m_nextShellSurfaceId.getValue() + 1);

        return params.shellSurfaceId;
    }

    void EmbeddedCompositingTestsFramework::sendDestroyShellSurfaceToTestApplication(TestApplicationShellSurfaceId shellSurfaceId)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_DestroyShellSurface,
                                                             shellSurfaceId);
    }

    void EmbeddedCompositingTestsFramework::sendSetShellSurfaceTitleToTestApplication(TestApplicationShellSurfaceId shellSurfaceId, const String& title)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_SetShellSurfaceTitle, shellSurfaceId);
        m_testForkingController.sendStringToTestApplication(title);
    }

    void EmbeddedCompositingTestsFramework::sendSetShellSurfaceDummyValuesToTestApplication(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId;
            TestApplicationShellSurfaceId shellSurfaceId;
        } params;

        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId    = surfaceId;
        params.shellSurfaceId = shellSurfaceId;

        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_SetShellSurfaceDummyValues,
                                                             params);
    }

    void EmbeddedCompositingTestsFramework::sendDestroySurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_DestroySurface, surfaceId);
    }

    void EmbeddedCompositingTestsFramework::sendDestroyIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_DestroyIVISurface, surfaceId);
    }

    void EmbeddedCompositingTestsFramework::sendCreateIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId, WaylandIviSurfaceId surfaceIviId)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId;
            UInt32 surfaceIviId;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId = surfaceId;
        params.surfaceIviId = surfaceIviId.getValue();
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_CreateIVISurface, params);
    }

    void EmbeddedCompositingTestsFramework::sendRenderOneFrameToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId;
            bool useCallback;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId = surfaceId;
        params.useCallback = false;
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_RenderOneFrame, params);
    }

    void EmbeddedCompositingTestsFramework::sendAttachBufferToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_AttachBuffer, surfaceId);
    }

    void EmbeddedCompositingTestsFramework::sendDestroyBuffersToTestApplication()
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_DestroyBuffers);
    }

    void EmbeddedCompositingTestsFramework::sendRenderOneFrameAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId;
            bool useCallback;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId = surfaceId;
        params.useCallback = true;
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_RenderOneFrame, params);
    }

    void EmbeddedCompositingTestsFramework::sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId1;
            TestApplicationSurfaceId surfaceId2;
            bool useCallback;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId1 = surfaceId1;
        params.surfaceId2 = surfaceId2;
        params.useCallback = true;
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_RenderOneFrameToTwoSurfaces, params);
    }

    void EmbeddedCompositingTestsFramework::sendAdditionalConnectToEmbeddedCompositorToTestApplication()
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_AdditionalConnectToEmbeddedCompositor);
    }

    void EmbeddedCompositingTestsFramework::sendDetachBufferFromSurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_DetachBufferFromSurface, surfaceId);
    }

    IEmbeddedCompositor& EmbeddedCompositingTestsFramework::getEmbeddedCompositor()
    {
        const TestDisplays& displays = getDisplays();
        assert(displays.size() > 0);
        return getTestRenderer().getEmbeddedCompositor(displays[0].displayId);
    }

    IEmbeddedCompositingManager& EmbeddedCompositingTestsFramework::getEmbeddedCompositorManager()
    {
        const TestDisplays& displays = getDisplays();
        assert(displays.size() > 0);
        return getTestRenderer().getEmbeddedCompositorManager(displays[0].displayId);
    }

    void EmbeddedCompositingTestsFramework::setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility)
    {
        getTestRenderer().setSurfaceVisibility(surfaceId, visibility);
    }

    void EmbeddedCompositingTestsFramework::sendSetSurfaceSizeToTestApplicaton(TestApplicationSurfaceId surfaceId, UInt32 width, UInt32 height)
    {
        struct Params
        {
            TestApplicationSurfaceId surfaceId;
            UInt32 width;
            UInt32 height;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.surfaceId = surfaceId;
        params.width = width;
        params.height = height;
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_SetSurfaceSize, params);
    }

    void EmbeddedCompositingTestsFramework::sendSetTriangleColorToTestApplication(ETriangleColor color)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_SetTriangleColor, color);
    }

    void EmbeddedCompositingTestsFramework::killTestApplication()
    {
        m_testForkingController.killTestApplication();
    }

    UInt32 EmbeddedCompositingTestsFramework::getNumberOfAllocatedSHMBufferFromTestApplication()
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_GetNumberOfAllocatedSHMBuffer);
        UInt32 numberOfAllocatedSHMBuffer(0);
        m_testForkingController.getAnswerFromTestApplication(numberOfAllocatedSHMBuffer, getEmbeddedCompositorManager());
        return numberOfAllocatedSHMBuffer;
    }

    Bool EmbeddedCompositingTestsFramework::getIsBufferFreeFromTestApplication(UInt32 buffer)
    {
        struct Params
        {
            UInt32 buffer;
        } params;
        PlatformMemory::Set(&params, 0, sizeof(params));
        params.buffer = buffer;
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_GetIsBufferFree, params);
        bool isBufferFree(false);
        m_testForkingController.getAnswerFromTestApplication(isBufferFree, getEmbeddedCompositorManager());
        return isBufferFree;
    }

    void EmbeddedCompositingTestsFramework::getOutputValuesFromTestApplication(Int32& width, Int32& height, Int32& scale, UInt32& doneCount)
    {
        m_testForkingController.sendMessageToTestApplication(ETestWaylandApplicationMessage_GetOutputValues);
        m_testForkingController.getAnswerFromTestApplication(width, getEmbeddedCompositorManager());
        m_testForkingController.getAnswerFromTestApplication(height, getEmbeddedCompositorManager());
        m_testForkingController.getAnswerFromTestApplication(scale, getEmbeddedCompositorManager());
        m_testForkingController.getAnswerFromTestApplication(doneCount, getEmbeddedCompositorManager());
    }
}
