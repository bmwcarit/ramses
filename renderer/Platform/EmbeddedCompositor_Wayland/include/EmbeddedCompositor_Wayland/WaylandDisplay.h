//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDDISPLAY_H
#define RAMSES_WAYLANDDISPLAY_H

#include "wayland-server.h"
#include "EmbeddedCompositor_Wayland/IWaylandDisplay.h"
#include "Collections/String.h"

namespace ramses_internal
{

    class WaylandDisplay: public IWaylandDisplay
    {
    public:
        WaylandDisplay();
        ~WaylandDisplay();
        virtual bool init(const String& socketName, const String& socketGroupName, int socketFD) override;
        virtual IWaylandGlobal* createGlobal(const wl_interface *interface, int version, void *data, wl_global_bind_func_t bind) override;
        virtual void dispatchEventLoop() override;
        virtual void flushClients() override;
        wl_display* get() const; // (AI) TODO - Eliminate, when TextureUploadingAdapter_Wayland does not need the wl_display anymore.

    private:
        bool addSocketToDisplay(const String& socketName, const String& socketGroupName, int socketFD);
        bool addSocketToDisplayWithFD(int socketFD);
        bool addSocketToDisplayWithName(const String& socketName, const String& socketGroupName);
        bool applyPermissionsGroupToEmbeddedCompositingSocket(const String& socketName, const String& socketGroupName);

        wl_display* m_display = nullptr;
    };
}

#endif
