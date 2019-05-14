//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MANAGEDRESOURCE_H
#define RAMSES_MANAGEDRESOURCE_H

#include "Resource/IResource.h"
#include "PlatformAbstraction/PlatformSharedPointer.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    class ManagedResource
    {
    public:
        ManagedResource()
        {}
        ManagedResource(const IResource& resource, ResourceDeleterCallingCallback& deleter)
            : m_resource(&resource, deleter)
        {
        }

        const IResource* getResourceObject() const
        {
            return m_resource.get();
        }

        bool operator==(const ManagedResource& managedResource) const
        {
            return m_resource == managedResource.m_resource;
        }

        bool operator!=(const ManagedResource& other) const
        {
            return !(*this == other);
        }

    private:
        PlatformSharedPointer<const IResource> m_resource;
    };

    typedef std::vector<ManagedResource> ManagedResourceVector;
}

#endif
