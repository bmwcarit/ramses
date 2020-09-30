//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_INATIVEWAYLANDRESOURCE_H
#define RAMSES_INATIVEWAYLANDRESOURCE_H

#include <cstdint>

typedef void(*IWaylandResourceDestroyFuncT) (struct wl_resource *resource);

struct wl_listener;
struct wl_resource;

namespace ramses_internal
{
    class String;

    class INativeWaylandResource
    {
    public:
        virtual ~INativeWaylandResource() {}
        virtual int getVersion() = 0;
        virtual void postError(uint32_t code, const String& message) = 0;
        virtual void* getUserData() = 0;
        virtual void setImplementation(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroyCallback) = 0;
        virtual void addDestroyListener(wl_listener* listener) = 0;
        virtual wl_resource* getLowLevelHandle() = 0;
        virtual void destroy() = 0;
    };
}

#endif
