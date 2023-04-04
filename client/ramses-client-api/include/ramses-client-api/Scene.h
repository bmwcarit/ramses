//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_H
#define RAMSES_SCENE_H

#include "ramses-client-api/ClientObject.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/EScenePublicationMode.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "ramses-framework-api/EDataType.h"
#include "ramses-framework-api/DataTypes.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include <string>

namespace ramses
{
    class Node;
    class MeshNode;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Appearance;
    class Effect;
    class GeometryBinding;
    class SceneImpl;
    class RamsesClientImpl;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class BlitPass;
    class PickableObject;
    class RenderTarget;
    class RenderTargetDescription;
    class TextureSampler;
    class TextureSamplerMS;
    class TextureSamplerExternal;
    class AttributeInput;
    class DataObject;
    class DataFloat;
    class DataVector2f;
    class DataVector3f;
    class DataVector4f;
    class DataMatrix22f;
    class DataMatrix33f;
    class DataMatrix44f;
    class DataInt32;
    class DataVector2i;
    class DataVector3i;
    class DataVector4i;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class ArrayBuffer;
    class Texture2DBuffer;
    class SceneReference;
    class SceneObject;
    class RamsesClient;
    class ArrayResource;
    class EffectDescription;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class Effect;
    class Resource;

    /**
     * @brief The Scene holds a scene graph.
     * It is the essential class for distributing
     * content to the ramses system.
    */
    class RAMSES_API Scene : public ClientObject
    {
    public:
        /**
        * @brief Publishes the scene to the ramses system
        *
        * Other ramses system participants will be informed about the existence
        * of the scene and can subscribe to it.
        * The scene content is then sent to all subscribers and following
        * flushes will send scene updates to them.
        *
        * @param[in] publicationMode A flag to signal if a scene should be only available
        * to local renderer(s) and not published outside of the client. By default, scene
        * is published and everyone in the RAMSES world can subscribe to it

        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t publish(EScenePublicationMode publicationMode = EScenePublicationMode_LocalAndRemote);

        /**
        * @brief Unpublish the scene from the ramses system
        *
        * The scene will be removed from the ramses system.
        * Subsequent changes to the scene will therefore not be propagated to the system.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t unpublish();

        /**
        * @brief Returns whether scene is currently published to the ramses system
        *
        * @return true, if scene is currently published.
        * @return false, if scene is currently not published.
        */
        [[nodiscard]] bool isPublished() const;

        /**
        * @brief Returns scene id defined at scene creation time
        *
        * @return Scene id.
        */
        [[nodiscard]] sceneId_t getSceneId() const;

        /**
        * @brief Saves all scene contents to a file.
        *
        * @param[in] fileName File name to save the scene to.
        * @param[in] compress if set to true, resources might be compressed before saving
        *                     otherwise, uncompressed data will be saved
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t saveToFile(const char* fileName, bool compress) const;

        /**
        * @brief Creates a Perspective Camera in this Scene
        *
        * @param[in] name The optional name of the Camera
        * @return Pointer to the created Camera, null on failure
        */
        PerspectiveCamera* createPerspectiveCamera(const char* name = nullptr);

        /**
        * @brief Creates a Orthographic Camera in this Scene
        *
        * @param[in] name The optional name of the Camera
        * @return Pointer to the created Camera, null on failure
        */
        OrthographicCamera* createOrthographicCamera(const char* name = nullptr);

        /**
        * @brief Creates a new Appearance.
        *
        * @param[in] effect The effect from which this appearance takes its parameters
        * @param[in] name The optional name of the created Appearance.
        * @return A pointer to the created Appearance, null on failure
        */
        Appearance* createAppearance(const Effect& effect, const char* name = nullptr);

        /**
        * @brief Creates a new GeometryBinding.
        *
        * @param[in] effect The effect which is used to create the GeometryBinding.
        * @param[in] name The optional name of the created GeometryBinding.
        * @return A pointer to the created GeometryBinding, null on failure
        */
        GeometryBinding* createGeometryBinding(const Effect& effect, const char* name = nullptr);

        /**
        * @brief Creates a scene graph node.
        * The basic purpose of Node is to define topology in scene graph
        * by links to parent and children nodes.
        * Node can also hold transformation which is then propagated to children,
        * thus defining a transformation topology. By default Node has identity transformation.
        * Node can also hold visibility information which is propagated to children,
        * thus defining a visibility topology. By default Node is visible, making
        * a Node invisible makes also its whole subgraph invisible regardless of sub-nodes' visibility state.
        *
        * @param[in] name Optional name of the object.
        * @return Pointer to the created Node, nullptr on failure.
        **/
        Node* createNode(const char* name = nullptr);

