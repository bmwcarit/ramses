//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/NativeWaylandResource.h"

#include <cassert>
#include <string>

namespace ramses::internal
{
    NativeWaylandResource::NativeWaylandResource() = default;

    NativeWaylandResource::NativeWaylandResource(wl_resource* resource)
        : m_resource(resource)
    {
        assert(m_resource != nullptr);
    }

    int NativeWaylandResource::getVersion()
    {
        return wl_resource_get_version(m_resource);
    }

    void NativeWaylandResource::postError(uint32_t code, const std::string& message)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        wl_resource_post_error(m_resource, code, "%s", message.c_str());
    }

    void* NativeWaylandResource::getUserData()
    {
        return wl_resource_get_user_data(m_resource);
    }

    void NativeWaylandResource::setImplementation(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroyCallback)
    {
        wl_resource_set_implementation(m_resource, implementation, data, destroyCallback);
    }

    void NativeWaylandResource::addDestroyListener(wl_listener* listener)
    {
        wl_resource_add_destroy_listener(m_resource, listener);
    }

    wl_resource* NativeWaylandResource::getLowLevelHandle()
    {
        return m_resource;
    }

    void NativeWaylandResource::destroy()
    {
        assert(m_resource);
        wl_resource_destroy(m_resource);
    }
}
