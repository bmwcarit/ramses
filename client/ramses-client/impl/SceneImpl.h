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
#include "ramses-client-api/EVisibilityMode.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/SceneReference.h"
#include "ramses-client-api/MipLevelData.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "ramses-framework-api/EDataType.h"

// internal
#include "ClientObjectImpl.h"
#include "RamsesObjectRegistry.h"
#include "ClientCommands/SceneCommandBuffer.h"
#include "AppearanceImpl.h"
#include "Components/FlushTimeInformation.h"
#include "Components/SceneFileHandle.h"

// ramses framework
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/EDataSlotType.h"
#include "SceneAPI/TextureSampler.h"
#include "Resource/ResourceTypes.h"
#include "Components/ManagedResource.h"

#include "Collections/Pair.h"
#include "Utils/StatisticCollection.h"
#include "RamsesFrameworkTypesImpl.h"

#include <chrono>
#include <unordered_map>
#include <string_view>

namespace ramses_internal
{
    class TextureResource;
    class IScene;
    class ClientScene;
    class EffectResource;
}

namespace ramses
{
    class Appearance;
    class RamsesClientImpl;
    class Camera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Appearance;
    class Node;
    class Effect;
    class MeshNode;
    class GeometryBinding;
    class AttributeInput;
    class NodeImpl;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class RenderTarget;
    class DataObject;
    class SceneConfigImpl;
    class RenderTargetDescriptionImpl;
    class BlitPass;
    class PickableObject;
    class TextureSampler;
    class TextureSamplerMS;
    class TextureSamplerExternal;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class ArrayBuffer;
    class ArrayBufferImpl;
    class Texture2DBuffer;
    class Texture2DBufferImpl;
    class RamsesClient;
    class ArrayResource;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class Effect;
    class EffectDescription;
    class Resource;

    class SceneImpl final : public ClientObjectImpl
    {
    public:
        SceneImpl(ramses_internal::ClientScene& scene, const SceneConfigImpl& sceneConfig, RamsesClient& ramsesClient);
        ~SceneImpl() override;

        void             initializeFrameworkData();
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        status_t validate() const override;

        status_t            publish(EScenePublicationMode publicationMode = EScenePublicationMode::LocalAndRemote);
        status_t            unpublish();
        bool                isPublished() const;
        sceneId_t           getSceneId() const;
        EScenePublicationMode getPublicationModeSetFromSceneConfig() const;

        status_t saveToFile(std::string_view fileName, bool compress) const;

