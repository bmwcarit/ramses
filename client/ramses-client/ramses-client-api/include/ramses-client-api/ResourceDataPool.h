//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEDATAPOOL_H
#define RAMSES_RESOURCEDATAPOOL_H

#include "ramses-client-api/EDataType.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/TextureSwizzle.h"

#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include <string>

namespace ramses
{
    class EffectDescription;
    class Resource;
    class Scene;
    class ResourceDataPoolImpl;

    /**
     * @brief The ResourceDataPool holds resource data which can be instantiated for a given scene. Resource data can either be
     *        added by calling the add functions or by attaching a resource data file to the pool. The same resource data can be
     *        instantiated by multiple scenes at the same time.
     *
     * @deprecated This class was being introduced to cover legacy ramses use cases. Using this class is discouraged
     *             and it might be removed without warning in the future.
    */
    class RAMSES_API ResourceDataPool
    {
    public:
        /**
         * @brief Add ArrayResource data to the pool. The pool is taking ownership of the given range of data of a certain type and keeps it
         *        to instantiate resource from it later via createResourceForScene. See #ramses::ArrayResource for more details.
         *        Readding the same resource data again will return the previous resource id, but not recreate the resource data.
         *
         * @param[in] type The data type of the array elements.
         * @param[in] numElements The number of elements of the given data type to use for the resource.
         * @param[in] arrayData Pointer to the data to be used to create the array from.
         * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
         * @param[in] name The optional name of the ArrayResource.
         * @return The resource id of the created pool ArrayResource
         */
        resourceId_t addArrayResourceData(
            EDataType type,
            uint32_t numElements,
            const void* arrayData,
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = nullptr);

        /**
        * @brief Add Texture2D data to the pool. It is taking ownership of the given range of texture data in the specified pixel format
        *        and keeps it to instantiate resource from it later via createResourceForScene. See #ramses::Texture2D for more details.
        *
        *        Readding the same resource data again will return the previous resource id, but not recreate the resource data.
        *
        * @param[in] format Pixel format of the Texture2D data.
        * @param[in] width Width of the texture (mipmap level 0).
        * @param[in] height Height of the texture (mipmap level 0).
        * @param[in] mipMapCount Number of mipmap levels contained in mipLevelData array.
        * @param[in] mipLevelData Array of MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order is lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] swizzle Describes how RGBA channels of the texture are swizzled,
        *          where each member of the struct represents one destination channel that the source channel should get sampled from.
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Texture2D.
        * @return The resource id of the pool Texture2D resource
        */
        resourceId_t addTexture2DData(
            ETextureFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t mipMapCount,
            const MipLevelData mipLevelData[],
            bool generateMipChain = false,
            const TextureSwizzle& swizzle = {},
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = nullptr);

        /**
        * @brief Add Texture3D data to the pool. It is taking ownership of the given range of texture data in the specified pixel format
        *        and keeps it to instantiate resource from it later via createResourceForScene. See #ramses::Texture3D for more details.
        *        Readding the same resource data again will return the previous resource id, but not recreate the resource data.
        *
        * @param[in] format Pixel format of the Texture3D data.
        * @param[in] width Width of the texture (mipmap level 0).
        * @param[in] height Height of the texture (mipmap level 0).
        * @param[in] depth Depth of the texture.
        * @param[in] mipMapCount Number of mipmap levels contained in mipLevelData array.
        * @param[in] mipLevelData Array of MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order is lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Texture3D.
        * @return The resource id of the pool Texture3D resource
        */
        resourceId_t addTexture3DData(
            ETextureFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipMapCount,
            const MipLevelData mipLevelData[],
            bool generateMipChain = false,
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = nullptr);


        /**
        * @brief Add Cube Texture data to the pool. It is taking ownership of the given range of texture data in the specified pixel format
        *        and keeps it to instantiate resource from it later via createResourceForScene. All texel values are initially initialized to 0.
        *        See #ramses::TextureCube for more details.
        *        Readding the same resource data again will return the previous resource id, but not recreate the resource data.
        *
        * @param[in] format Pixel format of the Cube Texture data.
        * @param[in] size edge length of one quadratic cube face, belonging to the texture.
        * @param[in] mipMapCount Number of mipmaps contained in mipLevelData array.
        * @param[in] mipLevelData Array of MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order ist lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] swizzle Describes how RGBA channels of the texture are swizzled,
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Cube Texture.
        * @return The resource id of the pool Cube Texture resource
        */
        resourceId_t addTextureCubeData(
            ETextureFormat format,
            uint32_t size,
            uint32_t mipMapCount,
            const CubeMipLevelData mipLevelData[],
            bool generateMipChain = false,
            const TextureSwizzle& swizzle = {},
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = nullptr);

        /**
        * @brief Add Effect data to the pool by parsing a GLSL shader described by an EffectDescription instance. The data can be used to
        *        instantiate a resource via createResourceForScene. Refer to RamsesClient::getLastEffectErrorMessages in case of parsing error.
        *        See #ramses::Effect for more details.
        *        Readding the same resource data again will return the previous resource id, but not recreate the resource data.
        *
        * @param[in] effectDesc Effect description.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the created Effect.
        * @return The resource id of the pool effect
        */
        resourceId_t addEffectData(
            const EffectDescription& effectDesc,
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = nullptr);

