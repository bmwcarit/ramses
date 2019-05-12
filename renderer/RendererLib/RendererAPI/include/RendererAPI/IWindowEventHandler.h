//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWINDOWEVENTHANDLER_H
#define RAMSES_IWINDOWEVENTHANDLER_H

#include "RendererLib/EMouseEventType.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EKeyCode.h"

namespace ramses_internal
{
    class IWindow;

    class IWindowEventHandler
    {
    public:
        virtual ~IWindowEventHandler() {};

        virtual void onKeyEvent(EKeyEventType event, UInt32 modifiers, EKeyCode keyCode) = 0;
        virtual void onMouseEvent(EMouseEventType event, Int32 posX, Int32 posY) = 0;
        virtual void onClose() = 0;
        virtual void onResize(Int32 width, Int32 height) = 0;
    };
}

#endif
