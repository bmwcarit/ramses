//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandResource.h"
#include "Collections/String.h"
#include "Utils/LogMacros.h"
#include <cassert>

namespace ramses_internal
{
    WaylandResource::WaylandResource(): m_resource(nullptr), m_ownership(false)
    {
    }

    WaylandResource::WaylandResource(wl_resource* resource, bool ownership)
        : m_resource(resource)
        , m_ownership(ownership)
    {
        assert(m_resource != nullptr);
    }

    WaylandResource::~WaylandResource()
    {
        if (m_ownership)
        {
            wl_resource_destroy(m_resource);
        }
    }

    int WaylandResource::getVersion()
    {
        return wl_resource_get_version(m_resource);
    }

    void WaylandResource::postError(uint32_t code, const String& message)
    {
        wl_resource_post_error(m_resource, code, "%s", message.c_str());
    }

    void* WaylandResource::getUserData()
    {
        return wl_resource_get_user_data(m_resource);
    }

    void WaylandResource::setImplementation(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroy)
    {
        wl_resource_set_implementation(m_resource, implementation, data, destroy);
    }

    void WaylandResource::addDestroyListener(WaylandListener* listener)
    {
        wl_resource_add_destroy_listener(m_resource, listener);
    }

    WaylandNativeResource WaylandResource::getWaylandNativeResource()
    {
        return m_resource;
    }

    void WaylandResource::disownWaylandResource()
    {
        m_ownership = false;
    }
}
