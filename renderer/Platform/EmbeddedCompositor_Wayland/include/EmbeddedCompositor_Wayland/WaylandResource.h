//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDRESOURCE_H
#define RAMSES_WAYLANDRESOURCE_H

#include "wayland-server.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"

namespace ramses_internal
{
    class WaylandResource : public IWaylandResource
    {
    public:
        WaylandResource();
        WaylandResource(wl_resource* resource, bool ownership);
        virtual ~WaylandResource();
        virtual int getVersion() override;
        virtual void postError(uint32_t code, const String& message) override;
        virtual void* getUserData() override;
        virtual void setImplementation(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroy) override;
        virtual void addDestroyListener(WaylandListener* listener) override;
        virtual WaylandNativeResource getWaylandNativeResource() override;
        virtual void disownWaylandResource() override;

    protected:
        wl_resource* m_resource;
        bool m_ownership;
    };
}

#endif
