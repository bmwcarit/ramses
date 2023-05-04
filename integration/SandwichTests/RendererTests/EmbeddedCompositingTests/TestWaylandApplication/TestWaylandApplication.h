//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTWAYLANDAPPLICATION_H
#define RAMSES_TESTWAYLANDAPPLICATION_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "RendererAPI/Types.h"
#include "EmbeddedCompositingTestMessages.h"
#include "NamedPipe.h"
#include "WaylandHandler.h"
#include "OpenGLTriangleDrawer.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "Utils/LogMacros.h"
#include <vector>

namespace ramses_internal
{
    class TestWaylandApplication
    {
    public:
        TestWaylandApplication(const String& testToWaylandClientPipeName, const String& waylandClientToTestPipeName);
        ~TestWaylandApplication();
        bool run();

    private:
        bool readFromTestFramework(void* data, UInt32 numberOfBytes);
        template <typename T>
        bool readFromTestFramework(T& value);
        bool readBufferFromTestFramework();
        bool dispatchIncomingMessage();

        bool initializeWayland(const String& displayName, bool connectUsingFD);
        void stopWayland();
        void renderFrameToEGLBuffer(TestApplicationSurfaceId surfaceId, bool useCallback);
        void renderFrameToSharedMemoryBuffer(TestApplicationSurfaceId surfaceId, bool useCallback);
        void attachBuffer(TestApplicationSurfaceId surfaceId, bool commit);
        void renderFrameToTwoSurfaces(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2, bool useCallback);
        void setTriangleColor(ETriangleColor color);
        void detachBufferFromSurface(TestApplicationSurfaceId surfaceId);
        bool startRamsesRendererAndRunRenderingTest(WaylandIviLayerId waylandIviLayerId, uint32_t iviSurfaceIdOffset);
        template <typename T>
        void sendAnswerToTestFramework(const T& value);

        NamedPipe                               m_testToWaylandClientPipe;
        NamedPipe                               m_waylandClientToTestPipe;
        WaylandHandler                          m_waylandHandler;
        WaylandHandler                          m_waylandHandler2;
        ETriangleColor                          m_triangleColor;

        std::vector<Byte>                       m_readingBuffer;
        std::unique_ptr<ramses_internal::UnixDomainSocket> m_socket;
    };
}
#endif
