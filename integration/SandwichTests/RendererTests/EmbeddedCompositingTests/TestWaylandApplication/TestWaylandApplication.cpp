//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandHandler.h"
#include "TestWaylandApplication.h"
#include "TestSignalHandler.h"
#include "PlatformAbstraction/PlatformSignal.h"
#include "SHMTriangleDrawer.h"

namespace ramses_internal
{
    TestWaylandApplication::TestWaylandApplication(const String& testToWaylandClientPipeName, const String& waylandClientToTestPipeName)
        : m_testToWaylandClientPipe(testToWaylandClientPipeName, false)
        , m_waylandClientToTestPipe(waylandClientToTestPipeName, false)
        , m_triangleColor(ETriangleColor_Red)
    {
        setTriangleColor(ETriangleColor_Red);
        TestSignalHandler::RegisterSignalHandlersForCurrentProcess("TestWaylandApplication");

        m_testToWaylandClientPipe.open();
        m_waylandClientToTestPipe.open();
    }

    TestWaylandApplication::~TestWaylandApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication exiting");
    }

    Bool TestWaylandApplication::run()
    {
        if (!initializeWayland())
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::run(): failed initializing wayland");
            return false;
        }

        while(dispatchIncomingMessage())
        {
        }

        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication done handling incoming messages");

        stopWayland();
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication closing application");
        return true;
    }

    bool TestWaylandApplication::readFromTestFramework(void* data, UInt32 numberOfBytes)
    {
        const EReadFromPipeStatus readingStatus = m_testToWaylandClientPipe.read(data, numberOfBytes);
        switch (readingStatus)
        {
        case EReadFromPipeStatus_Failure:
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::readFromTestFramework(): can not read from pipe");
            return false;
        case EReadFromPipeStatus_Closed:
            LOG_WARN(CONTEXT_RENDERER, "TestWaylandApplication::readFromTestFramework(): pipe closed");
            return false;
        case EReadFromPipeStatus_Empty:
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::readFromTestFramework(): pipe empty");
            return false;
        default:
            return true;
        }
    }

    template <typename T>
    bool TestWaylandApplication::readFromTestFramework(T& value)
    {
        return readFromTestFramework(static_cast<void*>(&value), sizeof(value));
    }

    bool TestWaylandApplication::readStringFromTestFramework(String& string)
    {
        UInt32 stringLength;
        if (!readFromTestFramework(stringLength))
        {
            return false;
        }

        Vector<Char> characterVector(stringLength);
        if (!readFromTestFramework(characterVector.data(), sizeof(Char) * stringLength))
        {
            return false;
        }
        string = String(characterVector.data(), 0, stringLength - 1);
        return true;
    }

    Bool TestWaylandApplication::dispatchIncomingMessage()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): waiting for message...");

        uint32_t messageAsUInt;
        if (!readFromTestFramework(messageAsUInt))
        {
            return false;
        }

        switch(static_cast<ETestWaylandApplicationMessage>(messageAsUInt))
        {
        case ETestWaylandApplicationMessage_StopApplication:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message stop application");
            return false;
        }
        case ETestWaylandApplicationMessage_CreateSurface:
        {
            struct
            {
                TestApplicationSurfaceId surfaceId;
                UInt32 width;
                UInt32 height;
                UInt32 swapInterval;
                Bool useEGL;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message create surface size " << params.width << "/" << params.height << " swap interval " << params.swapInterval);

            bool bSuccess = m_waylandHandler.createWindow(params.surfaceId, params.width, params.height, params.swapInterval, params.useEGL);
            if (!bSuccess)
            {
                LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication: Could not setup Wayland");
            }

            return true;
        }
        case ETestWaylandApplicationMessage_CreateShellSurface:
        {
            struct
            {
                TestApplicationSurfaceId surfaceId;
                TestApplicationShellSurfaceId shellSurfaceId;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message create shell surface for surface with id " << params.surfaceId.getValue());
            m_waylandHandler.createShellSurface(params.surfaceId, params.shellSurfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage_DestroyShellSurface:
        {
            TestApplicationShellSurfaceId shellSurfaceId;
            if (!readFromTestFramework(shellSurfaceId))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message destroy shell surface with id "
                         << shellSurfaceId.getValue());
            m_waylandHandler.destroyShellSurface(shellSurfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage_SetShellSurfaceTitle:
        {
            TestApplicationShellSurfaceId shellSurfaceId;
            if (!readFromTestFramework(shellSurfaceId))
            {
                return false;
            }
            String title;
            if (!readStringFromTestFramework(title))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message set shell surface title for shell surface with id " << shellSurfaceId.getValue() << " title: " << title);
            m_waylandHandler.setShellSurfaceTitle(shellSurfaceId, title);
            return true;
        }
        case ETestWaylandApplicationMessage_SetShellSurfaceDummyValues:
        {
            struct
            {
                TestApplicationSurfaceId      surfaceId;
                TestApplicationShellSurfaceId shellSurfaceId;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message set shell surface dummy values "
                     "surface with id "
                         << params.surfaceId.getValue());
            m_waylandHandler.setShellSurfaceDummyValues(params.surfaceId, params.shellSurfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage_DestroySurface:
        {
            TestApplicationSurfaceId surfaceId;
            if (!readFromTestFramework(surfaceId))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message destroy surface with id " << surfaceId.getValue());
            m_waylandHandler.destroyWindow(surfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage_DestroyIVISurface:
        {
            TestApplicationSurfaceId surfaceId;
            if (!readFromTestFramework(surfaceId))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message destroy ivi surface for surface with id " << surfaceId.getValue());
            m_waylandHandler.destroyIVISurface(surfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage_CreateIVISurface:
        {
            struct
            {
                TestApplicationSurfaceId surfaceId;
                UInt32 surfaceIviId;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message create ivi surface for surface with id " << params.surfaceId.getValue() << " ivi-id: " << params.surfaceIviId);
            m_waylandHandler.createIVISurface(params.surfaceId, params.surfaceIviId);
            return true;
        }
        case ETestWaylandApplicationMessage_RenderOneFrame:
        {
            struct
            {
                TestApplicationSurfaceId surfaceId;
                bool useCallback;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message render one frame to surface with id " << params.surfaceId.getValue());
            renderFrame(params.surfaceId, params.useCallback);
            return true;
        }
        case ETestWaylandApplicationMessage_AttachBuffer:
        {
            TestApplicationSurfaceId surfaceId;
            if (!readFromTestFramework(surfaceId))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message attach buffer "
                     "with id "
                         << surfaceId.getValue());
            attachBuffer(surfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage_DestroyBuffers:
        {
            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message destroy buffers");
            m_waylandHandler.deleteSHMBuffers();
            return true;
        }
        case ETestWaylandApplicationMessage_SetSurfaceSize:
        {
            struct
            {
                TestApplicationSurfaceId surfaceId;
                UInt32 width;
                UInt32 height;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message change size for surface with id  " << params.surfaceId.getValue() << " to " << params.width << "*" << params.height);
            m_waylandHandler.resizeWindow(params.surfaceId, params.width, params.height);
            return true;
        }
        case ETestWaylandApplicationMessage_SetTriangleColor:
        {
            ETriangleColor triangleColor;
            if (!readFromTestFramework(triangleColor))
            {
                return false;
            }
            setTriangleColor(triangleColor);

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message set triangle color " << triangleColor);
            return true;
        }
        case ETestWaylandApplicationMessage_AdditionalConnectToEmbeddedCompositor:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message additional connect to embedded compositor");
            m_waylandHandler2.initWithSharedDisplayConnection(m_waylandHandler);
            return true;
        }

        case ETestWaylandApplicationMessage_DetachBufferFromSurface:
        {
            TestApplicationSurfaceId surfaceId;
            if (!readFromTestFramework(surfaceId))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message detach buffer from surface with id " << surfaceId.getValue());
            detachBufferFromSurface(surfaceId);
            return true;
        }

        case ETestWaylandApplicationMessage_GetNumberOfAllocatedSHMBuffer:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message get number of allocated shm buffer");
            const UInt32 numberOfAllocatedSHMBuffer = m_waylandHandler.getNumberOfAllocatedSHMBuffer();
            sendAnswerToTestFramework(numberOfAllocatedSHMBuffer);
            return true;
        }
        case ETestWaylandApplicationMessage_RenderOneFrameToTwoSurfaces:
        {
            struct
            {
                TestApplicationSurfaceId surfaceId1;
                TestApplicationSurfaceId surfaceId2;
                bool useCallback;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message render one frame to two surfaces with ivi id's " << params.surfaceId1.getValue() << ", " << params.surfaceId2.getValue());
            renderFrameToTwoSurfaces(params.surfaceId1, params.surfaceId2, params.useCallback);
            return true;
        }
        case ETestWaylandApplicationMessage_GetIsBufferFree:
        {
            struct
            {
                UInt32 buffer;
            } params;
            if (!readFromTestFramework(params))
            {
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message get is buffer free");

            const bool isBufferFree = m_waylandHandler.getIsSHMBufferFree(params.buffer);
            sendAnswerToTestFramework(isBufferFree);
            if (params.buffer >= m_waylandHandler.getNumberOfAllocatedSHMBuffer())
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandHandler::getIsSHMBufferFree buffer: " << params.buffer << " does not exist !");
                return false;
            }

            return true;
        }
        case ETestWaylandApplicationMessage_GetOutputValues:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message get output values");
            sendAnswerToTestFramework(m_waylandHandler.getOutputWidth());
            sendAnswerToTestFramework(m_waylandHandler.getOutputHeight());
            sendAnswerToTestFramework(m_waylandHandler.getOutputScale());
            sendAnswerToTestFramework(m_waylandHandler.getOutputDoneCount());
            return true;
        }
        }

        return false;
    }

    bool TestWaylandApplication::initializeWayland()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication started");
        bool bSuccess = m_waylandHandler.init();
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication m_waylandHandler initialized");
        if (!bSuccess)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication: Could not setup Wayland");
        }
        return bSuccess;
    }

    void TestWaylandApplication::stopWayland()
    {
        m_waylandHandler2.deinit();
        m_waylandHandler.deinit();
    }

    void TestWaylandApplication::renderFrame(TestApplicationSurfaceId surfaceId, bool useCallback)
    {
        if (m_waylandHandler.getUseEGL(surfaceId))
        {
            m_waylandHandler.enableContextForSurface(surfaceId);
            OpenGLTriangleDrawer triangleDrawer(m_triangleColor);
            triangleDrawer.draw();
            m_waylandHandler.swapBuffersAndProcessEvents(surfaceId, useCallback);
            m_waylandHandler.disableContextForSurface();
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrame buffers swapped");
        }
        else
        {
            uint32_t width;
            uint32_t height;
            m_waylandHandler.getWindowSize(surfaceId, width, height);
            SHMBuffer* buffer = m_waylandHandler.getFreeSHMBuffer(width, height);
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrame render to SHMBuffer with id  " << buffer->getId());
            SHMTriangleDrawer triangleDrawer(m_triangleColor);

            triangleDrawer.draw(buffer);

            m_waylandHandler.swapBuffersAndProcessEvents(surfaceId, *buffer, useCallback);
            if (useCallback)
            {
                m_waylandHandler.waitOnFrameCallback(surfaceId);
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrame shm buffers swapped");
        }
    }

    void TestWaylandApplication::attachBuffer(TestApplicationSurfaceId surfaceId)
    {
        uint32_t width;
        uint32_t height;
        m_waylandHandler.getWindowSize(surfaceId, width, height);
        SHMBuffer* buffer = m_waylandHandler.getFreeSHMBuffer(width, height);
        LOG_INFO(CONTEXT_RENDERER,
                 "TestWaylandApplication::renderFrame render to SHMBuffer with id  " << buffer->getId());
        SHMTriangleDrawer triangleDrawer(m_triangleColor);

        triangleDrawer.draw(buffer);

        m_waylandHandler.attachBuffer(surfaceId, *buffer);
    }

    void TestWaylandApplication::renderFrameToTwoSurfaces(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2, bool useCallback)
    {
        if (m_waylandHandler.getUseEGL(surfaceId1) || m_waylandHandler.getUseEGL(surfaceId2))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::renderFrameToTwoSurfaces Rendering to two surfaces with EGL not supported!");
        }
        else
        {
            uint32_t width1;
            uint32_t height1;
            m_waylandHandler.getWindowSize(surfaceId1, width1, height1);

            uint32_t width2;
            uint32_t height2;
            m_waylandHandler.getWindowSize(surfaceId2, width2, height2);

            if (width1 != width2 || height1 != height2)
            {
                LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::renderFrameToTwoSurfaces Surfaces must have same size!");
                assert(false);
                return;
            }

            SHMBuffer* buffer = m_waylandHandler.getFreeSHMBuffer(width1, height2);
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrame render to SHMBuffer with id  " << buffer->getId());
            SHMTriangleDrawer triangleDrawer(m_triangleColor);
            triangleDrawer.draw(buffer);

            m_waylandHandler.swapBuffersAndProcessEvents(surfaceId1, *buffer, useCallback);
            m_waylandHandler.swapBuffersAndProcessEvents(surfaceId2, *buffer, useCallback);
            if (useCallback)
            {
                m_waylandHandler.waitOnFrameCallback(surfaceId1);
                m_waylandHandler.waitOnFrameCallback(surfaceId2);
            }
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrame shm buffers swapped");
        }
    }

    void TestWaylandApplication::setTriangleColor(ETriangleColor color)
    {
        m_triangleColor = color;
    }

    void TestWaylandApplication::detachBufferFromSurface(TestApplicationSurfaceId surfaceId)
    {
        m_waylandHandler.detachBuffer(surfaceId);
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::detachBufferFromSurface buffer detached");
    }

    template <typename T>
    void TestWaylandApplication::sendAnswerToTestFramework(const T& value)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::sendAnswerToTestFramework message");
        if (!m_waylandClientToTestPipe.write(&value, sizeof(value)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::sendAnswerToTestFramework failed to write data to pipe!");
        }
    }
}
