//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESCLIENT_H
#define RAMSES_RAMSESCLIENT_H

#include "ramses-client-api/RamsesObject.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/SceneConfig.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-framework-api/RamsesFramework.h"

/**
* ramses namespace
*
* @brief The RAMSES namespace contains all client side objects and
* functions used to implement RAMSES applications. RAMSES refers to these
* applications as clients.
*/
namespace ramses
{
    class Scene;
    class EffectDescription;
    class ResourceFileDescription;
    class ResourceFileDescriptionSet;
    class IClientEventHandler;
    class Resource;
    class FloatArray;
    class Vector2fArray;
    class Vector3fArray;
    class Vector4fArray;
    class UInt16Array;
    class UInt32Array;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class Effect;

    /**
    * @brief Entry point of RAMSES client API.
    *
    * The RAMSES client class handles application state and and is a factory for scenes and client resources.
    */
    class RAMSES_API RamsesClient : public RamsesObject
    {
    public:
        /**
        * @brief Creates an instance of the RAMSES client API
        *
        * @param name Name of RAMSES application
        * @param framework The ramses framework object to be used with this client
        */
        explicit RamsesClient(const char* name, RamsesFramework& framework);

        /**
        * @brief Destructor of RamsesClient
        */
        virtual ~RamsesClient();

        /**
        * @brief Create a new empty Scene.
        *
        * @param[in] sceneId The scene id for global identification of the Scene (is used for scene mapping on renderer side).
        * @param[in] sceneConfig The optional scene configuration that can be used to change parameters of scene to be created.
        * @param[in] name The optional name of the created Scene.
        * @return A pointer to the created Scene, null on failure
        */
        Scene* createScene(sceneId_t sceneId, const SceneConfig& sceneConfig = SceneConfig(), const char* name = 0);

        /**
        * @brief Create a new FloatArray
        *
        * @param[in] numberOfFloats The number of float values in the FloatArray
        * @param[in] arrayData Pointer to the float data to be used to create the array from.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The optional name of the FloatArray.
        * @return A pointer to the created FloatArray, null on failure
        */
        const FloatArray* createConstFloatArray(uint32_t numberOfFloats, const float* arrayData, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = 0);

        /**
        * @brief Create a new Vector2fArray
        *
        * @param[in] numberOfVectors The number of vectors in the Vector2fArray.
        * @param[in] arrayData Pointer to the float data to be used to create the array from.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The optional name of the Vector2fArray.
        * @return A pointer to the created Vector2fArray, null on failure
        */
        const Vector2fArray* createConstVector2fArray(uint32_t numberOfVectors, const float* arrayData, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = 0);

        /**
        * @brief Create a new Vector3fArray
        *
        * @param[in] numberOfVectors The number of vectors in the Vector3fArray.
        * @param[in] arrayData Pointer to the float data to be used to create the array from.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The optional name of the Vector3fArray.
        * @return A pointer to the created Vector3fArray, null on failure
        */
        const Vector3fArray* createConstVector3fArray(uint32_t numberOfVectors, const float* arrayData, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = 0);

        /**
        * @brief Create a new Vector4fArray
        *
        * @param[in] numberOfVectors The number of vectors in the Vector4fArray.
        * @param[in] arrayData Pointer to the float data to be used to create the array from.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The optional name of the Vector4fArray.
        * @return A pointer to the created Vector4fArray, null on failure
        */
        const Vector4fArray* createConstVector4fArray(uint32_t numberOfVectors, const float* arrayData, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = 0);

        /**
        * @brief Create a new UInt16Array
        *
        * @param[in] numberOfIndices The number of indices in the UInt16Array
        * @param[in] arrayData Pointer to the uint16_t data to be used to create the array from.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The optional name of the UInt16Array
        * @return A pointer to the created UInt16Array, null on failure
        */
        const UInt16Array* createConstUInt16Array(uint32_t numberOfIndices, const uint16_t* arrayData, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = 0);

        /**
        * @brief Create a new UInt32Array
        *
        * @param[in] numberOfIndices The number of indices in the UInt32Array
        * @param[in] arrayData Pointer to the uint32_t data to be used to create the array from.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The optional name of the UInt32Array
        * @return A pointer to the created UInt32Array, null on failure
        */
        const UInt32Array* createConstUInt32Array(uint32_t numberOfIndices, const uint32_t* arrayData, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = 0);

        /**
        * @brief Saves all scene contents (including all client resources) to a file.
        *
        * @param[in] scene Scene to save to a file.
        * @param[in] fileName File name to save the scene to.
        * @param[in] resourceFileInformation The distribution of resources among resource files to use
        * @param[in] compress if set to true, resources might be compressed before saving
        *                     otherwise, uncompressed data will be saved
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t saveSceneToFile(const Scene& scene, const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation, bool compress) const;

        /**
        * @brief Loads scene contents and resources from a file.
        *        The file format has to match current Ramses SDK version in major and minor version number.
        *        This method is not back compatible and will fail
        *        if trying to load scene files saved using older Ramses SDK version.
        *
        * @param[in] fileName File name to load the scene from.
        * @param[in] resourceFileInformation Description of resource files to be used for loading
        * @return New instance of scene with contents loaded from a file.
        */
        Scene* loadSceneFromFile(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation);

