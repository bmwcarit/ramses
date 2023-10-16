//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IWindow.h"
#include "gmock/gmock.h"

namespace ramses::internal
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

        MOCK_METHOD(uint32_t, getWidth, (), (const, override));
        MOCK_METHOD(uint32_t, getHeight, (), (const, override));
        MOCK_METHOD(float, getAspectRatio, (), (const, override));
        MOCK_METHOD(int32_t, getPosX, (), (const, override));
        MOCK_METHOD(int32_t, getPosY, (), (const, override));

        MOCK_METHOD(void, setTitle, (std::string_view), (override));
        MOCK_METHOD(const std::string&, getTitle, (), (const, override));
        MOCK_METHOD(bool, hasTitle, (), (const, override));
        MOCK_METHOD(WaylandIviSurfaceId, getWaylandIviSurfaceID, (), (const, override));

        static const uint32_t FakeWidth = 1280u;
        static const uint32_t FakeHeight = 480u;

    private:
        void createDefaultMockCalls();
    };
}
