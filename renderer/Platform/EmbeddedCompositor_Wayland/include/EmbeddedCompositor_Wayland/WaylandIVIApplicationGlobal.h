//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDIVIAPPLICATIONGLOBAL_H
#define RAMSES_WAYLANDIVIAPPLICATIONGLOBAL_H

#include "EmbeddedCompositor_Wayland/IWaylandIVIApplicationGlobal.h"
#include "wayland-server.h"

namespace ramses_internal
{
    class EmbeddedCompositor_Wayland;
    class WaylandDisplay;
    class IWaylandGlobal;

    class WaylandIVIApplicationGlobal: public IWaylandIVIApplicationGlobal
    {
    public:
        explicit WaylandIVIApplicationGlobal(EmbeddedCompositor_Wayland& compositor);
        ~WaylandIVIApplicationGlobal();

        bool init(WaylandDisplay& serverDisplay);
        void destroy();

    protected:
        void iviApplicationBind(IWaylandClient& client, uint32_t version, uint32_t id) override;

    private:
        static void IVIApplicationBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id);

        IWaylandGlobal* m_waylandGlobal = nullptr;
        EmbeddedCompositor_Wayland& m_compositor;
    };
}

#endif