        /**
        * @brief Loads scene contents and resources asynchronously from a file.
        *        The file format has to match current Ramses SDK version in major and minor version number.
        *        This method is not backwards compatible and will fail
        *        if trying to load scene files saved using older Ramses SDK version.
        *
        *        This method does not directly return a scene but provides its results
        *        after loading is finished when calling dispatchEvents().
        *        There will be one event for each loaded resource file followed by an
        *        event for the scene file.
        *
        * @param[in] fileName File name to load the scene from.
        * @param[in] resourceFileInformation Description of resource files to be used for loading
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t loadSceneFromFileAsync(const char* fileName, const ResourceFileDescriptionSet& resourceFileInformation);

        /**
        * @brief Marks the scene with sceneId as valid for local only
        *        optimization. This has the same effect as calling
        *        SceneConfig::setPublicationMode(EScenePublicationMode_LocalOnly) before saving.
        *
        *        Important: This function must be called before loading the file to be effective.
        *
        * @param[in] sceneId The scene that should be marked for local only optimization
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage() on client.
        */
        status_t markSceneIdForLoadingAsLocalOnly(sceneId_t sceneId);

        /**
        * @brief Destroys the given Scene. The reference of Scene is invalid after this call
        *
        * @param[in] scene The Scene to destroy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroy(Scene& scene);

        /**
        * @brief Destroys the given Resource.The reference of Resource is invalid after this call
        *
        * @param[in] resource The Resource to destroy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroy(const Resource& resource);

        /**
        * Stores internal data for implementation specifics of the API.
        */
        class RamsesClientImpl& impl;

        /**
        *  @brief Saves selected resources to a file.
        *
        *  @param fileDescription Contains the information about the filename and the
        *                         resources, which should be saved.
        *  @param compress if set to true, resources might be compressed before saving
        *                  otherwise, uncompressed data will be saved
        *  @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
        */
        status_t saveResources(const ResourceFileDescription& fileDescription, bool compress) const;

        /**
        *  @brief Saves selected resources to multiple files.
        *
        *  @param fileDescriptions A vector containing several ResourceFileDescriptions. Each of
        *                          them containing the information about the filename and the
        *                          resources, which should be saved.
        *  @param compress if set to true, resources might be compressed before saving
        *                  otherwise, uncompressed data will be saved
        *  @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
        */
        status_t saveResources(const ResourceFileDescriptionSet& fileDescriptions, bool compress) const;

        /**
        *  @brief Loads resources from a file.
        *         The file format has to match current Ramses SDK version in major and minor version number.
        *         This method is not back compatible and will fail
        *         if trying to load files saved using older Ramses SDK version.
        *
        *  @param fileDescription Contains the filename of the resources file.
        *  @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
        */
        status_t loadResources(const ResourceFileDescription& fileDescription) const;

        /**
        *  @brief Loads resources from multiple files.
        *          The file format has to match current Ramses SDK version in major and minor version number.
        *          This method is not back compatible and will fail
        *          if trying to load files saved using older Ramses SDK version.
        *
        *  @param fileDescriptions A vector containing several ResourceFileDescriptions. Each of
        *                          them containing the filename.
        *  @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t loadResources(const ResourceFileDescriptionSet& fileDescriptions) const;

        /**
        *  @brief Loads resources asynchronously from a file.
        *         The file format has to match current Ramses SDK version in major and minor version number.
        *         This method is not backwards compatible and will fail
        *         if trying to load files saved using older Ramses SDK version.
        *
        *         This method returns directly without waiting for the resources to load.
        *         To get the information when the loading of this resource file is finished
        *         dispatchEvents() can be called.
        *         There will be one event generated by this method call.
        *
        *  @param fileDescription Contains the filename of the resources file.
        *  @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
        */
        status_t loadResourcesAsync(const ResourceFileDescription& fileDescription);

