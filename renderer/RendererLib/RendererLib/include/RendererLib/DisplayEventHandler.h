//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYEVENTHANDLER_H
#define RAMSES_DISPLAYEVENTHANDLER_H

#include "RendererAPI/Types.h"
#include "RendererAPI/IWindowEventHandler.h"

namespace ramses_internal
{

    class RendererEventCollector;

    class DisplayEventHandler : public IWindowEventHandler
    {
    public:
        DisplayEventHandler(DisplayHandle displayHandle, RendererEventCollector& eventCollector);
        ~DisplayEventHandler() override;

        /* Inherited from IWindowEventHandler */
        void onKeyEvent(EKeyEventType event, uint32_t modifiers, EKeyCode keyCode) override;
        void onMouseEvent(EMouseEventType event, int32_t posX, int32_t posY) override;
        void onClose() override;
        void onResize(uint32_t width, uint32_t height) override;
        void onWindowMove(int32_t posX, int32_t posY) override;

    private:
        const DisplayHandle     m_displayHandle;
        RendererEventCollector& m_eventCollector;
    };
}

#endif
