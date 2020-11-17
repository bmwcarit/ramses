//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/ResourceDataPool.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Resource.h"
#include "ResourceDataPoolImpl.h"
#include "APILoggingMacros.h"
#include "RamsesFrameworkTypesImpl.h"
#include "RamsesClientTypesImpl.h"

namespace ramses
{

    ResourceDataPool::ResourceDataPool(ResourceDataPoolImpl& pimpl)
        : impl(pimpl)
    {
    }

    ResourceDataPool::~ResourceDataPool()
    {
        delete& impl;
    }

    ramses::resourceId_t ResourceDataPool::addArrayResourceData(EDataType type, uint32_t numElements, const void* arrayData, resourceCacheFlag_t cacheFlag /*= ResourceCacheFlag_DoNotCache*/, const char* name /*= nullptr*/)
    {
        auto resourceId = impl.addArrayResourceData(type, numElements, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API5(type, resourceId, numElements, LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return resourceId;
    }

    ramses::resourceId_t ResourceDataPool::addTexture2DData(ETextureFormat format, uint32_t width, uint32_t height, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain /*= false*/, const TextureSwizzle& swizzle /*= {}*/, resourceCacheFlag_t cacheFlag /*= ResourceCacheFlag_DoNotCache*/, const char* name /*= nullptr*/)
    {
        auto resourceId = impl.addTexture2DData(format, width, height, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        LOG_HL_CLIENT_API9(getTextureFormatString(format), resourceId, width, height, mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, swizzle, cacheFlag, name);
        return resourceId;
    }

    ramses::resourceId_t ResourceDataPool::addTexture3DData(ETextureFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain /*= false*/, resourceCacheFlag_t cacheFlag /*= ResourceCacheFlag_DoNotCache*/, const char* name /*= nullptr*/)
    {
        auto resourceId = impl.addTexture3DData(format, width, height, depth, mipMapCount, mipLevelData, generateMipChain, cacheFlag, name);
        LOG_HL_CLIENT_API9(getTextureFormatString(format), resourceId, width, height, depth, mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, cacheFlag, name);
        return resourceId;
    }

    ramses::resourceId_t ResourceDataPool::addTextureCubeData(ETextureFormat format, uint32_t size, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain /*= false*/, const TextureSwizzle& swizzle /*= {}*/, resourceCacheFlag_t cacheFlag /*= ResourceCacheFlag_DoNotCache*/, const char* name /*= nullptr*/)
    {
        auto resourceId = impl.addTextureCubeData(format, size, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        LOG_HL_CLIENT_API8(getTextureFormatString(format), resourceId, size, mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, swizzle, cacheFlag, name);
        return resourceId;
    }

    ramses::resourceId_t ResourceDataPool::addEffectData(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag /*= ResourceCacheFlag_DoNotCache*/, const char* name /*= nullptr*/)
    {
        auto resourceId = impl.addEffectData(effectDesc, cacheFlag, name);
        LOG_HL_CLIENT_API3(resourceId, LOG_API_GENERIC_OBJECT_STRING(effectDesc), cacheFlag, name);
        return resourceId;
    }

    std::string ResourceDataPool::getLastEffectErrorMessages() const
    {
        return impl.getLastEffectErrorMessages();
    }

    bool ResourceDataPool::removeResourceData(resourceId_t const& id)
    {
        bool success = impl.removeResourceData(id);
        LOG_HL_CLIENT_API1(success, id);
        return success;
    }

    bool ResourceDataPool::addResourceDataFile(std::string const& filename)
    {
        bool success = impl.addResourceDataFile(filename);
        LOG_HL_CLIENT_API1(success, filename);
        return success;
    }

    bool ResourceDataPool::forceLoadResourcesFromResourceDataFile(std::string const& filename)
    {
        bool success = impl.forceLoadResourcesFromResourceDataFile(filename);
        LOG_HL_CLIENT_API1(success, filename);
        return success;
    }

    bool ResourceDataPool::removeResourceDataFile(std::string const& filename)
    {
        bool success = impl.removeResourceDataFile(filename);
        LOG_HL_CLIENT_API1(success, filename);
        return success;
    }

    ramses::Resource* ResourceDataPool::createResourceForScene(Scene& scene, resourceId_t const& id)
    {
        auto resource = impl.createResourceForScene(scene, id);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(resource), LOG_API_GENERIC_OBJECT_STRING(scene), id);
        return resource;
    }
}
