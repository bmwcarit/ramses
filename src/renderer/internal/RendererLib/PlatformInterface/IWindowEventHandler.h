//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Enums/EMouseEvent.h"
#include "internal/RendererLib/Enums/EKeyEvent.h"
#include "internal/RendererLib/Enums/EKeyCode.h"
#include "internal/RendererLib/Enums/EKeyModifier.h"

namespace ramses::internal
{
    class IWindow;

    class IWindowEventHandler
    {
    public:
        virtual ~IWindowEventHandler() = default;

        virtual void onKeyEvent(EKeyEvent event, KeyModifiers modifiers, ramses::EKeyCode keyCode) = 0;
        virtual void onMouseEvent(EMouseEvent event, int32_t posX, int32_t posY) = 0;
        virtual void onClose() = 0;
        virtual void onResize(uint32_t width, uint32_t height) = 0;
        virtual void onWindowMove(int32_t posX, int32_t posY) = 0;
    };
}
