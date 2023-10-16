//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandOutputParams.h"
#include <cstdint>

struct wl_client;

namespace ramses::internal
{
    class IWaylandGlobal;
    class IWaylandClient;
    class IWaylandDisplay;

    class WaylandOutputGlobal final
    {
    public:
        explicit WaylandOutputGlobal(const WaylandOutputParams& waylandOutputParams);
        ~WaylandOutputGlobal();
        bool init(IWaylandDisplay& serverDisplay);
        void destroy();
        void globalBind(IWaylandClient& client, uint32_t version, uint32_t id);

    private:
        static void GlobalBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id);

        const WaylandOutputParams m_waylandOutputParams;
        IWaylandGlobal* m_waylandGlobal = nullptr;
    };
}
