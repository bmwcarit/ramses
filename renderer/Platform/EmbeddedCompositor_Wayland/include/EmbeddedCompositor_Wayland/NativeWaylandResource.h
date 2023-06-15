//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_NATIVEWAYLANDRESOURCE_H
#define RAMSES_NATIVEWAYLANDRESOURCE_H

#include "EmbeddedCompositor_Wayland/INativeWaylandResource.h"
#include "wayland-server.h"

namespace ramses_internal
{
    /**
      A word about wayland resource (wl_resource) and their lifecycle.

      A wayland resource is needed to specify the behavior (aka implementation) according to the
      different interfaces, e.g., what should a surface do when a buffer is attached to it?

      Creation of wl_resource objects is always straight forward and explicit. The resource must be created
      to implement a specific interface and a specify the callbacks for every function in that interface.
      It is also mandatory for EVERY wl_resource to specify a destroy callback (more on it downstairs)
      REGARDLESS of the interface being implemented. More specifically, a destroy callback has to be set
      regardless whether the implemented interface has a destroy function in it or no.

      So suggested from this, there can be different ways a wl_resource needs to be destroyed. In particular
      there are 3 (actually 2.5) ways to destroy a wl_resource:
      1. The wayland lib destroys the resource, e.g., because the connection with the client was closed. In this
         case the destroy callback specified during creation is called. Important to note here is that by the time
         the destroy callback is called the wl_resource is already in process of being destroyed (can still call
         functions on it like get_user_data(), but the ownership of its lifecycle has already been given away
         to wayland lib), so no need to call wl_resource_destroy on it.
      2. The implementation of an interfaces requests the EC to destroy a specific object, e.g., the wayland
         client requests the destruction of a buffer or a surface created before.
         In this case the wl_resource object has to be destroyed explicitly by calling
         wl_resource_destroy on it. It is important to note here that this will result in the destroy callback
         set during creation will be called. This might lead to a logic error if care is not taken to remove
         the destroy callback before explicitly destroying wl_resource object (as destroy callback might try to
         cleanup the same object that has ownership of the wl_resource, i.e., the object whose destructor is
         calling wl_resource_destroy)
      3. EC decides it wants to get rid of the wl_resource, e.g., because EC destructor is called and it needs
         to do some cleanup. This case is very similar to case 2
     */

    //This class is a simple wrapper around wl_resuorce
    class NativeWaylandResource : public INativeWaylandResource
    {
    public:
        NativeWaylandResource();
        explicit NativeWaylandResource(wl_resource* resource);

        int getVersion() override;
        void postError(uint32_t code, const std::string& message) override;
        void* getUserData() override;
        void setImplementation(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroyCallback) override;
        void addDestroyListener(wl_listener* listener) override;
        wl_resource* getLowLevelHandle() override;
        void destroy() override;

    protected:
        wl_resource* m_resource;
    };
}

#endif
