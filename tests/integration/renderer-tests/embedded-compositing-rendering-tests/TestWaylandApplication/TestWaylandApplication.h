//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "EmbeddedCompositingTestMessages.h"
#include "NamedPipe.h"
#include "WaylandHandler.h"
#include "OpenGLTriangleDrawer.h"
#include "internal/Platform/Wayland/UnixDomainSocket.h"
#include "internal/Core/Utils/LogMacros.h"

#include <cstdint>
#include <vector>
#include <string_view>
#include <string>

namespace ramses::internal
{
    class TestWaylandApplication
    {
    public:
        TestWaylandApplication(std::string_view testToWaylandClientPipeName, std::string_view waylandClientToTestPipeName);
        ~TestWaylandApplication();
        bool run();

    private:
        bool readFromTestFramework(void* data, uint32_t numberOfBytes);
        template <typename T>
        bool readFromTestFramework(T& value);
        bool readBufferFromTestFramework();
        bool dispatchIncomingMessage();

        bool initializeWayland(const std::string& displayName, bool connectUsingFD);
        void stopWayland();
        void renderFrameToEGLBuffer(TestApplicationSurfaceId surfaceId, bool useCallback);
        void renderFrameToSharedMemoryBuffer(TestApplicationSurfaceId surfaceId, bool useCallback);
        void attachBuffer(TestApplicationSurfaceId surfaceId, bool commit);
        void reattachBuffer(TestApplicationSurfaceId surfaceId, uint32_t count);
        void renderFrameToTwoSurfaces(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2, bool useCallback);
        void setTriangleColor(ETriangleColor color);
        void detachBufferFromSurface(TestApplicationSurfaceId surfaceId);
        static bool StartRamsesRendererAndRunRenderingTest(WaylandIviLayerId waylandIviLayerId, uint32_t iviSurfaceIdOffset);
        template <typename T>
        void sendAnswerToTestFramework(const T& value);

        NamedPipe                               m_testToWaylandClientPipe;
        NamedPipe                               m_waylandClientToTestPipe;
        WaylandHandler                          m_waylandHandler;
        WaylandHandler                          m_waylandHandler2;
        ETriangleColor                          m_triangleColor;

        std::vector<std::byte>                  m_readingBuffer;
        std::unique_ptr<UnixDomainSocket>       m_socket;
    };
}

