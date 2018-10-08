//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDSHELLGLOBAL_H
#define RAMSES_WAYLANDSHELLGLOBAL_H

#include "wayland-server.h"
#include "EmbeddedCompositor_Wayland/IWaylandShellGlobal.h"

namespace ramses_internal
{
    class WaylandDisplay;
    class IWaylandGlobal;

    class WaylandShellGlobal: public IWaylandShellGlobal
    {
    public:
        WaylandShellGlobal();
        ~WaylandShellGlobal();

        bool init(WaylandDisplay& serverDisplay);
        void destroy();
        void shellBind(IWaylandClient& client, uint32_t version, uint32_t id) override;

    private:
        static void ShellBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id);

        IWaylandGlobal* m_waylandGlobal = nullptr;
    };
}

#endif
