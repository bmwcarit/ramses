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
#include "PlatformAbstraction/PlatformThread.h"
#include "EmbeddedCompositingTestMessages.h"
#include "TestSignalHandler.h"
#include "ETriangleColor.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "TestForkingController.h"

#include "Utils/BinaryOutputStream.h"

namespace ramses_internal
{
    EmbeddedCompositingTestsFramework::EmbeddedCompositingTestsFramework(bool generateScreenshots, TestForkingController& testForkingController, const ramses::RamsesFrameworkConfig& config)
        : RendererTestsFramework(generateScreenshots, config)
        , m_testForkingController(testForkingController)
    {
        TestSignalHandler::RegisterSignalHandlersForCurrentProcess("EmbeddedCompositingTestsFramework");
    }

    void EmbeddedCompositingTestsFramework::startTestApplication(bool initialize)
    {
        m_testForkingController.startTestApplication();

        if(initialize)
            initializeTestApplication();
    }

    void EmbeddedCompositingTestsFramework::startTestApplicationAndWaitUntilConnected()
    {
        m_testForkingController.startTestApplication();
        initializeTestApplication();
        waitUntilNumberOfCompositorConnections(1);
    }

    void EmbeddedCompositingTestsFramework::initializeTestApplication()
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::InitializeTestApplication;
        m_testForkingController.sendMessageToTestApplication(bos);
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

    void EmbeddedCompositingTestsFramework::logEmbeddedCompositor(RendererLogContext& logContext)
    {
        getEmbeddedCompositor().logInfos(logContext);
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
        return getTestRenderer().waitForStreamSurfaceAvailabilityChange(ramses::waylandIviSurfaceId_t(streamSource.getValue()), available);
    }