        PerspectiveCamera*  createPerspectiveCamera(std::string_view name);
        OrthographicCamera* createOrthographicCamera(std::string_view name);
        Appearance*         createAppearance(const Effect& effect, std::string_view name);
        GeometryBinding*    createGeometryBinding(const Effect& effect, std::string_view name);
        Node*               createNode(std::string_view name);
        MeshNode*           createMeshNode(std::string_view name);
        ramses::RenderGroup*  createRenderGroup(std::string_view name);
        ramses::RenderPass*   createRenderPass(std::string_view name);
        ramses::BlitPass*     createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, std::string_view name);
        ramses::PickableObject* createPickableObject(const ArrayBuffer& geometryBuffer, const pickableObjectId_t id, std::string_view name);
        ramses::RenderBuffer* createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, std::string_view name);
        ramses::RenderTarget* createRenderTarget(const RenderTargetDescriptionImpl& rtDesc, std::string_view name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const Texture2D& texture,
            std::string_view name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureAddressMode wrapRMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            const Texture3D& texture,
            std::string_view name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const TextureCube& texture,
            std::string_view name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const RenderBuffer& renderBuffer,
            std::string_view name);

        ramses::TextureSampler* createTextureSampler(
            ETextureAddressMode wrapUMode,
            ETextureAddressMode wrapVMode,
            ETextureSamplingMethod minSamplingMethod,
            ETextureSamplingMethod magSamplingMethod,
            uint32_t anisotropyLevel,
            const Texture2DBuffer& textureBuffer,
            std::string_view name);

        ramses::TextureSamplerMS* createTextureSamplerMS(const RenderBuffer& renderBuffer, std::string_view name);

        ramses::TextureSamplerExternal* createTextureSamplerExternal(
                ETextureSamplingMethod minSamplingMethod,
                ETextureSamplingMethod magSamplingMethod,
                std::string_view name);

        DataObject*     createDataObject(EDataType dataType, std::string_view name);

        status_t createTransformationDataProvider(const Node& node, dataProviderId_t id);
        status_t createTransformationDataConsumer(const Node& node, dataConsumerId_t id);
        status_t createDataProvider(const DataObject& dataObject, dataProviderId_t id);
        status_t createDataConsumer(const DataObject& dataObject, dataConsumerId_t id);
        status_t createTextureProvider(const Texture2D& texture, dataProviderId_t id);
        status_t updateTextureProvider(const Texture2D& texture, dataProviderId_t id);
        status_t createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t id);
        status_t createTextureConsumer(const TextureSamplerMS& sampler, dataConsumerId_t id);
        status_t createTextureConsumer(const TextureSamplerExternal& sampler, dataConsumerId_t id);

        ArrayBuffer*             createArrayBuffer(EDataType dataType, uint32_t maxNumElements, std::string_view name);
        Texture2DBuffer*         createTexture2DBuffer (uint32_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, std::string_view name);

        SceneReference* createSceneReference(sceneId_t referencedScene, std::string_view name);
        status_t linkData(SceneReference* providerReference, dataProviderId_t providerId, SceneReference* consumerReference, dataConsumerId_t consumerId);
        status_t unlinkData(SceneReference* consumerReference, dataConsumerId_t consumerId);

        status_t destroy(SceneObject& object);

        status_t setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds);

        status_t flush(sceneVersionTag_t sceneVersion);

        status_t resetUniformTimeMs();
        int32_t getUniformTimeMs() const;

        template <typename T>
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        ArrayResource* createArrayResource(uint32_t numElements, const T* arrayData, resourceCacheFlag_t cacheFlag, std::string_view name);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        Texture2D* createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, std::string_view name);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        Texture3D* createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, std::string_view name);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        TextureCube* createTextureCube(uint32_t size, ETextureFormat format, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, std::string_view name);
        Effect* createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, std::string_view name);
        std::string getLastEffectErrorMessages() const;

        ArrayResource* createHLArrayResource(ramses_internal::ManagedResource const& resource, std::string_view name);
        Texture2D* createHLTexture2D(ramses_internal::ManagedResource const& resource, std::string_view name);
        Texture3D* createHLTexture3D(ramses_internal::ManagedResource const& resource, std::string_view name);
        TextureCube* createHLTextureCube(ramses_internal::ManagedResource const& resource, std::string_view name);
        Effect* createHLEffect(ramses_internal::ManagedResource const& resource, std::string_view name);

        const ramses_internal::ClientScene& getIScene() const;
        ramses_internal::ClientScene& getIScene();

        bool                containsSceneObject(const SceneObjectImpl& object) const;
        const RamsesObject* findObjectByName(std::string_view name) const;
        RamsesObject*       findObjectByName(std::string_view name);
        Resource* getResource(resourceId_t rid) const;

        const SceneObject* findObjectById(sceneObjectId_t id) const;
        SceneObject* findObjectById(sceneObjectId_t id);

        RamsesObjectRegistry&       getObjectRegistry();
        const RamsesObjectRegistry& getObjectRegistry() const;

        void setSceneVersionForNextFlush(sceneVersionTag_t sceneVersion);

        sceneObjectId_t getNextSceneObjectId();

        ramses_internal::StatisticCollectionScene& getStatisticCollection();
        SceneReference* getSceneReference(sceneId_t referencedSceneId);

        RamsesClient& getHlRamsesClient();

        template <typename T>
        void enqueueSceneCommand(T commands);

        Resource* scanForResourceWithHash(ramses_internal::ResourceContentHash hash) const;

        template <typename ObjectType, typename ObjectImplType>
        status_t createAndDeserializeObjectImpls(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count);

        void setSceneFileHandle(ramses_internal::SceneFileHandle handle);
        void closeSceneFile();
        ramses_internal::SceneFileHandle getSceneFileHandle() const;

        void updateResourceId(resourceId_t const& oldId, Resource& resourceWithNewId);

    private:
        ramses::TextureSampler* createTextureSamplerImpl(
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
            std::string_view name /*= 0*/);

        template <typename SAMPLER>
        status_t createTextureConsumerImpl(const SAMPLER& sampler, dataConsumerId_t id, bool checkDuplicate = true);

        RenderPass* createRenderPassInternal(std::string_view name);

        template <typename T, typename ImplT>
        T& registerCreatedResourceObject(std::unique_ptr<ImplT> resourceImpl);

        void removeAllDataSlotsForNode(const Node& node);

        template <typename OBJECT, typename CONTAINER>
        void removeObjectFromAllContainers(const OBJECT& object);

        template <typename SAMPLER>
        status_t destroyTextureSampler(SAMPLER& sampler);

        void markAllChildrenDirty(Node& node);

        bool cameraIsAssignedToRenderPasses(const Camera& camera);

        status_t destroyRenderTarget(RenderTarget& renderTarget);
        status_t destroyCamera(Camera& camera);
        status_t destroyRenderGroup(RenderGroup& group);
        status_t destroyMeshNode(MeshNode& mesh);
        status_t destroyNode(Node& node);
        status_t destroyDataObject(DataObject& dataObject);
        status_t destroyResource(Resource& resource);
        status_t destroyObject(SceneObject& object);

        using NodeVisibilityPair = std::pair<NodeImpl*, EVisibilityMode>;
        using NodeVisibilityInfoVector = std::vector<NodeVisibilityPair>;

        void applyVisibilityToSubtree(NodeImpl& initialNode, EVisibilityMode initialVisibility);
        void prepareListOfDirtyNodesForHierarchicalVisibility(NodeVisibilityInfoVector& nodesToProcess);
        void applyHierarchicalVisibility();

        status_t writeSceneObjectsToStream(ramses_internal::IOutputStream& outputStream) const;

        bool removeResourceWithIdFromResources(resourceId_t const& id, Resource& resource);

        ramses_internal::ClientScene&           m_scene;
        ramses_internal::SceneCommandBuffer     m_commandBuffer;
        sceneVersionTag_t                       m_nextSceneVersion;
        sceneObjectId_t                         m_lastSceneObjectId;

        RamsesObjectRegistry                    m_objectRegistry;
        std::unordered_multimap <resourceId_t, Resource*>  m_resources;

        // This is essentially a local variable only used in the "applyVisibilityToSubtree" method.
        // This is for performance reasons, so we can re-use the same vector each time the method is called.
        NodeVisibilityInfoVector m_dataStackForSubTreeVisibilityApplying;
        EScenePublicationMode m_futurePublicationMode;

        ramses_internal::FlushTime::Clock::time_point m_expirationTimestamp{ ramses_internal::FlushTime::InvalidTimestamp };

        ramses_internal::HashMap<sceneId_t, SceneReference*> m_sceneReferences;

        RamsesClient& m_hlClient;

        std::string m_effectErrorMessages;

        ramses_internal::SceneFileHandle m_sceneFileHandle;

        bool m_sendEffectTimeSync = false;
    };

    // define here to allow inlining
    inline const ramses_internal::ClientScene& SceneImpl::getIScene() const
    {
        return m_scene;
    }

    inline ramses_internal::ClientScene& SceneImpl::getIScene()
    {
        return m_scene;
    }

    template <typename T>
    void SceneImpl::enqueueSceneCommand(T commands)
    {
        m_commandBuffer.enqueueCommand(std::move(commands));
    }
}

#endif
