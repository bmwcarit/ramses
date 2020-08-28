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
#include "Utils/LogMacros.h"
#include <vector>

namespace ramses_internal
{
    class TestWaylandApplication
    {
    public:
        TestWaylandApplication(const String& testToWaylandClientPipeName, const String& waylandClientToTestPipeName);
        ~TestWaylandApplication();
        Bool run();

    private:
        bool readFromTestFramework(void* data, UInt32 numberOfBytes);
        template <typename T>
        bool readFromTestFramework(T& value);
        bool readBufferFromTestFramework();
        Bool dispatchIncomingMessage();

        bool initializeWayland();
        void stopWayland();
        void renderFrame(TestApplicationSurfaceId surfaceId, bool useCallback);
        void attachBuffer(TestApplicationSurfaceId surfaceId);
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
    };
}
#endif