    void EmbeddedCompositingTestsFramework::sendStopToTestApplication()
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::StopApplication;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    TestApplicationSurfaceId EmbeddedCompositingTestsFramework::sendCreateSurfaceWithEGLContextToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval)
    {
        return sendCreateSurfaceToTestApplication(width, height, swapInterval, true);
    }

    TestApplicationSurfaceId EmbeddedCompositingTestsFramework::sendCreateSurfaceWithoutEGLContextToTestApplication(UInt32 width, UInt32 height)
    {
        return sendCreateSurfaceToTestApplication(width, height, 0, false);
    }

    TestApplicationSurfaceId EmbeddedCompositingTestsFramework::sendCreateSurfaceToTestApplication(UInt32 width, UInt32 height, UInt32 swapInterval, Bool useEGL)
    {
        const auto surfaceId = m_nextSurfaceId;

        BinaryOutputStream bos;
        IOutputStream& ios = bos;
        ios << ETestWaylandApplicationMessage::CreateSurface << surfaceId.getValue() << width << height << swapInterval << useEGL;
        m_testForkingController.sendMessageToTestApplication(bos);

        m_nextSurfaceId = TestApplicationSurfaceId(m_nextSurfaceId.getValue() + 1);

        return surfaceId;
    }

    TestApplicationShellSurfaceId EmbeddedCompositingTestsFramework::sendCreateShellSurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        const auto shellSurfaceId = m_nextShellSurfaceId;
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::CreateShellSurface << surfaceId.getValue() << shellSurfaceId.getValue();
        m_testForkingController.sendMessageToTestApplication(bos);

        m_nextShellSurfaceId = TestApplicationShellSurfaceId(m_nextShellSurfaceId.getValue() + 1);

        return shellSurfaceId;
    }

    void EmbeddedCompositingTestsFramework::sendDestroyShellSurfaceToTestApplication(TestApplicationShellSurfaceId shellSurfaceId)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::DestroyShellSurface << shellSurfaceId.getValue();
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendSetShellSurfaceTitleToTestApplication(TestApplicationShellSurfaceId shellSurfaceId, const String& title)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::SetShellSurfaceTitle << shellSurfaceId.getValue() << title;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendSetShellSurfaceDummyValuesToTestApplication(TestApplicationSurfaceId surfaceId, TestApplicationShellSurfaceId shellSurfaceId)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::SetShellSurfaceDummyValues << surfaceId.getValue() << shellSurfaceId.getValue();
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendDestroySurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::DestroySurface << surfaceId.getValue();
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendDestroyIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::DestroyIVISurface << surfaceId.getValue();
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendCreateIVISurfaceToTestApplication(TestApplicationSurfaceId surfaceId, WaylandIviSurfaceId surfaceIviId)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::CreateIVISurface << surfaceId.getValue() << surfaceIviId.getValue();
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendRenderOneFrameToEGLBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool waitOnFramecallback)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::RenderOneFrame_ToEGLBuffer << surfaceId.getValue() << waitOnFramecallback;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendRenderOneFrameToSharedMemoryBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool waitOnFramecallback)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::RenderOneFrame_ToSharedMemoryBuffer << surfaceId.getValue() << waitOnFramecallback;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendAttachBufferToTestApplication(TestApplicationSurfaceId surfaceId, bool commit)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::AttachBuffer << surfaceId.getValue() << commit;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendDestroyBuffersToTestApplication()
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::DestroyBuffers;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendRenderOneFrameToTwoSurfacesAndWaitOnFrameCallbackToTestApplication(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::RenderOneFrameToTwoSurfaces << surfaceId1.getValue() << surfaceId2.getValue() << true;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendAdditionalConnectToEmbeddedCompositorToTestApplication()
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::AdditionalConnectToEmbeddedCompositor;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendDetachBufferFromSurfaceToTestApplication(TestApplicationSurfaceId surfaceId)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::DetachBufferFromSurface << surfaceId.getValue();
        m_testForkingController.sendMessageToTestApplication(bos);
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
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::SetSurfaceSize << surfaceId.getValue() << width << height;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::sendSetTriangleColorToTestApplication(ETriangleColor color)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::SetTriangleColor << static_cast<uint32_t>(color);
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    bool EmbeddedCompositingTestsFramework::sendStartRamsesRendererAndRunRenderingTest()
    {
        const auto& displays = getDisplays();

        const ramses::waylandIviLayerId_t waylandIviLayerId = displays[0].config.getWaylandIviLayerID();
        const UInt32 iviSurfaceOffset = 10u;

        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::StartRamsesRendererAndRunRenderingTest << waylandIviLayerId.getValue() << iviSurfaceOffset;
        m_testForkingController.sendMessageToTestApplication(bos);

        bool testSucessResult = false;
        m_testForkingController.getAnswerFromTestApplication(testSucessResult, getTestRenderer().getEmbeddedCompositorManager(displays[0].displayId));

        return testSucessResult;
    }

    void EmbeddedCompositingTestsFramework::sendSetRequiredWaylandOutputVersion(uint32_t protocolVersion)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::SetRequiredWaylandOutputVersion << protocolVersion;
        m_testForkingController.sendMessageToTestApplication(bos);
    }

    void EmbeddedCompositingTestsFramework::killTestApplication()
    {
        m_testForkingController.killTestApplication();
    }

    void EmbeddedCompositingTestsFramework::setEnvironmentVariableWaylandSocket()
    {
        m_testForkingController.setEnvironmentVariableWaylandSocket();
    }

    void EmbeddedCompositingTestsFramework::setEnvironmentVariableWaylandDisplay()
    {
        m_testForkingController.setEnvironmentVariableWaylandDisplay();
    }

    UInt32 EmbeddedCompositingTestsFramework::getNumberOfAllocatedSHMBufferFromTestApplication()
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::GetNumberOfAllocatedSHMBuffer;
        m_testForkingController.sendMessageToTestApplication(bos);

        UInt32 numberOfAllocatedSHMBuffer(0);
        m_testForkingController.getAnswerFromTestApplication(numberOfAllocatedSHMBuffer, getEmbeddedCompositorManager());
        return numberOfAllocatedSHMBuffer;
    }

    Bool EmbeddedCompositingTestsFramework::getIsBufferFreeFromTestApplication(UInt32 buffer)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::GetIsBufferFree << buffer;
        m_testForkingController.sendMessageToTestApplication(bos);

        bool isBufferFree(false);
        m_testForkingController.getAnswerFromTestApplication(isBufferFree, getEmbeddedCompositorManager());
        return isBufferFree;
    }

    Bool EmbeddedCompositingTestsFramework::getWaylandOutputParamsFromTestApplication(WaylandOutputTestParams& resultWaylandOutputParams)
    {
        BinaryOutputStream bos;
        bos << ETestWaylandApplicationMessage::GetWaylandOutputParams;
        m_testForkingController.sendMessageToTestApplication(bos);

        bool errorsFound = false;
        m_testForkingController.getAnswerFromTestApplication(errorsFound, getEmbeddedCompositorManager());
        m_testForkingController.getAnswerFromTestApplication(resultWaylandOutputParams, getEmbeddedCompositorManager());

        return errorsFound;
    }
}
