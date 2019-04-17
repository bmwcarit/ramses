//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYEVENTHANDLERMANAGER_H
#define RAMSES_DISPLAYEVENTHANDLERMANAGER_H

#include "RendererAPI/Types.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    class DisplayEventHandler;
    class RendererEventCollector;
    class RendererScenes;
    class Renderer;

    class DisplayEventHandlerManager
    {
    public:
        DisplayEventHandlerManager(RendererEventCollector& eventCollector);
        virtual ~DisplayEventHandlerManager();

        DisplayEventHandler& createHandler(DisplayHandle display);
        void                 destroyHandler(DisplayHandle display);

        DisplayEventHandler& getHandler(DisplayHandle display);

    private:
        RendererEventCollector&                      m_eventCollector;
        HashMap<DisplayHandle, DisplayEventHandler*> m_displayHandlers;
    };
}

#endif
