//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERRESOURCECACHEFAKE_H
#define RAMSES_RENDERERRESOURCECACHEFAKE_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IRendererResourceCache.h"
#include "SceneAPI/ResourceContentHash.h"
#include "RendererResourceCacheMock.h"
#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal {

class RendererResourceCacheFake : public RendererResourceCacheMock
{
public:

    virtual bool hasResource(ResourceContentHash resourceId, uint32_t& size) const override
    {
        RendererResourceCacheMock::hasResource(resourceId, size);

        size = static_cast<uint32_t>(m_data.size());
        return !m_data.empty();
    }

    virtual bool getResourceData(ResourceContentHash resourceId, uint8_t* buffer, uint32_t bufferSize) const override
    {
        RendererResourceCacheMock::getResourceData(resourceId, buffer, bufferSize);

        if (m_data.empty())
        {
            return false;
        }

        PlatformMemory::Copy(buffer, m_data.data(), m_data.size());
        return true;
    }

    virtual void storeResource(ResourceContentHash resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, ramses_internal::ResourceCacheFlag cacheFlag, ramses_internal::SceneId sceneId) override
    {
        RendererResourceCacheMock::storeResource(resourceId, resourceData, resourceDataSize, cacheFlag, sceneId);

        m_data.resize(resourceDataSize);
        PlatformMemory::Copy(m_data.data(), resourceData, resourceDataSize);
    }

private:

    std::vector<uint8_t> m_data;
};
}
#endif
