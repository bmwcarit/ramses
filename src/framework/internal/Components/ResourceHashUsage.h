//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/IResource.h"
#include "ResourceHashUsageCallback.h"
#include <memory>

namespace ramses::internal
{
    class ResourceHashUsage
    {
    public:
        ResourceHashUsage() = default;

        ResourceHashUsage(const ResourceContentHash& hash, ResourceHashUsageCallback& deleter)
            : m_resource(&hash, deleter)
        {
        }

        [[nodiscard]] bool isValid() const
        {
            return m_resource != nullptr;
        }

        [[nodiscard]] const ResourceContentHash& getHash() const
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
        std::shared_ptr<const ResourceContentHash> m_resource;
    };
}
