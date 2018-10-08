//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDOUTPUTCONNECTION_H
#define RAMSES_WAYLANDOUTPUTCONNECTION_H

#include "EmbeddedCompositor_Wayland/IWaylandOutputConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "wayland-server-protocol.h"

namespace ramses_internal
{
    class WaylandOutputResource;

    class WaylandOutputConnection : public IWaylandOutputConnection
    {
    public:
        WaylandOutputConnection(IWaylandClient& client, uint32_t version, uint32_t id, int32_t width, int32_t height, int32_t refresh);
        virtual ~WaylandOutputConnection();
        bool wasSuccessfullyInitialized() const;

        virtual void resourceDestroyed() override;

    private:
        static void ResourceDestroyedCallback(wl_resource* outputConnectionResource);
        static void OutputReleaseCallback(wl_client* client, wl_resource* outputConnectionResource);

        WaylandOutputResource* m_resource = nullptr;

        // Before wl_output_release was added, the struct wl_output_interface was not defined at all.
#ifdef WL_OUTPUT_RELEASE_SINCE_VERSION
        const struct Output_Interface : private wl_output_interface
        {
            Output_Interface()
            {
                release = OutputReleaseCallback;
            }
        } m_outputInterface;
#else
        const struct Output_Interface
        {
            Output_Interface()
            {
            }
        } m_outputInterface;
#endif
    };
}

#endif
