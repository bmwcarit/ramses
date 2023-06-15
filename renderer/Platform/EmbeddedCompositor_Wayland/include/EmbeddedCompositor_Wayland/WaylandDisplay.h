//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDDISPLAY_H
#define RAMSES_WAYLANDDISPLAY_H

#include "EmbeddedCompositor_Wayland/IWaylandDisplay.h"
#include "wayland-server.h"

#include <string>

namespace ramses_internal
{

    class WaylandDisplay: public IWaylandDisplay
    {
    public:
        WaylandDisplay();
        ~WaylandDisplay() override;
        bool init(const std::string& socketName, const std::string& socketGroupName, uint32_t socketPermissions, int socketFD) override;
        IWaylandGlobal* createGlobal(const wl_interface *interface, int version, void *data, wl_global_bind_func_t bind) override;
        void dispatchEventLoop() override;
        void flushClients() override;
        [[nodiscard]] wl_display* get() const; // (AI) TODO - Eliminate, when TextureUploadingAdapter_Wayland does not need the wl_display anymore.

    private:
        bool addSocketToDisplay(const std::string& socketName, const std::string& socketGroupName, uint32_t socketPermissions, int socketFD);
        bool addSocketToDisplayWithFD(int socketFD);
        bool addSocketToDisplayWithName(const std::string& socketName, const std::string& socketGroupName, uint32_t socketPermissions);
        bool applyGroupToEmbeddedCompositingSocket(const std::string& socketFullPath, const std::string& socketGroupName);
        bool applyPermissionsToEmbeddedCompositingSocket(const std::string& socketFullPath, uint32_t socketPermissions);
        [[nodiscard]] std::string getSocketFullPath(const std::string& socketName) const;

        wl_display* m_display = nullptr;
    };
}

#endif
