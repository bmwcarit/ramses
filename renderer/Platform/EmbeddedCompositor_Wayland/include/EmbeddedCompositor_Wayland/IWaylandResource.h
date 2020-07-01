//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_IWAYLANDRESOURCE_H
#define RAMSES_IWAYLANDRESOURCE_H

#include <cstdint>

typedef void(*IWaylandResourceDestroyFuncT) (struct wl_resource *resource);

struct wl_listener;

namespace ramses_internal
{
    class String;

    class IWaylandResource
    {
    public:
        virtual ~IWaylandResource() {}
        virtual int getVersion() = 0;
        virtual void postError(uint32_t code, const String& message) = 0;
        virtual void* getUserData() = 0;
        virtual void setImplementation(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroy) = 0;
        virtual void addDestroyListener(wl_listener* listener) = 0;
        virtual void* getWaylandNativeResource() = 0;
        virtual void disownWaylandResource() = 0;
    };
}

#endif
