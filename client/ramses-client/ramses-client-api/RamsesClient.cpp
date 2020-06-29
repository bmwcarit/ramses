//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// api
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Resource.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"

// private
#include "RamsesClientImpl.h"
#include "ramses-client-api/SceneConfig.h"
#include "SceneConfigImpl.h"
#include "RamsesClientTypesImpl.h"

namespace ramses
{
    RamsesClient::RamsesClient(RamsesClientImpl& impl_)
    : RamsesObject(impl_)
    , impl(impl_)
    {
        impl.setHLObject(this);
    }

    RamsesClient::~RamsesClient()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    Scene* RamsesClient::createScene(sceneId_t sceneId, const SceneConfig& sceneConfig /*= SceneConfig()*/, const char* name)
    {
        Scene* scene =  impl.createScene(sceneId, sceneConfig.impl, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), sceneId, name);
        return scene;
    }

    Effect* RamsesClient::createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, const char* name)
    {
        Effect* effect = impl.createEffect(effectDesc, cacheFlag, name);
        LOG_HL_CLIENT_API3(LOG_API_RESOURCE_PTR_STRING(effect), LOG_API_GENERIC_OBJECT_STRING(effectDesc), cacheFlag, name);
        return effect;
    }

    std::string RamsesClient::getLastEffectErrorMessages() const
    {
        return impl.getLastEffectErrorMessages();
    }

    const Vector3fArray* RamsesClient::createConstVector3fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        const Vector3fArray* arr = impl.createConstVector3fArray(count, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API4(LOG_API_RESOURCE_PTR_STRING(arr), count, LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return arr;
    }

    const UInt16Array* RamsesClient::createConstUInt16Array(uint32_t count, const uint16_t* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        const UInt16Array* arr = impl.createConstUInt16Array(count, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API4(LOG_API_RESOURCE_PTR_STRING(arr), count, LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return arr;
    }

    const UInt32Array* RamsesClient::createConstUInt32Array(uint32_t count, const uint32_t* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        const UInt32Array* arr = impl.createConstUInt32Array(count, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API4(LOG_API_RESOURCE_PTR_STRING(arr), count, LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return arr;
    }

    status_t RamsesClient::destroy(Scene& scene)
    {
        const status_t status = impl.destroy(scene);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(scene));
        return status;
    }

    status_t RamsesClient::destroy(const Resource& resource)
    {
        const status_t status = impl.destroy(resource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(resource));
        return status;
    }

    const FloatArray* RamsesClient::createConstFloatArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        const FloatArray* arr = impl.createConstFloatArray(count, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API4(LOG_API_RESOURCE_PTR_STRING(arr), count, LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return arr;
    }

    const Vector2fArray* RamsesClient::createConstVector2fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        const Vector2fArray* arr = impl.createConstVector2fArray(count, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API4(LOG_API_RESOURCE_PTR_STRING(arr), count, LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return arr;
    }

    const Vector4fArray* RamsesClient::createConstVector4fArray(uint32_t count, const float* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        const Vector4fArray* arr = impl.createConstVector4fArray(count, arrayData, cacheFlag, name);
        LOG_HL_CLIENT_API4(LOG_API_RESOURCE_PTR_STRING(arr), count, LOG_API_GENERIC_PTR_STRING(arrayData), cacheFlag, name);
        return arr;
    }

    Texture2D* RamsesClient::createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name /* = 0 */)
    {
        Texture2D* tex = impl.createTexture2D(width, height, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        LOG_HL_CLIENT_API9(LOG_API_RESOURCE_PTR_STRING(tex), width, height, format, mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, swizzle, cacheFlag, name);
        return tex;
    }

    Texture3D* RamsesClient::createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name /* = 0 */)
    {
        Texture3D* tex = impl.createTexture3D(width, height, depth, format, mipMapCount, mipLevelData, generateMipChain, cacheFlag, name);
        LOG_HL_CLIENT_API9(LOG_API_RESOURCE_PTR_STRING(tex), width, height, depth, format, mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, cacheFlag, name);
        return tex;
    }

    TextureCube* RamsesClient::createTextureCube(uint32_t size, ETextureFormat format, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name /* = 0 */)
    {
        TextureCube* tex = impl.createTextureCube(size, format, cacheFlag, name, mipMapCount, mipLevelData, generateMipChain, swizzle);
        LOG_HL_CLIENT_API8(LOG_API_RESOURCE_PTR_STRING(tex), size, format, mipMapCount, LOG_API_GENERIC_PTR_STRING(mipLevelData), generateMipChain, swizzle, cacheFlag, name);
        return tex;
    }

    status_t RamsesClient::saveSceneToFile(const Scene& scene, const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation, bool compress) const
    {
        auto status = impl.saveSceneToFile(scene.impl, fileName, resourceFileInformation, compress);
        LOG_HL_CLIENT_API4(status, LOG_API_GENERIC_OBJECT_STRING(scene), fileName, resourceFileInformation, compress);
        return status;
    }

    ramses::Scene* RamsesClient::loadSceneFromFile(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation)
    {
        auto scene = impl.loadSceneFromFile(fileName, resourceFileInformation);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), fileName, resourceFileInformation);
        return scene;
    }

    status_t RamsesClient::loadSceneFromFileAsync(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation)
    {
        auto status = impl.loadSceneFromFileAsync(fileName, resourceFileInformation);
        LOG_HL_CLIENT_API2(status, fileName, resourceFileInformation);
        return status;
    }

    status_t RamsesClient::markSceneIdForLoadingAsLocalOnly(sceneId_t sceneId)
    {
        auto status = impl.markSceneIdForLoadingAsLocalOnly(sceneId);
        LOG_HL_CLIENT_API1(status, sceneId);
        return status;
    }

    status_t RamsesClient::saveResources(const ResourceFileDescription& fileDescription, bool compress) const
    {
        auto status = impl.saveResources(fileDescription, compress);
        LOG_HL_CLIENT_API2(status, fileDescription, compress);
        return status;
    }

    status_t RamsesClient::saveResources(const ResourceFileDescriptionSet& fileDescriptions, bool compress) const
    {
        auto status = impl.saveResources(fileDescriptions, compress);
        LOG_HL_CLIENT_API2(status, fileDescriptions, compress);
        return status;
    }

    status_t RamsesClient::loadResources(const ResourceFileDescription& fileDescription) const
    {
        auto status = impl.loadResources(fileDescription);
        LOG_HL_CLIENT_API1(status, fileDescription);
        return status;
    }

    status_t RamsesClient::loadResources(const ResourceFileDescriptionSet& fileDescriptions) const
    {
        auto status = impl.loadResources(fileDescriptions);
        LOG_HL_CLIENT_API1(status, fileDescriptions);
        return status;
    }

    status_t RamsesClient::loadResourcesAsync(const ResourceFileDescription& fileDescription)
    {
        auto status = impl.loadResourcesAsync(fileDescription);
        LOG_HL_CLIENT_API1(status, fileDescription);
        return status;
    }

    status_t RamsesClient::loadResourcesAsync(const ResourceFileDescriptionSet& resourceFileInformation)
    {
        auto status = impl.loadResourcesAsync(resourceFileInformation);
        LOG_HL_CLIENT_API1(status, resourceFileInformation);
        return status;
    }

    status_t RamsesClient::forceCloseResourceFileAsync(const ResourceFileDescription& fileDescription) const
    {
        auto status = impl.closeResourceFile(fileDescription);
        LOG_HL_CLIENT_API1(status, fileDescription);
        return status;
    }

    status_t RamsesClient::forceCloseResourceFilesAsync(const ResourceFileDescriptionSet& fileDescriptions) const
    {
        auto status = impl.closeResourceFiles(fileDescriptions);
        LOG_HL_CLIENT_API1(status, fileDescriptions);
        return status;
    }

    const RamsesObject* RamsesClient::findObjectByName(const char* name) const
    {
        return impl.findObjectByName(name);
    }

    RamsesObject* RamsesClient::findObjectByName(const char* name)
    {
        return impl.findObjectByName(name);
    }

    const Resource* RamsesClient::findResourceById(resourceId_t id) const
    {
        return impl.getHLResource_Threadsafe(id);
    }

    Resource* RamsesClient::findResourceById(resourceId_t id)
    {
        return impl.getHLResource_Threadsafe(id);
    }

    status_t RamsesClient::dispatchEvents(IClientEventHandler& clientEventHandler)
    {
        auto status = impl.dispatchEvents(clientEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(clientEventHandler));
        return status;
    }
}
