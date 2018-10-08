//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOWMOCK_H
#define RAMSES_WINDOWMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IWindow.h"


namespace ramses_internal
{
    class WindowMock : public IWindow
    {
    public:
        WindowMock();
        ~WindowMock() override;

        MOCK_METHOD0(init, Bool()); // Does not exist in IWindow, needed for only for testing
        MOCK_METHOD1(setFullscreen, Bool(Bool));
        MOCK_CONST_METHOD0(canRenderNewFrame, Bool());
        MOCK_CONST_METHOD0(isOffscreen, Bool());
        MOCK_METHOD0(handleEvents, void());
        MOCK_METHOD0(frameRendered, void());

        MOCK_CONST_METHOD0(getWidth, UInt32());
        MOCK_CONST_METHOD0(getHeight, UInt32());
        MOCK_CONST_METHOD0(getAspectRatio, Float());
        MOCK_CONST_METHOD0(getPosX, Int32());
        MOCK_CONST_METHOD0(getPosY, Int32());

        MOCK_METHOD1(setTitle, void(const String&));
        MOCK_CONST_METHOD0(getTitle, const String&());
        MOCK_CONST_METHOD0(hasTitle, bool());
        MOCK_CONST_METHOD0(getWaylandIviSurfaceID, WaylandIviSurfaceId());

        static const UInt32 FakeWidth = 16u;
        static const UInt32 FakeHeight = 8u;

    private:
        void createDefaultMockCalls();
    };

    class WindowMockWithDestructor : public WindowMock
    {
    public:
        WindowMockWithDestructor();
        ~WindowMockWithDestructor() override;
        MOCK_METHOD0(Die, void());
    };
}

#endif
