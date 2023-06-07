//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWAYLANDDISPLAY_H
#define RAMSES_IWAYLANDDISPLAY_H

#include "wayland-server.h"

#include <string>

namespace ramses_internal
{
    class IWaylandGlobal;

    class IWaylandDisplay
    {
    public:
        virtual ~IWaylandDisplay() {}
        virtual bool init(const std::string& socketName, const std::string& socketGroupName, uint32_t socketPermissions, int socketFD) = 0;
        virtual IWaylandGlobal* createGlobal(const wl_interface* interface, int version, void* data, wl_global_bind_func_t bind) = 0;
        virtual void dispatchEventLoop() = 0;
        virtual void flushClients()      = 0;
    };
}

#endif
