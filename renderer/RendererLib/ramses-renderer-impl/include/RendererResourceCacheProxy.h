//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERRESOURCECACHEPROXY_H
#define RAMSES_RENDERERRESOURCECACHEPROXY_H

#include "RendererAPI/IRendererResourceCache.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/ResourceContentHash.h"
#include "ramses-renderer-api/Types.h"

namespace ramses
{
    class IRendererResourceCache;
    class RendererResourceCacheProxy : public ramses_internal::IRendererResourceCache
    {
    public:
        explicit RendererResourceCacheProxy(ramses::IRendererResourceCache& cache);
        virtual ~RendererResourceCacheProxy() override;

        virtual bool hasResource(ramses_internal::ResourceContentHash resourceId, uint32_t& size) const override;
        virtual bool getResourceData(ramses_internal::ResourceContentHash resourceId, uint8_t* buffer, uint32_t bufferSize) const override;
        virtual bool shouldResourceBeCached(ramses_internal::ResourceContentHash resourceId, uint32_t resourceDataSize, ramses_internal::ResourceCacheFlag cacheFlag, ramses_internal::SceneId sceneId) const override;
        virtual void storeResource(ramses_internal::ResourceContentHash resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, ramses_internal::ResourceCacheFlag cacheFlag, ramses_internal::SceneId sceneId) override;

    private:

        static ramses::rendererResourceId_t ConvertInternalTypeToPublic(ramses_internal::ResourceContentHash input);
        static ramses::resourceCacheFlag_t ConvertInternalTypeToPublic(ramses_internal::ResourceCacheFlag input);
        static ramses::sceneId_t ConvertInternalTypeToPublic(ramses_internal::SceneId input);

        ramses::IRendererResourceCache& m_cache;
    };
}

#endif
