//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEIMPL_H
#define RAMSES_SCENEIMPL_H

#include "ramses-client-api/EScenePublicationMode.h"
#include "ramses-client-api/EDataType.h"
#include "ramses-client-api/TextureEnums.h"

// internal
#include "ClientObjectImpl.h"
#include "RamsesObjectRegistry.h"
#include "ClientCommands/SceneCommandBuffer.h"
#include "AppearanceImpl.h"
#include "Components/FlushTimeInformation.h"

// ramses framework
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/EDataSlotType.h"
#include "SceneAPI/TextureSampler.h"
#include "AnimationAPI/IAnimationSystem.h"

#include "Collections/Pair.h"
#include "Utils/StatisticCollection.h"
#include <chrono>

namespace ramses_internal
{
    class IScene;
    class ClientScene;
}

namespace ramses
{
    class Appearance;
    class RamsesClientImpl;
    class Camera;
    class RemoteCamera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Appearance;
    class Node;
    class Effect;
    class MeshNode;
    class AnimationSystem;
    class AnimationSystemRealTime;
    class GeometryBinding;
    class AnimationSystemImpl;
    class AttributeInput;
    class NodeImpl;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class RenderTarget;
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
    class SceneConfigImpl;
    class RenderTargetDescriptionImpl;
    class BlitPass;
    class TextureSampler;
    class StreamTexture;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class IndexDataBuffer;
    class IndexDataBufferImpl;
    class VertexDataBuffer;
    class VertexDataBufferImpl;
    class Texture2DBuffer;
    class Texture2DBufferImpl;

    class SceneImpl final : public ClientObjectImpl
    {
    public:
        SceneImpl(ramses_internal::ClientScene& scene, const SceneConfigImpl& sceneConfig, RamsesClientImpl& ramsesClient);
        virtual ~SceneImpl();

        void             initializeFrameworkData();
        virtual void     deinitializeFrameworkData() override final;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override final;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override final;
        virtual status_t validate(uint32_t indent) const override;

        status_t            publish(EScenePublicationMode publicationMode = EScenePublicationMode_LocalAndRemote);
        status_t            unpublish();
        bool                isPublished() const;
        sceneId_t           getSceneId() const;
        EScenePublicationMode getPublicationModeSetFromSceneConfig() const;

        RemoteCamera*       createRemoteCamera(const char* name);
        PerspectiveCamera*  createPerspectiveCamera(const char* name);
        OrthographicCamera* createOrthographicCamera(const char* name);

        Appearance*         createAppearance(const Effect& effect, const char* name);
        AppearanceImpl*     createAppearanceImpl(const char* name);

        StreamTexture*      createStreamTexture(const Texture2D& fallbackTexture, streamSource_t source, const char* name);
        GeometryBinding*    createGeometryBinding(const Effect& effect, const char* name);

        Node*               createNode(const char* name);
        MeshNode*           createMeshNode(const char* name);

        ramses::RenderGroup*  createRenderGroup(const char* name);
        ramses::RenderPass*   createRenderPass(const char* name);
        ramses::BlitPass*     createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, const char* name);

