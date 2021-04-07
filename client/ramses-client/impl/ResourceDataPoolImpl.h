//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEDATAPOOLIMPL_H
#define RAMSES_RESOURCEDATAPOOLIMPL_H

#include "ramses-client-api/EDataType.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "ramses-client-api/RamsesObjectTypes.h"

#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include "Components/InputStreamContainer.h"
#include "Components/SceneFileHandle.h"
#include "Collections/HashMap.h"
#include "ResourceObjects.h"
#include "Components/ManagedResource.h"

#include <string>
#include <vector>

namespace ramses
{
    class EffectDescription;
    class Resource;
    class Scene;
    class RamsesClientImpl;

    class ResourceDataPoolImpl
    {
    public:
        explicit ResourceDataPoolImpl(RamsesClientImpl& client);

        resourceId_t addArrayResourceData(EDataType type, uint32_t size, const void* arrayData, resourceCacheFlag_t cacheFlag, const char* name);
        resourceId_t addTexture2DData(ETextureFormat format, uint32_t width, uint32_t height, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name);
        resourceId_t addTexture3DData(ETextureFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name);
        resourceId_t addTextureCubeData(ETextureFormat format, uint32_t size, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name);
        resourceId_t addEffectData(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, const char* name);

        std::string getLastEffectErrorMessages() const;

        bool removeResourceData(resourceId_t const& id);

        bool addResourceDataFile(std::string const& filename);
        bool forceLoadResourcesFromResourceDataFile(std::string const& filename);
        bool removeResourceDataFile(std::string const& filename);
        bool saveResourceDataFile(std::string const& filename, ResourceObjects const& resources, bool compress) const;

        Resource* createResourceForScene(Scene& scene, resourceId_t const& id);

        void getAllResourceDataFileResourceIds(std::vector<resourceId_t>& resources) const;

    private:
        struct ResourceData
        {
            ramses_internal::ManagedResource managedResource;
            std::string name;
            int refCount = 0;
        };

        struct ResourceFileAddress
        {
            ramses_internal::InputStreamContainerSPtr file;
            uint64_t offset;
            ramses_internal::SceneFileHandle fileHandle;
        };

        ramses::Resource* createResourceForScene(Scene& scene, ResourceFileAddress const& address, resourceId_t const& id) const;
        ramses::Resource* createResourceForScene(Scene& scene, ResourceData const& resourceData) const;

        resourceId_t addResourceDataToPool(ramses_internal::ManagedResource const& res, const char* name, ERamsesObjectType type);

        RamsesClientImpl& m_client;
        std::string m_effectErrorMessages;

        ramses_internal::HashMap<resourceId_t, std::vector<ResourceFileAddress>> m_resourceFileAddressRegister;
        ramses_internal::HashMap<std::string, std::vector<resourceId_t>> m_resourceDataFileContent;
        ramses_internal::HashMap<resourceId_t, ResourceData> m_resourcePoolData;
        ramses_internal::HashMap<std::string, ramses_internal::SceneFileHandle> m_filenameToHandle;
    };
}

#endif
