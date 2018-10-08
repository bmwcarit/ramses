//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWINDOWEVENTSPOLLINGMANAGER_H
#define RAMSES_IWINDOWEVENTSPOLLINGMANAGER_H

namespace ramses_internal
{
    class IWindow;

    class IWindowEventsPollingManager
    {
    public:
        virtual ~IWindowEventsPollingManager()
        {
        }

        virtual void pollWindowsTillAnyCanRender() const = 0;
    };
}

#endif