        ramses::RenderBuffer* createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, const char* name);
        ramses::RenderTarget* createRenderTarget(const RenderTargetDescriptionImpl& rtDesc, const char* name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const Texture2D& texture,
            const char* name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureAddressMode wrapRMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const Texture3D& texture,
            const char* name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const TextureCube& texture,
            const char* name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const RenderBuffer& renderBuffer,
            const char* name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const Texture2DBuffer& textureBuffer,
            const char* name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const StreamTexture& streamTexture,
            const char* name);

        TextureSamplerImpl& createTextureSamplerImpl(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureAddressMode wrapRMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            ERamsesObjectType samplerType,
            ramses_internal::TextureSampler::ContentType contentType,
            ramses_internal::ResourceContentHash textureResourceHash,    // The sampler stores either a texture, or...
            ramses_internal::MemoryHandle contentHandle,                 // a render target's color buffer, or a texture buffer, or a stream texture
            const char* name /*= 0*/);

        DataFloat*     createDataFloat(const char* name);
        DataVector2f*  createDataVector2f(const char* name);
        DataVector3f*  createDataVector3f(const char* name);
        DataVector4f*  createDataVector4f(const char* name);
        DataMatrix22f* createDataMatrix22f(const char* name);
        DataMatrix33f* createDataMatrix33f(const char* name);
        DataMatrix44f* createDataMatrix44f(const char* name);
        DataInt32*     createDataInt32(const char* name);
        DataVector2i*  createDataVector2i(const char* name);
        DataVector3i*  createDataVector3i(const char* name);
        DataVector4i*  createDataVector4i(const char* name);

        status_t createTransformationDataProvider(const Node& node, dataProviderId_t id);
        status_t createTransformationDataConsumer(const Node& node, dataConsumerId_t id);
        status_t createDataProvider(const DataObject& dataObject, dataProviderId_t id);
        status_t createDataConsumer(const DataObject& dataObject, dataConsumerId_t id);
        status_t createTextureProvider(const Texture2D& texture, dataProviderId_t id);
        status_t updateTextureProvider(const Texture2D& texture, dataProviderId_t id);
        status_t createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t id);

        AnimationSystem*         createAnimationSystem(uint32_t flags, const char* name);
        AnimationSystemRealTime* createRealTimeAnimationSystem(uint32_t flags, const char* name);

        IndexDataBuffer*        createIndexDataBuffer(uint32_t maximumSizeInBytes, EDataType dataType, const char* name);
        IndexDataBufferImpl*    createIndexDataBufferImpl(uint32_t maximumSizeInBytes, EDataType dataType, const char* name);

        VertexDataBuffer*       createVertexDataBuffer(uint32_t maximumSizeInBytes, EDataType dataType, const char* name);
        VertexDataBufferImpl*   createVertexDataBufferImpl(uint32_t maximumSizeInBytes, EDataType dataType, const char* name);

        Texture2DBuffer*        createTexture2DBuffer (uint32_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, const char* name);
        Texture2DBufferImpl*    createTexture2DBufferImpl (uint32_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, const char* name);

        status_t destroy(SceneObject& object);

        status_t setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds);

        status_t flush(sceneVersionTag_t sceneVersion);

        const ramses_internal::ClientScene& getIScene() const;
        ramses_internal::ClientScene& getIScene();

        bool                containsSceneObject(const SceneObjectImpl& object) const;
        const RamsesObject* findObjectByName(const char* name) const;
        RamsesObject*       findObjectByName(const char* name);

        RamsesObjectRegistry&       getObjectRegistry();
        const RamsesObjectRegistry& getObjectRegistry() const;

        void enqueueSceneCommand( const ramses_internal::SceneCommand& command );
        void setSceneVersionForNextFlush(sceneVersionTag_t sceneVersion);

        ramses_internal::StatisticCollectionScene& getStatisticCollection();

    private:
        RenderPass* createRenderPassInternal(const char* name);
        void registerCreatedObject(SceneObject& object);
        AnimationSystemImpl& createAnimationSystemImpl(uint32_t flags, ERamsesObjectType type, const char* name);
        template <typename ObjectType, typename ObjectImplType>
        status_t createAndDeserializeObjectImpls(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count);

        void removeAllDataSlotsForNode(const Node& node);

        template <typename OBJECT, typename CONTAINER>
        void removeObjectFromAllContainers(const OBJECT& object);

        void markAllChildrenDirty(Node& node);

        bool cameraIsAssignedToRenderPasses(const Camera& camera);

        status_t destroyRenderTarget(RenderTarget& renderTarget);
        status_t destroyCamera(Camera& camera);
        status_t destroyRenderGroup(RenderGroup& group);
        status_t destroyMeshNode(MeshNode& mesh);
        status_t destroyAnimationSystem(AnimationSystem& animationSystem);
        status_t destroyNode(Node& node);
        status_t destroyDataObject(DataObject& dataObject);
        status_t destroyTextureSampler(TextureSampler& sampler);
        status_t destroyObject(SceneObject& object);

        typedef std::pair<NodeImpl*, bool> NodeVisibilityPair;
        typedef std::vector<NodeVisibilityPair> NodeVisibilityInfoVector;

        void applyVisibilityToSubtree(NodeImpl& node, bool visibilityToApply);
        void prepareListOfDirtyNodesForHierarchicalVisibility(NodeVisibilityInfoVector& nodesToProcess);
        void applyHierarchicalVisibility();

        ramses_internal::ClientScene&           m_scene;
        ramses_internal::SceneCommandBuffer     m_commandBuffer;
        sceneVersionTag_t                       m_nextSceneVersion;

        RamsesObjectRegistry m_objectRegistry;

        // This is essentially a local variable only used in the "applyVisibilityToSubtree" method.
        // This is for performance reasons, so we can re-use the same vector each time the method is called.
        NodeVisibilityInfoVector m_dataStackForSubTreeVisibilityApplying;
        EScenePublicationMode m_futurePublicationMode;

        ramses_internal::FlushTime::Clock::time_point m_expirationTimestamp;
    };
}

#endif