        /**
         * @brief Creates a scene graph MeshNode.
         * MeshNode is a Node with additional properties and bindings that represent
         * a renderable geometry with appearance.
         *
         * @param[in] name The optional name of the MeshNode.
         * @return Pointer to the created MeshNode, null on failure.
        */
        MeshNode* createMeshNode(const char* name = nullptr);

        /**
        * @brief Destroys a previously created object using this scene
        * The object must be owned by this scene in order to be destroyed.
        * The reference to the object is no longer valid after it is destroyed.
        * SceneObjects will automatically be destroyed once the scene is destroyed.
        *
        * @param object The object of the Scene to destroy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroy(SceneObject& object);

        /**
         * @brief   Expiration timestamp is a point in time till which the scene is considered to be up-to-date.
         * @details Logic on renderer side will check the time every frame and in case it detects the scene
         *          to be rendered after its expiration timestamp it will generate an event (#ramses::IRendererSceneControlEventHandler::sceneExpired).
         *
         *          IMPORTANT: Expiration timestamp value is bound to current state of scene (once it is flushed) and for all subsequent flushes until changed again or disabled.
         *                     Once expiration timestamp is set to non-zero all subscribed renderers will periodically check it from that point on.
         *                     User is responsible for calling this method to keep the expiration up-to-date during the lifecycle of the scene.
         *
         *          Setting the expiration timestamp to non-zero value enables the monitoring (#ramses::IRendererSceneControlEventHandler::sceneExpirationMonitoringEnabled),
         *          to disable the expiration monitoring set expiration timestamp to 0 followed by a flush.
         *          By default the expiration checking is disabled.
         *          If an expired scene recovers while monitoring is disabled, there will be no event of recovery after monitoring re-enabled.
         *          If scene is expired at the moment monitoring is enabled, there will be event of expiration
         *          (after #ramses::IRendererSceneControlEventHandler::sceneExpirationMonitoringEnabled).
         *
         * @param[in] ptpExpirationTimestampInMilliseconds Expiration timestamp in milliseconds from synchronized clock.
         *                                                 To avoid issues, keep this up-to-date reasonably enough in future.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds);

        /**
        * @brief Commits all changes done to the scene since the last flush or since scene creation. This makes a new
        *        valid scene state available to all local and remote renderers.
        *
        * @param[in] sceneVersionTag updates the version tag of the scene along with the flushed scene updates.
        *            If set to a valid value, a subscribed renderer will generate a sceneFlushed() event when the
        *            scene update has been applied. Invalid scene version ids are ignored.
        *            Defaults to InvalidSceneVersionTag.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t flush(sceneVersionTag_t sceneVersionTag = InvalidSceneVersionTag);

        /**
         * @brief resets the semantic uniform #ramses::EEffectUniformSemantic::TimeMs
         * The uniform value will contain the time elapsed since this method was called for the last time.
         * Use this method to avoid possible overflow issues.
         * The reset will be applied to the rendered scene with the next flush.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t resetUniformTimeMs();

        /**
        * @brief Gets the current value used for the semantic uniform #ramses::EEffectUniformSemantic::TimeMs
        * Value wraps to 0 every ~24 days, measured from the beginning of synchronized clock epoch.
        *
        * In order to avoid handling the wrap in the shader code or potential overflow issues the value should be reset using #ramses::Scene::resetUniformTimeMs()
        * The value is not reset automatically at startup.
        *
        * @return time in milliseconds, value range is 0 .. std::numeric_limits<int32_t>::max() (~24 days)
        */
        [[nodiscard]] int32_t getUniformTimeMs() const;

        /**
        * @brief Get an object from the scene by name
        *
        * @param[in] name The name of the object to get.
        * @return Pointer to the object if found, nullptr otherwise.
        */
        const RamsesObject* findObjectByName(const char* name) const;

        /**
        * @copydoc findObjectByName(const char*) const
        **/
        RamsesObject* findObjectByName(const char* name);

        /**
        * @brief Get an object from the scene by id
        *
        * @param[in] id The id of the object to get.
        * @return Pointer to the object if found, nullptr otherwise.
        */
        [[nodiscard]] const SceneObject* findObjectById(sceneObjectId_t id) const;

