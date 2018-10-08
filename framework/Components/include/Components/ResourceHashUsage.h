//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEHASHUSAGE_H
#define RAMSES_RESOURCEHASHUSAGE_H

#include "Resource/IResource.h"
#include "PlatformAbstraction/PlatformSharedPointer.h"
#include "ResourceHashUsageCallback.h"

namespace ramses_internal
{
    class ResourceHashUsage
    {
    public:
        ResourceHashUsage()
        {}
        ResourceHashUsage(const ResourceContentHash& hash, ResourceHashUsageCallback& deleter)
            : m_resource(&hash, deleter)
        {
        }

        bool isValid() const
        {
            return m_resource != nullptr;
        }

        const ResourceContentHash& getHash() const
        {
            return *m_resource;
        }

        bool operator==(const ResourceHashUsage& resourceHashUsage) const
        {
            return m_resource == resourceHashUsage.m_resource;
        }

        bool operator!=(const ResourceHashUsage& other) const
        {
            return !(*this == other);
        }

    private:
        PlatformSharedPointer<const ResourceContentHash> m_resource;
    };
}

#endif
