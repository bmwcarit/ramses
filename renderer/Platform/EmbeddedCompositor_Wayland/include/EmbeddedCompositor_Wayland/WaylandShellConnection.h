//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDSHELLCONNECTION_H
#define RAMSES_WAYLANDSHELLCONNECTION_H

#include "wayland-server.h"
#include "ivi-application-server-protocol.h"
#include "EmbeddedCompositor_Wayland/IWaylandShellConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"

namespace ramses_internal
{
    class WaylandShellConnection : public IWaylandShellConnection
    {
    public:
        WaylandShellConnection(IWaylandClient& client, uint32_t version, uint32_t id);
        virtual ~WaylandShellConnection();
        bool wasSuccessfullyInitialized() const;

        virtual void resourceDestroyed() override;
        virtual void shellGetShellSurface(IWaylandClient& client, uint32_t id, IWaylandResource& surfaceResource);

    private:
        static void ResourceDestroyedCallback(wl_resource* shellConnectionResource);
        static void ShellGetShellSurfaceCallback(wl_client *client, wl_resource* shellConnectionResource, uint32_t id, wl_resource *surfaceResource);

        IWaylandResource* m_resource = nullptr;

        const struct Shell_Interface : private wl_shell_interface
        {
            Shell_Interface()
            {
                get_shell_surface = ShellGetShellSurfaceCallback;
            }
        } m_shellInterface;
    };
}

#endif