        /**
        * @copydoc findObjectById(sceneObjectId_t id) const
        **/
        SceneObject* findObjectById(sceneObjectId_t id);

        /**
        * Stores internal data for implementation specifics of Scene.
        */
        SceneImpl& impl;

        /**
        * @brief Create a RenderGroup instance in the scene.
        *
        * @param[in] name The optional name of the created RenderGroup instance.
        * @return A pointer to the created RenderGroup, null on failure
        **/
        RenderGroup* createRenderGroup(const char* name = nullptr);

        /**
        * @brief Create a render pass in the scene.
        *
        * @param[in] name The optional name of the created render pass.
        * @return A render pass.
        **/
        RenderPass* createRenderPass(const char* name = nullptr);

        /**
        * @brief Create a blit pass in the scene.
        * Source and destination render buffers must have same type, format and dimensions.
        * By default the blitting region is set to the whole render buffers.
        *
        * @param[in] sourceRenderBuffer Render buffer used as source to blit from
        * @param[in] destinationRenderBuffer Render buffer used as destination to blit to
        * @param[in] name The optional name of the created blit pass.
        * @return A pointer to a BlitPass if successful or nullptr on failure.
        **/
        BlitPass* createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, const char* name = nullptr);

        /**
        * @brief   Create a RenderBuffer to be used with RenderTarget for rendering into and TextureSampler for sampling from.
        * @details A multisampled buffer will be created if sampleCount greater than 0, note that the value is just a hint for the device,
        *          the actual number of samples might be different depending on device driver implementation.
        *          If the number of samples exceeds device capabilities the number of samples it will be clamped to its
        *          maximum supported (creation will succeeded with a warning log).
        *
        * @param[in] width The width of the RenderBuffer in pixel.
        * @param[in] height The height of the RenderBuffer in pixel.
        * @param[in] bufferType Type of the RenderBuffer to be created (color, depth, depth/stecil).
        * @param[in] bufferFormat Data format to use for the RenderBuffer to be created.
        * @param[in] accessMode Read/Write access mode of render buffer
        * @param[in] sampleCount Optional sample count for MSAA number of samples. Default value is 0 for no MSAA.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created RenderBuffer, null on failure.
        **/
        RenderBuffer* createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount = 0u, const char* name = nullptr);

        /**
        * @brief Create a PickableObject.
        * @details PickableObject provides a way to specify a 'pickable' area, when this area is picked (see ramses::RamsesRenderer API)
        *          a message is sent to RamsesClient with list of picked objects, these can be dispatched and handled using ramses::IRendererEventHandler::objectsPicked.
        *          Geometry to specify PickableObject has to be of data type ramses::EDataType::Vector3F and every 3 elements are vertices forming a triangle.
        *          Geometry will be interpreted as triangle list (no indices used) - it should be a simplified representation of the actual renderable geometry
        *          that it is assigned to, typically a bounding box.
        *          PickableObject is a ramses::Node and as such can be placed in scene transformation topology,
        *          the vertices should therefore be in local (model) space and transformations will be applied according node topology when calculating picking.
        *          Geometry is defined in 3D coordinates but does not have to be volumetric, in fact when combined with the right camera (ramses::PickableObject::setCamera)
        *          it can represent a screen space area.
        *
        * @param[in] geometryBuffer Vertex buffer containing triangles defining geometry of PickableObject.
        * @param[in] id User ID assigned to PickableObject, it will be used in callback ramses::IRendererEventHandler::objectsPicked when this PickableObject is picked.
        * @param[in] name Name of the PickableObject.
        * @return Pointer to the created PickableObject, nullptr on failure.
        **/
        PickableObject* createPickableObject(const ArrayBuffer& geometryBuffer, const pickableObjectId_t id, const char* name = nullptr);

        /**
        * @brief Create a render target providing a set of RenderBuffers.
        *
        * @param[in] rtDesc Instance of RenderTargetDescription holding all information needed to create a RenderTarget.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created RenderTarget, null on failure.
        **/
        RenderTarget* createRenderTarget(const RenderTargetDescription& rtDesc, const char* name = nullptr);

        /**
        * @brief Creates a texture sampler object.
        * @param[in] wrapUMode texture wrap mode for u axis.
        * @param[in] wrapVMode texture wrap mode for v axis.
        * @param[in] minSamplingMethod texture min sampling method.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] texture Texture to be used with this sampler object.
        * @param[in] anisotropyLevel Texture sampling anisotropy level.
        *            1: isotropic sampling, >1: anisotropic sampling
        *            usual values: 1, 2, 4, 8, 16 (depending on graphics platform)
        * @param[in] name Optional name of the object.
        * @return Pointer to the created TextureSampler, null on failure.
        */
        TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const Texture2D& texture,
            uint32_t anisotropyLevel = 1,
            const char* name = nullptr);

        /**
        * @brief Creates a texture sampler object.
        * @param[in] wrapUMode texture wrap mode for u axis.
        * @param[in] wrapVMode texture wrap mode for v axis.
        * @param[in] wrapRMode texture wrap mode for r axis.
        * @param[in] minSamplingMethod texture min sampling method.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] texture Texture to be used with this sampler object.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created TextureSampler, null on failure.
        */
        TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureAddressMode wrapRMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const Texture3D& texture,
            const char* name = nullptr);

        /**
        * @brief Creates a texture sampler object.
        * @param[in] wrapUMode texture wrap mode for u axis.
        * @param[in] wrapVMode texture wrap mode for v axis.
        * @param[in] minSamplingMethod texture min sampling method.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] texture Texture to be used with this sampler object.
        * @param[in] anisotropyLevel Texture sampling anisotropy level.
        *            1: isotropic sampling, >1: anisotropic sampling
        *            usual values: 1, 2, 4, 8, 16 (dependent on graphics platform)
        * @param[in] name Optional name of the object.
        * @return Pointer to the created TextureSampler, null on failure.
        */
        TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const TextureCube& texture,
            uint32_t anisotropyLevel = 1,
            const char* name = nullptr);

        /**
        * @brief Creates a texture sampler object.
        * @param[in] wrapUMode texture wrap mode for u axis.
        * @param[in] wrapVMode texture wrap mode for v axis.
        * @param[in] minSamplingMethod texture min sampling method.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] renderBuffer RenderBuffer to be used with this sampler object. The render buffer must have access mode of read/write and 0 samples (#ramses::RenderBuffer::getSampleCount).
        * @param[in] anisotropyLevel Texture sampling anisotropy level.
        *            1: isotropic sampling, >1: anisotropic sampling
        *            usual values: 1, 2, 4, 8, 16 (dependent on graphics platform)
        * @param[in] name Optional name of the object.
        * @return Pointer to the created TextureSampler, null on failure.
        */
        TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const RenderBuffer& renderBuffer,
            uint32_t anisotropyLevel = 1,
            const char* name = nullptr);

        /**
        * @brief Creates a texture sampler object for mutable texture.
        * @param[in] wrapUMode texture wrap mode for u axis.
        * @param[in] wrapVMode texture wrap mode for v axis.
        * @param[in] minSamplingMethod texture min sampling method.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] texture2DBuffer Texture2DBuffer to be used with this sampler object.
        * @param[in] anisotropyLevel Texture sampling anisotropy level.
        *            1: isotropic sampling, >1: anisotropic sampling
        *            usual values: 1, 2, 4, 8, 16 (dependent on graphics platform)
        * @param[in] name Optional name of the object.
        * @return Pointer to the created TextureSampler, null on failure.
        */
        TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const Texture2DBuffer& texture2DBuffer,
            uint32_t anisotropyLevel = 1,
            const char* name = nullptr);

        /**
        * @brief Creates a multisampled texture sampler object.
        * @param[in] renderBuffer RenderBuffer to be used with this sampler object. The render buffer must be multisampled and have access mode of read/write.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created #ramses::TextureSamplerMS, null on failure.
        */
        TextureSamplerMS* createTextureSamplerMS(const RenderBuffer& renderBuffer, const char* name);

        /**
        * @brief Creates a texture sampler object that can sample from external textures.
        *
        * @details Provides support for sampling from samplers represented by variables of type "samplerExternalOES" in GLSL shaders,
        *          according to OpenGL extension "OES_EGL_image_external".
        *          According to the spec, external texture samplers must have "clamp to edge" for the wrap mode, and are not allowed
        *          to have mip maps so minification filtering must be set to either linear or nearest.
        *
        *          Since it is not possible to directly provide content to external textures similar to conventional 2D textures, external
        *          textures can get their content only through the platform specific mechanisms. This can be done by creating a texture
        *          consumer from the sampler object and linking it to an external buffer created on the renderer.
        *
        *          Note: external texture sampler can only be used with external textures, i.e., it is not possible to use
        *          the same sampler variable in the shader to sample from conventional 2D textures as sort of fallback textures
        *          for example.
        *
        * @param[in] minSamplingMethod texture min sampling method. Must be set to either Nearest or Linear.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created TextureSampler, null on failure.
        */
        TextureSamplerExternal* createTextureSamplerExternal(
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const char* name = nullptr);

        /**
        * @brief Create a new #ramses::ArrayResource. It makes a copy of the given data of a certain type as a resource, an immutable data object.
        *        Data type must be a type that passes #ramses::IsArrayResourceDataType, i.e. one of #ramses::EDataType.
        *        See #ramses::ArrayResource for more details.
        *
        * @details If an #ramses::ArrayResource object is created with type #ramses::Byte (#ramses::EDataType::ByteBlob) then an element
        *          is defined as one byte, rather than a logical vertex element. Hence, functions of the class
        *          #ramses::ArrayResource referring to element refer to a single byte within byte array, element size is 1 byte
        *          and number of elements is the same as max size in bytes.
        *
        * @param[in] numElements The number of elements of the given data type to use for the resource.
        * @param[in] arrayData Pointer to the data to be used to create the array from.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The optional name of the ArrayResource.
        * @return A pointer to the created ArrayResource, null on failure
        */
        template <typename T>
        ArrayResource* createArrayResource(
            uint32_t numElements,
            const T* arrayData,
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = nullptr);

        /**
        * @brief Create a new Texture2D. It makes a copy of the given data of a certain type as a resource, an immutable data object.
        *        See #ramses::Texture2D for more details. See #ramses::MipLevelData for more details on expected texel alignment.
        *
        * @param[in] format Pixel format of the Texture2D data.
        * @param[in] width Width of the texture (mipmap level 0).
        * @param[in] height Height of the texture (mipmap level 0).
        * @param[in] mipMapCount Number of mipmap levels contained in mipLevelData array.
        * @param[in] mipLevelData Array of #ramses::MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order is lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] swizzle Describes how RGBA channels of the texture are swizzled,
        *          where each member of the struct represents one destination channel that the source channel should get sampled from.
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Texture2D.
        * @return A pointer to the created Texture2D, null on failure. Will fail with data == nullptr and/or width/height == 0.
        */
        Texture2D* createTexture2D(
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
        * @brief Create a new Texture3D. It makes a copy of the given data of a certain type as a resource, an immutable data object.
        *        See #ramses::Texture3D for more details. See #ramses::MipLevelData for more details on expected texel alignment.
        *
        * @param[in] format Pixel format of the Texture3D data.
        * @param[in] width Width of the texture (mipmap level 0).
        * @param[in] height Height of the texture (mipmap level 0).
        * @param[in] depth Depth of the texture.
        * @param[in] mipMapCount Number of mipmap levels contained in mipLevelData array.
        * @param[in] mipLevelData Array of #ramses::MipLevelData structs defining mipmap levels
        *                         to use. Amount and sizes of supplied mipmap levels have to
        *                         conform to GL specification. Order is lowest level (biggest
        *                         resolution) to highest level (smallest resolution).
        * @param[in] generateMipChain Auto generate mipmap levels. Cannot be used if custom data for lower mipmap levels provided.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the Texture3D.
        * @return A pointer to the created Texture3D, null on failure. Will fail with data == nullptr and/or width/height/depth == 0.
        */
        Texture3D* createTexture3D(
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
        * @brief Create a new Cube Texture. It makes a copy of the given data of a certain type as a resource, an immutable data object.
        *        All texel values are initially initialized to 0. See #ramses::TextureCube for more details. See #ramses::CubeMipLevelData for more details on expected texel alignment.
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
        * @return A pointer to the created Cube Texture, null on failure. Will fail with any face-data == nullptr and/or size == 0.
        */
        TextureCube* createTextureCube(
            ETextureFormat format,
            uint32_t size,
            uint32_t mipMapCount,
            const CubeMipLevelData mipLevelData[],
            bool generateMipChain = false,
            const TextureSwizzle& swizzle = {},
            resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache,
            const char* name = nullptr);

        /**
        * @brief Create a new Effect by parsing a GLSL shader described by an EffectDescription instance.
        *        Refer to RamsesClient::getLastEffectErrorMessages in case of parsing error.
        *        See #ramses::Effect for more details.
        *
        * @param[in] effectDesc Effect description.
        * @param[in] cacheFlag The optional flag sent to the renderer. The value describes how the cache implementation should handle the resource.
        * @param[in] name The name of the created Effect.
        * @return A pointer to the created Effect, null on failure
        */
        Effect* createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache, const char* name = nullptr);

        /**
         * @brief Get the GLSL error messages that were produced at the creation of the last Effect
         *
         * @return A string containing the GLSL error messages of the last effect
         */
        [[nodiscard]] std::string getLastEffectErrorMessages() const;

        /**
        * @brief Create a new #ramses::ArrayBuffer. The created object is a mutable buffer object that can be used as index or
        * as vertex buffer in #ramses::GeometryBinding.
        *
        * @details The created object has mutable contents and immutable size that has
        *          to be specified at creation time. Upon creation the contents are undefined. The contents of the object
        *          can be (partially) updated, the change to the object data is transferred to renderer on next flush.
        *
        *          Note: if an #ramses::ArrayBuffer object is created with type #ramses::EDataType::ByteBlob then an element
        *          is defined as one byte, rather than a logical vertex element. Hence, all functions of the class
        *          #ramses::ArrayBuffer referring to element refer to a single byte within byte array, element size is 1 byte
        *          and max number of elements is the same as max size in bytes.
        *
        * @param[in] dataType Data type of the array data. See #ramses::GetEDataType to get #ramses::EDataType from static type.
        * @param[in] maxNumElements The maximum number of data elements this buffer can hold.
        * @param[in] name The optional name of the created array buffer.
        * @return A pointer to the created array buffer.
        */
        ArrayBuffer* createArrayBuffer(EDataType dataType, uint32_t maxNumElements, const char* name = nullptr);

        /**
        * @brief Create a new Texture2DBuffer. The created object is a mutable buffer object that can be used
        * as a texture in TextureSampler. The created object has mutable contents and immutable size that has
        * to be specified at creation time. Upon creation the contents are undefined. The contents of the object
        * can be (partially) updated, the change to the object data is transferred to renderer on next flush.
        *
        * The sizes of mipmap levels are computed according to OpenGL specification (see documentation of glTexStorage2D).
        * The mipLevelCount has to be consistent with the width and height, so that no mipMap will have size zero.
        *
        * @param[in] textureFormat texture format. Only uncompressed texture formats are supported
        * @param[in] width width of the first and largest mipmap level.
        * @param[in] height height of the first and largest mipmap level.
        * @param[in] mipLevelCount Number of mipmap levels created for the Texture2DBuffer.
        * @param[in] name The optional name of the created Texture2DBuffer.
        * @return A pointer to the created Texture2DBuffer.
        */
        Texture2DBuffer* createTexture2DBuffer(ETextureFormat textureFormat, uint32_t width, uint32_t height, uint32_t mipLevelCount, const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type float.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataFloat, null on failure.
        */
        DataFloat* createDataFloat(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector2f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector2f, null on failure.
        */
        DataVector2f* createDataVector2f(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector3f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector3f, null on failure.
        */
        DataVector3f* createDataVector3f(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector4f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector4f, null on failure.
        */
        DataVector4f* createDataVector4f(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Matrix22f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created Matrix22f, null on failure.
        */
        DataMatrix22f* createDataMatrix22f(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Matrix33f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created Matrix33f, null on failure.
        */
        DataMatrix33f* createDataMatrix33f(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Matrix44f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created Matrix44f, null on failure.
        */
        DataMatrix44f* createDataMatrix44f(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type int32.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataInt32, null on failure.
        */
        DataInt32* createDataInt32(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector2i.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector2i, null on failure.
        */
        DataVector2i* createDataVector2i(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector3i.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector3i, null on failure.
        */
        DataVector3i* createDataVector3i(const char* name = nullptr);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector4i.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector4i, null on failure.
        */
        DataVector4i* createDataVector4i(const char* name = nullptr);

        /**
        * @brief Annotates a Node as a transformation data provider.
        *        Data provider and data consumer can be linked on Ramses Renderer side.
        *        Linking data means that the consumer's data property will be overridden by provider's data property.
        * @param[in] node from which transformation data shall be provided.
        * @param[in] dataId id to reference the provider node in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createTransformationDataProvider(const Node& node, dataProviderId_t dataId);

        /**
        * @brief Annotates a Node as a transformation data consumer.
        *        Data provider and data consumer can be linked on Ramses Renderer side.
        *        Linking data means that the consumer's data property will be overridden by provider's data property.
        * @param[in] node which shall consume data from another node.
        * @param[in] dataId id to reference the consumer node in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createTransformationDataConsumer(const Node& node, dataConsumerId_t dataId);

        /**
        * @brief Annotates a DataObject as a data provider.
        *        Data provider and data consumer can be linked on Ramses Renderer side.
        *        Linking data means that the consumer's data property will be overridden by provider's data property.
        * @param[in] dataObject from which data shall be provided.
        * @param[in] dataId id to reference the provider in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createDataProvider(const DataObject& dataObject, dataProviderId_t dataId);

        /**
        * @brief Annotates a DataObject as a data consumer.
        *        Data provider and data consumer can be linked on Ramses Renderer side.
        *        Linking data means that the consumer's data property will be overridden by provider's data property.
        * @param[in] dataObject which shall consume data from another DataObject.
        * @param[in] dataId id to reference the consumer in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createDataConsumer(const DataObject& dataObject, dataConsumerId_t dataId);

        /**
        * @brief Annotates a Texture2D as a content provider.
        *        Texture provider and texture consumer can be linked on Ramses Renderer side.
        *        Linking textures means that the consumer's sampler will use provider's texture as content.
        * @param[in] texture from which content shall be provided.
        * @param[in] dataId id to reference the provider in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createTextureProvider(const Texture2D& texture, dataProviderId_t dataId);

        /**
        * @brief Sets a new texture to an existing provider.
        *        This allows the provider to change the provided content which is then
        *        automatically applied to all linked consumers on the renderer side without the
        *        need to recreate or relink any provider/consumer.
        * @param[in] texture from which content shall be provided.
        * @param[in] dataId id to reference the provider in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t updateTextureProvider(const Texture2D& texture, dataProviderId_t dataId);

        /**
        * @brief Annotates a #ramses::TextureSampler as a content consumer.
        *        Texture provider and texture consumer can be linked on Ramses Renderer side.
        *        Linking textures means that the consumer's sampler will use provider's texture as content.
        * @param[in] sampler which shall consume texture content from provider texture.
        * @param[in] dataId id to reference the consumer in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t dataId);

        /**
        * @brief Annotates a #ramses::TextureSamplerMS as a content consumer.
        *        Texture provider and texture consumer can be linked on Ramses Renderer side.
        *        Linking textures means that the consumer's sampler will use provider's texture as content.
        * @param[in] sampler which shall consume texture content from provider texture.
        * @param[in] dataId id to reference the consumer in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createTextureConsumer(const TextureSamplerMS& sampler, dataConsumerId_t dataId);

        /**
        * @brief Annotates a #ramses::TextureSamplerExternal as a content consumer.
        *        Texture provider and texture consumer can be linked on Ramses Renderer side.
        *        Linking textures means that the consumer's sampler will use provider's texture as content.
        *
        *        Sampler for external texture can only be linked to an external buffer on Ramses Renderer side
        *        (#ramses::RamsesRenderer::createExternalBuffer).
        *
        * @param[in] sampler which shall consume texture content from provider texture.
        * @param[in] dataId id to reference the consumer in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createTextureConsumer(const TextureSamplerExternal& sampler, dataConsumerId_t dataId);

        /**
        * @brief Creates a new SceneReference object.
        * @details The SceneReference object references a scene, which might be unknown
        *          to this RamsesClient, but is or expected to be known to the RamsesRenderer subscribed to this scene.
        *          It allows to remotely change limited set of states of the referenced scene on renderer side.
        *          There can be only one instance of #ramses::SceneReference referring to a sceneId across all
        *          RamsesClients connected to a RamsesRenderer. Creating more than one instance referring to the same sceneId
        *          in one RamsesClient will result in an error, multiple instances referring to the same sceneId across
        *          different RamsesClients results in undefined behavior.
        *          #ramses::SceneReference can be destroyed and re-created but there are certain aspects to consider,
        *          see #ramses::SceneReference for details.
        *          More than one level of referencing, i.e. a master scene being also a scene reference
        *          for another master scene, is currently not supported.
        *
        * @param[in] referencedScene A scene id of a scene known to the RamsesRenderer.
        * @param[in] name The optional name of the created SceneReference.
        * @return A pointer to the created SceneReference.
        */
        SceneReference* createSceneReference(sceneId_t referencedScene, const char* name = nullptr);

        /**
        * @brief Tell the RamsesRenderer to link a data provider to a data consumer across two scenes.
        * @details Data provider and data consumer must be created via client scene API and their data type must match.
        *          Linking data means that the consumer's data property will be overridden by provider's data property.
        *          A consumer within a scene can be linked to exactly one provider.
        *
        *          A link between provider and consumer is possible, if they are either part of the this scene
        *          or one of its scene references.
        *
        *          If the function returns StatusOK, #ramses::IClientEventHandler::dataLinked will be emitted after the link is
        *          processed by renderer. The link will fail (reported via callback result argument) if either the data provider
        *          or data consumer does not exist or their data type does not match.
        *
        *          Scene reference must be known to renderer side before attempting to make a data link. Make sure any involved
        *          #ramses::SceneReference was reported to be in state Ready or higher
        *          (see #ramses::IClientEventHandler::sceneReferenceStateChanged).
        *
        *          Should one of the scene references of the data link go to state Available or Unavailable while the data link is active,
        *          the link will implicitly be destroyed and needs to be rerequested again.
        *
        *          If successful the operation can be assumed to be effective in the next frame consumer scene is rendered after flushed.
        *          If the data consumer is already linked to a provider (data or offscreen buffer), the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *
        * @param providerReference A pointer to a SceneReference created by this scene, or nullptr to use this scene.
        * @param providerId The id of the data provider within the providerScene or providerReference.
        * @param consumerReference A pointer to the SceneReference which consumes the data, or nullptr to use this scene.
        * @param consumerId The id of the data consumer within the consumerScene or consumerReference.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t linkData(SceneReference* providerReference, dataProviderId_t providerId, SceneReference* consumerReference, dataConsumerId_t consumerId);

        /**
        * @brief   Removes an existing link between two scenes (see #ramses::Scene::linkData).
        * @details If the function returns StatusOK, #ramses::IClientEventHandler::dataUnlinked will be emitted after it is
        *          processed by renderer. If successful the operation can be assumed to be effective in the next frame consumer
        *          scene is rendered after flushed.
        *
        * @param consumerReference The pointer to the SceneReference which consumes the data, or nullptr to use this scene.
        * @param consumerId The id of the data consumer within the consumerReference.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t unlinkData(SceneReference* consumerReference, dataConsumerId_t consumerId);

        /**
         * @brief Getter for #ramses::RamsesClient this Scene was created from
         *
         * @return the parent RamsesCLient
         */
        RamsesClient& getRamsesClient();

        /**
        * @brief Get a resource which is owned by the scene by id
        *
        * @param[in] id The resource id of the resource to get.
        * @return Pointer to the resource if found, nullptr otherwise.
        */
        [[nodiscard]] const Resource* getResource(resourceId_t id) const;

        /**
        * @copydoc getResource(resourceId_t id) const
        **/
        Resource* getResource(resourceId_t id);

    protected:
        /**
        * @brief RamsesClient is the factory for creating Scene instances.
        */
        friend class RamsesClientImpl;

        /**
        * @brief Constructor of the Scene
        *
        * @param[in] pimpl Internal data for implementation specifics of Scene (sink - instance becomes owner)
        */
        explicit Scene(SceneImpl& pimpl);

        /**
        * @brief Copy constructor of Scene
        *
        * @param[in] other Other instance of Scene class
        */
        Scene(const Scene& other);

        /**
        * @brief Assignment operator of Scene.
        *
        * @param[in] other Other instance of Scene class
        * @return This instance after assignment
        */
        Scene& operator=(const Scene& other);

        /**
         * @brief Destructor of the Scene
        */
        ~Scene() override;

    private:
        /// Internal implementation of #createArrayResource
        template <typename T> RAMSES_API ArrayResource* createArrayResourceInternal(uint32_t numElements, const T* arrayData, resourceCacheFlag_t cacheFlag, const char* name);
    };

    template <typename T> ArrayResource* Scene::createArrayResource(uint32_t numElements, const T* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        static_assert(IsArrayResourceDataType<T>(), "Unsupported data type!");
        return createArrayResourceInternal<T>(numElements, arrayData, cacheFlag, name);
    }
}

#endif
