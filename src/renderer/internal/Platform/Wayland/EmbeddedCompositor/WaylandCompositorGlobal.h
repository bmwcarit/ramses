//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandCompositorGlobal.h"

struct wl_client;
struct wl_global;

namespace ramses::internal
{
    class IEmbeddedCompositor_Wayland;
    class IWaylandGlobal;

    class WaylandCompositorGlobal: public IWaylandCompositorGlobal
    {
    public:
        explicit WaylandCompositorGlobal(IEmbeddedCompositor_Wayland& compositor);
        ~WaylandCompositorGlobal() override;
        bool init(IWaylandDisplay& serverDisplay) override;
        void destroy() override;
        void compositorBind(IWaylandClient& client, uint32_t version, uint32_t id) override;

    private:
        static void CompositorBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id);

        IEmbeddedCompositor_Wayland& m_compositor;
        IWaylandGlobal* m_waylandGlobal = nullptr;
    };
}
