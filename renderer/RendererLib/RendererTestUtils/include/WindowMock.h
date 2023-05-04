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

        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(bool, setExternallyOwnedWindowSize, (uint32_t, uint32_t ), (override));
        MOCK_METHOD(bool, setFullscreen, (bool), (override));
        MOCK_METHOD(bool, canRenderNewFrame, (), (const, override));
        MOCK_METHOD(void, handleEvents, (), (override));
        MOCK_METHOD(void, frameRendered, (), (override));

        MOCK_METHOD(UInt32, getWidth, (), (const, override));
        MOCK_METHOD(UInt32, getHeight, (), (const, override));
        MOCK_METHOD(Float, getAspectRatio, (), (const, override));
        MOCK_METHOD(Int32, getPosX, (), (const, override));
        MOCK_METHOD(Int32, getPosY, (), (const, override));

        MOCK_METHOD(void, setTitle, (const String&), (override));
        MOCK_METHOD(const String&, getTitle, (), (const, override));
        MOCK_METHOD(bool, hasTitle, (), (const, override));
        MOCK_METHOD(WaylandIviSurfaceId, getWaylandIviSurfaceID, (), (const, override));

        static const UInt32 FakeWidth = 1280u;
        static const UInt32 FakeHeight = 480u;

    private:
        void createDefaultMockCalls();
    };
}

#endif
