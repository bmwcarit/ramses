//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOWEVENTHANDLERMOCK_H
#define RAMSES_WINDOWEVENTHANDLERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IWindowEventHandler.h"

#include "RendererAPI/IWindow.h"

namespace ramses_internal
{
    class WindowEventHandlerMock : public IWindowEventHandler
    {
    public:
        WindowEventHandlerMock();
        ~WindowEventHandlerMock() override;

        MOCK_METHOD1(onResize, void(Int32 width, Int32 height));
        MOCK_METHOD1(onMove, void(const IWindow& surface));
        MOCK_METHOD1(onFocusChange, void(Bool bFocused));
        MOCK_METHOD3(onKeyEvent, void(EKeyEventType event, UInt32 modifiers, EKeyCode keyCode));
        MOCK_METHOD3(onMouseEvent, void(EMouseEventType event, Int32 posX, Int32 posY));
        MOCK_METHOD0(onClose, void());
    };
}

#endif
