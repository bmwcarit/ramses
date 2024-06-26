//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    class WindowEventHandlerMock : public IWindowEventHandler
    {
    public:
        WindowEventHandlerMock();
        ~WindowEventHandlerMock() override;

        MOCK_METHOD(void, onResize, (uint32_t width, uint32_t height), (override));
        MOCK_METHOD(void, onKeyEvent, (EKeyEvent event, KeyModifiers modifiers, EKeyCode keyCode), (override));
        MOCK_METHOD(void, onMouseEvent, (EMouseEvent event, int32_t posX, int32_t posY), (override));
        MOCK_METHOD(void, onClose, (), (override));
        MOCK_METHOD(void, onWindowMove, (int32_t posX, int32_t posY), (override));
    };
}
