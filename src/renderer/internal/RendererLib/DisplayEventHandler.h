//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"

namespace ramses::internal
{

    class RendererEventCollector;

    class DisplayEventHandler : public IWindowEventHandler
    {
    public:
        DisplayEventHandler(DisplayHandle displayHandle, RendererEventCollector& eventCollector);
        ~DisplayEventHandler() override;

        /* Inherited from IWindowEventHandler */
        void onKeyEvent(EKeyEvent event, KeyModifiers modifiers, EKeyCode keyCode) override;
        void onMouseEvent(EMouseEvent event, int32_t posX, int32_t posY) override;
        void onClose() override;
        void onResize(uint32_t width, uint32_t height) override;
        void onWindowMove(int32_t posX, int32_t posY) override;

    private:
        const DisplayHandle     m_displayHandle;
        RendererEventCollector& m_eventCollector;
    };
}
