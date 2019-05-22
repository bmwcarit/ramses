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
#include "ramses-client-api/AnimationSystemEnums.h"
#include "ramses-client-api/EScenePublicationMode.h"
#include "ramses-client-api/EDataType.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class Node;
    class MeshNode;
    class RemoteCamera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Appearance;
    class Effect;
    class GeometryBinding;
    class SceneImpl;
    class RamsesClientImpl;
    class AnimationSystem;
    class AnimationSystemRealTime;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class BlitPass;
    class RenderTarget;
    class RenderTargetDescription;
    class TextureSampler;
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
    class StreamTexture;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class IndexDataBuffer;
    class VertexDataBuffer;
    class Texture2DBuffer;
    class SceneObject;

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
        bool isPublished() const;

        /**
        * @brief Returns scene id defined at scene creation time
        *
        * @return Scene id.
        */
        sceneId_t getSceneId() const;

        /**
        * @brief Creates a Remote Camera in this Scene
        *
        * @param[in] name The optional name of the Remote Camera
        * @return Pointer to the created RemoteCamera, null on failure
        */
        RemoteCamera* createRemoteCamera(const char* name = 0);

        /**
        * @brief Creates a Perspective Camera in this Scene
        *
        * @param[in] name The optional name of the Camera
        * @return Pointer to the created Camera, null on failure
        */
        PerspectiveCamera* createPerspectiveCamera(const char* name = 0);

        /**
        * @brief Creates a Orthographic Camera in this Scene
        *
        * @param[in] name The optional name of the Camera
        * @return Pointer to the created Camera, null on failure
        */
        OrthographicCamera* createOrthographicCamera(const char* name = 0);

        /**
        * @brief Creates a new Appearance.
        *
        * @param[in] effect The effect from which this appearance takes its parameters
        * @param[in] name The optional name of the created Appearance.
        * @return A pointer to the created Appearance, null on failure
        */
        Appearance* createAppearance(const Effect& effect, const char* name = 0);

        /**
        * @brief Creates a new GeometryBinding.
        *
        * @param[in] effect The effect which is used to create the GeometryBinding.
        * @param[in] name The optional name of the created GeometryBinding.
        * @return A pointer to the created GeometryBinding, null on failure
        */
        GeometryBinding* createGeometryBinding(const Effect& effect, const char* name = 0);

        /**
        * @brief Create a Stream Texture
        *
        * @param[in] fallbackTexture Texture2D used as a fallback texture.
        * @param[in] source Stream source identifier
        * @param[in] name The name of the Stream Texture.
        * @return A pointer to the created Stream Texture, null on failure.
        */
        StreamTexture* createStreamTexture(const Texture2D& fallbackTexture, streamSource_t source, const char* name = 0);

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
        Node* createNode(const char* name = 0);

        /**
         * @brief Creates a scene graph MeshNode.
         * MeshNode is a Node with additional properties and bindings that represent
         * a renderable geometry with appearance.
         *
         * @param[in] name The optional name of the MeshNode.
         * @return Pointer to the created MeshNode, null on failure.
        */
        MeshNode* createMeshNode(const char* name = 0);

        /**
        * @brief Destroys a previously created object using this scene
        * The object must be owned by this scene in order to be destroyed.
        * The reference to the object is no longer valid after it is destroyed.
        *
        * @param object The object of the Scene to destroy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroy(SceneObject& object);

        /**
         * @brief Expiration timestamp is a point in time till which the scene is considered to be up-to-date.
         *        Logic on renderer side will check the time every frame and in case it detects the scene
         *        to be rendered after its expiration timestamp it will generate an event (invoke \c IRendererEventHandler::sceneExpired).
         *
         *        IMPORTANT: Expiration timestamp value is bound to current state of scene (once it is flushed) and for all subsequent flushes until changed again or disabled.
         *                   Once expiration timestamp is set to non-zero all subscribed renderers will periodically check it from that point on.
         *                   User is responsible for calling this method to keep the expiration reasonably in future.
         *
         *        To disable the expiration checking set expiration timestamp to 0 followed by a flush.
         *        By default the expiration checking is disabled.
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
        * @brief Get an object from the scene by name
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
        * Stores internal data for implementation specifics of Scene.
        */
        SceneImpl& impl;

        /**
        * @brief Create a RenderGroup instance in the scene.
        *
        * @param[in] name The optional name of the created RenderGroup instance.
        * @return A pointer to the created RenderGroup, null on failure
        **/
        RenderGroup* createRenderGroup(const char* name = 0);

        /**
        * @brief Create a render pass in the scene.
        *
        * @param[in] name The optional name of the created render pass.
        * @return A render pass.
        **/
        RenderPass* createRenderPass(const char* name = 0);

        /**
        * @brief Create a blit pass in the scene.
        * Source and destination render buffers must have same type, format and dimensions.
        * By default the blitting region is set to the whole render buffers.
        *
        * @param[in] sourceRenderBuffer Render buffer used as source to blit from
        * @param[in] destinationRenderBuffer Render buffer used as destination to blit to
        * @param[in] name The optional name of the created blit pass.
        * @return A pointer to a BlitPass if successful or NULL on failure.
        **/
        BlitPass* createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, const char* name = 0);

        /**
        * @brief Create a RenderBuffer to be used with RenderTarget for rendering into and TextureSampler for sampling from.
        *
        * @param[in] width The width of the RenderBuffer in pixel.
        * @param[in] height The height of the RenderBuffer in pixel.
        * @param[in] bufferType Type of the RenderBuffer to be created (color, depth, depth/stecil).
        * @param[in] bufferFormat Data format to use for the RenderBuffer to be created.
        * @param[in] accessMode Read/Write access mode of render buffer
        * @param[in] sampleCount Optional sample count for MSAA number of samples. Can only be set for write-only render buffers.
                                 Default value is Zero, which disables MSAA for the render buffer. Trying to create read/write render
                                 buffer with sampleCount other than Zero will fail.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created RenderBuffer, null on failure.
        **/
        RenderBuffer* createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount = 0u, const char* name = 0);

        /**
        * @brief Create a render target providing a set of RenderBuffers.
        *
        * @param[in] rtDesc Instance of RenderTargetDescription holding all information needed to create a RenderTarget.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created RenderTarget, null on failure.
        **/
        RenderTarget* createRenderTarget(const RenderTargetDescription& rtDesc, const char* name = 0);

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
            const char* name = 0);

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
            const char* name = 0);

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
            const char* name = 0);

        /**
        * @brief Creates a texture sampler object.
        * @param[in] wrapUMode texture wrap mode for u axis.
        * @param[in] wrapVMode texture wrap mode for v axis.
        * @param[in] minSamplingMethod texture min sampling method.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] renderBuffer RenderBuffer to be used with this sampler object. The render buffer must have access mode of read/write.
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
            const char* name = 0);

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
            const char* name = 0);

        /**
        * @brief Creates a texture sampler object.
        * @param[in] wrapUMode texture wrap mode for u axis.
        * @param[in] wrapVMode texture wrap mode for v axis.
        * @param[in] minSamplingMethod texture min sampling method.
        * @param[in] magSamplingMethod texture mag sampling method. Must be set to either Nearest or Linear.
        * @param[in] streamTexture StreamTexture to be used with this sampler object.
        * @param[in] name Optional name of the object.
        * @return Pointer to the created TextureSampler, null on failure.
        */
        TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const StreamTexture& streamTexture,
            const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type float.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataFloat, null on failure.
        */
        DataFloat* createDataFloat(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector2f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector2f, null on failure.
        */
        DataVector2f* createDataVector2f(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector3f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector3f, null on failure.
        */
        DataVector3f* createDataVector3f(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector4f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector4f, null on failure.
        */
        DataVector4f* createDataVector4f(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Matrix22f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created Matrix22f, null on failure.
        */
        DataMatrix22f* createDataMatrix22f(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Matrix33f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created Matrix33f, null on failure.
        */
        DataMatrix33f* createDataMatrix33f(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Matrix44f.
        * @param[in] name optional name of the object.
        * @return Pointer to the created Matrix44f, null on failure.
        */
        DataMatrix44f* createDataMatrix44f(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type int32.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataInt32, null on failure.
        */
        DataInt32* createDataInt32(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector2i.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector2i, null on failure.
        */
        DataVector2i* createDataVector2i(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector3i.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector3i, null on failure.
        */
        DataVector3i* createDataVector3i(const char* name = 0);

        /**
        * @brief Creates a data object within the scene, which holds a data value of type Vector4i.
        * @param[in] name optional name of the object.
        * @return Pointer to the created DataVector4i, null on failure.
        */
        DataVector4i* createDataVector4i(const char* name = 0);

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
        * @brief Annotates a TextureSampler as a content consumer.
        *        Texture provider and texture consumer can be linked on Ramses Renderer side.
        *        Linking textures means that the consumer's sampler will use provider's texture as content.
        * @param[in] sampler which shall consume texture content from provider texture.
        * @param[in] dataId id to reference the consumer in this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t dataId);

        /**
        * @brief Create a new animation system. The animation system will be
        * updated on renderer side after calls to AnimationSystem::setTime().
        * The animation system is not automatically updated on client side.
        * If live updates of animated values are needed on client side, provide
        * the creation flag EAnimationSystemFlags_ClientSideProcessing. Calls to
        * AnimationSystem::setTime() then also update the animation systems
        * client side state.
        *
        * @param[in] flags Optional creation flags for the animation system.
        * @param[in] name The optional name of the created animation system.
        * @return A reference to the created animation system.
        */
        AnimationSystem* createAnimationSystem(uint32_t flags = EAnimationSystemFlags_Default, const char* name = 0);

        /**
        * @brief Create a new animation system that is designed to work with system
        * time. The animation system will be updated automatically every frame on
        * renderer side using its system time. The animation system is not
        * automatically updated on client side. If live updates of animated values
        * are needed on client side, provide the creation flag
        * EAnimationSystemFlags_ClientSideProcessing, and make sure to call
        * AnimationSystem::updateLocalTime() before accessing any values. Calls
        * to AnimationSystem::updateLocalTime() are also mandatory before any
        * client side changes to the state of the animation system.
        *
        * @param[in] flags Optional creation flags for the animation system.
        * @param[in] name The optional name of the created animation system.
        * @return A reference to the created animation system.
        */
        AnimationSystemRealTime* createRealTimeAnimationSystem(uint32_t flags = EAnimationSystemFlags_Default, const char* name = 0);

        /**
        * @brief Create a new IndexDataBuffer. The created object can be used as a mutable resource
        * as index buffer in GeometryBinding. The created resource has mutable contents and immutable size that has to be specified
        * at creation time. Upon creation the contents of the resource data are undefined. The contents of the resource
        * data can be (partially) updated, where the change to the resource data is transferred to renderer on next flush.
        *
        * @param[in] maximumSizeInBytes Maximum size of the resource data.
        * @param[in] dataType Data type of the resource data. Must be an integral data type.
        * @param[in] name The optional name of the created index data buffer.
        * @return A reference to the created index data buffer.
        */
        IndexDataBuffer* createIndexDataBuffer(uint32_t maximumSizeInBytes, EDataType dataType, const char* name = 0);

        /**
        * @brief Create a new VertexDataBuffer. The created object can be used as a mutable resource
        * as vertex buffers in GeometryBinding. The created resource has mutable contents and immutable size that has to be specified
        * at creation time. Upon creation the contents of the resource data are undefined. The contents of the resource
        * data can be (partially) updated, where the change to the resource data is transferred to renderer on next flush.
        *
        * @param[in] maximumSizeInBytes Maximum size of the resource data.
        * @param[in] dataType Data type of the resource data. Must be a float or float vector data type.
        * @param[in] name The optional name of the created vertex data buffer.
        * @return A reference to the created vertex data buffer.
        */
        VertexDataBuffer* createVertexDataBuffer(uint32_t maximumSizeInBytes, EDataType dataType, const char* name = 0);

        /**
        * @brief Create a new Texture2DBuffer. The created object can be used as a mutable resource
        * in TextureSampler objects. The created resource has mutable contents and immutable size that has to be specified
        * at creation time. Upon creation the contents of the resource data are undefined. The contents of the resource
        * data can be (partially) updated, where the change to the resource data is transferred to renderer on next flush.
        *
        * The sizes of mipmap levels are computed according to OpenGL specification (see documentation of glTexStorage2D).
        * The mipLevelCount has to be consistent with the width and height, so that no mipMap will have size zero.
        *
        * @param[in] mipLevelCount Number of mipmap levels created for the Texture2DBuffer.
        * @param[in] width width of the first and largest mipmap level.
        * @param[in] height height of the first and largest mipmap level.
        * @param[in] textureFormat texture format. Only uncompressed texture formats are supported
        * @param[in] name The optional name of the created Texture2DBuffer.
        * @return A reference to the created Texture2DBuffer.
        */
        Texture2DBuffer* createTexture2DBuffer(uint32_t mipLevelCount, uint32_t width, uint32_t height, ETextureFormat textureFormat, const char* name = 0);

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
        virtual ~Scene();
    };
}

#endif