        /**
         * @brief Get the GLSL error messages that were produced at the creation of the last Effect data
         *
         * @return A string containing the GLSL error messages of the last effect
         */
        std::string getLastEffectErrorMessages() const;

        /**
        * @brief Removes data which was added to the pool. The provided resource id can then not be used anymore
        *        to instantiate a resource via createResourceForScene. Already instantiated resources will not be affected by this.
        *
        * @param[in] id The resource id of the previously added resource data.
        * @return true for success.
        */
        bool removeResourceData(resourceId_t const& id);

        /**
        * @brief Adds a file to the resource data pool, so the contained data can be instantiated via
        *        createResourceForScene.
        *
        * @param[in] filename The file name of the resource file to be added to pool.
        * @return true for success.
        */
        bool addResourceDataFile(std::string const& filename);

        /**
        * @brief Loads all resources in a file currently in use by any scene from that file to memory.
        *
        * @details When resources are created via createResourceForScene, not the full resource data is loaded immediately,
        *          but lazily at a later time when the data is actually needed. This function will fully load all resources
        *          which are currently instantiated in any scene and make sure resources are complete and independent of the
        *          resource file.
        *
        *          This operation can be used to trigger the potentially heavy resource loading at a user chosen, most
        *          convenient point in time to ensure fast resource handling once the resource is actually used for rendering.
        *
        *          This operation is recommended to be called before removeResourceDataFile to avoid removing a resource file
        *          whose resource data hasn't been fully loaded yet for all its resources in any scene.
        *
        * @param[in] filename The file name of the resource file resource are supposed to force loaded from.
        * @return true for success.
        */
        bool forceLoadResourcesFromResourceDataFile(std::string const& filename);

        /**
        * @brief Removes a resource file from the pool. The contained data can then not be used anymore
        *        to instantiate a resource via createResourceForScene.
        *
        * @details Calling this function might make resource data contained in the file unavailable for loading which
        *          leads to a scene not being rendered or rendered in a wrong way. It is recommended to call
        *          forceLoadResourcesFromResourceDataFile before to make sure all resource data is in memory and
        *          don't do any other resource operation before calling removeResourceDataFile.
        *
        * @param[in] filename The file name of the resource file to be removed from pool.
        * @return true for success.
        */
        bool removeResourceDataFile(std::string const& filename);

        /**
        * @brief Creates a resource for a scene out of pool data. The resource can then be used in
        *        scene as if created with the scenes create resource functions.
        *
        * @param[in] scene The scene to instantiate the resource in.
        * @param[in] id The resource id of the previously added resource.
        * @return Pointer to the created resource.
        */
        Resource* createResourceForScene(Scene& scene, resourceId_t const& id);

        /**
        * Stores internal data for implementation specifics of ResourceDataPool.
        */
        ResourceDataPoolImpl& impl;

        /**
         * \brief Deprecated overload of addArrayResourceData function with different order of parameters. Do not use.
         * \deprecated Use documented versions above. Will be removed in next major versions.
         */
        resourceId_t addArrayResourceData(uint32_t, EDataType, const void*, resourceCacheFlag_t = ResourceCacheFlag_DoNotCache, const char* = nullptr);

        /**
         * \brief Deprecated overload of addTexture2DData function with different order of parameters. Do not use.
         * \deprecated Use documented versions above. Will be removed in next major versions.
         */
        resourceId_t addTexture2DData(uint32_t, uint32_t, ETextureFormat, uint32_t, const MipLevelData mipLevelData[], bool = false, const TextureSwizzle& = {}, resourceCacheFlag_t = ResourceCacheFlag_DoNotCache, const char* = nullptr);

        /**
         * \brief Deprecated overload of addTexture3DData function with different order of parameters. Do not use.
         * \deprecated Use documented versions above. Will be removed in next major versions.
         */
        resourceId_t addTexture3DData(uint32_t, uint32_t, uint32_t, ETextureFormat, uint32_t, const MipLevelData mipLevelData[], bool = false, resourceCacheFlag_t = ResourceCacheFlag_DoNotCache, const char* = nullptr);

        /**
         * \brief Deprecated overload of addTextureCubeData function with different order of parameters. Do not use.
         * \deprecated Use documented versions above. Will be removed in next major versions.
         */
        resourceId_t addTextureCubeData(uint32_t, ETextureFormat, uint32_t, const CubeMipLevelData mipLevelData[], bool = false, const TextureSwizzle& = {}, resourceCacheFlag_t = ResourceCacheFlag_DoNotCache, const char* = nullptr);

    private:

        /**
        * @brief RamsesClientImpl is the factory for creating the ResourceDataPool instance.
        */
        friend class RamsesClientImpl;

        /**
        * @brief Constructor of the ResourceDataPool
        *
        * @param[in] pimpl Internal data for implementation specifics of ResourceDataPool
        */
        explicit ResourceDataPool(ResourceDataPoolImpl& pimpl);

        /**
        * @brief Destructor of the ResourceDataPool
        */
        ~ResourceDataPool();
    };
}

#endif
