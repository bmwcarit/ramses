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
        virtual ~DisplayEventHandler();

        /* Inherited from IWindowEventHandler */
        virtual void onKeyEvent(EKeyEventType event, UInt32 modifiers, EKeyCode keyCode) override;
        virtual void onMouseEvent(EMouseEventType event, Int32 posX, Int32 posY) override;
        virtual void onClose() override;
        virtual void onResize(Int32 width, Int32 height) override;

    private:
        const DisplayHandle     m_displayHandle;
        RendererEventCollector& m_eventCollector;
    };
}

#endif