        /**
        *  @brief Loads resources asynchronously from multiple files.
        *          The file format has to match current Ramses SDK version in major and minor version number.
        *          This method is not backwards compatible and will fail
        *          if trying to load files saved using older Ramses SDK version.
        *
        *         This method returns directly without waiting for the resources to load.
        *         To get the information when the loading of each of the resource files
        *         is finished dispatchEvents() can be called.
        *         There will be an event for each file in the set generated by this
        *         method call.
        *
        *  @param fileDescriptions A vector containing several ResourceFileDescriptions. Each of
        *                          them containing the filename.
        *  @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t loadResourcesAsync(const ResourceFileDescriptionSet& fileDescriptions);

        /**
        *  @brief Forces ramses to close given resource file.
        *         The file must have been loaded previously and its *NOT*
        *         guaranteed that the file is closed immediately or before return of the function.
        *         The user is responsible that no resources are needed to be loaded after closing the file
        *
        *  @param fileDescription Contains the filename of the resources file to close.
        *  @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
        */
        status_t forceCloseResourceFileAsync(const ResourceFileDescription& fileDescription) const;

        /**
        *  @brief Forces ramses to close given resource files.
        *         The file must have been loaded previously and its *NOT*
        *         guaranteed that the file is closed immediately or before return of the function.
        *         The user is responsible that no resources are needed to be loaded after closing the file
        *
        *  @param fileDescriptions Contains the filenames of the resources files to close.
        *  @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
        */
        status_t forceCloseResourceFilesAsync(const ResourceFileDescriptionSet& fileDescriptions) const;

        /**
        * @brief Create a new Texture2D
        *
        * @param[in] width Width of the texture (mipmap level 0).
        * @param[in] height Height of the texture (mipmap level 0).
        * @param[in] format Pixel format of the Texture2D data.
        * @param[in] mipMapCount Number of mipmap levels contained in mipLevelData array.
        * @param[in] mipLevelData Array of MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order is lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Texture2D.
        * @return A pointer to the created Texture2D, null on failure. Will fail with data == NULL and/or width/height == 0.
        */
        Texture2D* createTexture2D(
            uint32_t width,
            uint32_t height,
            ETextureFormat format,
            uint32_t mipMapCount,
            const MipLevelData mipLevelData[],
            bool generateMipChain = false,
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = 0);

        /**
        * @brief Create a new Texture3D
        *
        * @param[in] width Width of the texture (mipmap level 0).
        * @param[in] height Height of the texture (mipmap level 0).
        * @param[in] depth Depth of the texture.
        * @param[in] format Pixel format of the Texture3D data.
        * @param[in] mipMapCount Number of mipmap levels contained in mipLevelData array.
        * @param[in] mipLevelData Array of MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order is lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Texture3D.
        * @return A pointer to the created Texture3D, null on failure. Will fail with data == NULL and/or width/height/depth == 0.
        */
        Texture3D* createTexture3D(
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            ETextureFormat format,
            uint32_t mipMapCount,
            const MipLevelData mipLevelData[],
            bool generateMipChain = false,
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = 0);

        /**
        * @brief Create a new Cube Texture. All texel values are initially initialized to 0.
        *
        * @param[in] size edge length of one quadratic cube face, belonging to the texture.
        * @param[in] format Pixel format of the Cube Texture data.
        * @param[in] mipMapCount Number of mipmaps contained in mipLevelData array.
        * @param[in] mipLevelData Array of MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order ist lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Cube Texture.
        * @return A pointer to the created Cube Texture, null on failure. Will fail with any face-data == NULL and/or size == 0.
        */
        TextureCube* createTextureCube(
            uint32_t size,
            ETextureFormat format,
            uint32_t mipMapCount,
            const CubeMipLevelData mipLevelData[],
            bool generateMipChain = false,
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = 0);

        /**
        * @brief Create a new Effect by parsing a GLSL shader described by an EffectDescription instance.
        *
        * @param[in] effectDesc Effect description.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the created Effect.
        * @return A pointer to the created Effect, null on failure
        */
        Effect* createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = 0);

        /**
        * @brief Get an object from the client by name.
        *        Only resource and scene names are searched.
        *
        * @param[in] name The name of the object to get.
        * @return Pointer to the object if found, NULL otherwise.
        */
        const RamsesObject* findObjectByName(const char* name) const;

        /**
        * @copydoc findObjectByName(const char*) const
        **/
        RamsesObject* findObjectByName(const char* name);

        /**
        * @brief Get a resource from the client by id
        *
        * @param[in] id The resource id of the resource to get.
        * @return Pointer to the resource if found, NULL otherwise.
        */
        const Resource* findResourceById(resourceId_t id) const;

        /**
        * @copydoc findResourceById(resourceId_t id) const
        **/
        Resource* findResourceById(resourceId_t id);

        /**
        * @brief Some methods on the client provide asynchronous results. These can be synchronously
        *        received by calling this functions at regular intervals.
        *
        *        The point in time and the ordering of asynchronous events from different calls is
        *        unspecified. The ordering of events from one asynchronous load call is as specified
        *        in the documentation for that call.
        *
        * @param[in] clientEventHandler User class that implements the callbacks that can be triggered if a corresponding event happened.
        *                               Check IClientEventHandler documentation for more details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t dispatchEvents(IClientEventHandler& clientEventHandler);

    };
}

#endif
