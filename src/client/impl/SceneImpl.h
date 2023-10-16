//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/EScenePublicationMode.h"
#include "ramses/framework/EVisibilityMode.h"
#include "ramses/framework/TextureEnums.h"
#include "ramses/client/SceneReference.h"
#include "ramses/client/MipLevelData.h"
#include "ramses/client/TextureSwizzle.h"
#include "ramses/framework/EDataType.h"
#include "ramses/framework/Issue.h"

// internal
#include "impl/ClientObjectImpl.h"
#include "impl/SceneObjectRegistry.h"
#include "impl/AppearanceImpl.h"
#include "internal/ClientCommands/SceneCommandBuffer.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Components/SceneFileHandle.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/EDataSlotType.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "internal/SceneGraph/Resource/ResourceTypes.h"
#include "internal/Components/ManagedResource.h"

#include "internal/PlatformAbstraction/Collections/Pair.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "impl/RamsesFrameworkTypesImpl.h"

#include <chrono>
#include <unordered_map>
#include <string_view>

namespace ramses
{
    class Appearance;
    class Camera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Node;
    class Effect;
    class MeshNode;
    class Geometry;
    class AttributeInput;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class RenderTarget;
    class DataObject;
    class BlitPass;
    class PickableObject;
    class TextureSampler;
    class TextureSamplerMS;
    class TextureSamplerExternal;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class ArrayBuffer;
    class Texture2DBuffer;
    class RamsesClient;
    class ArrayResource;
    class EffectDescription;
    class Resource;
    class LogicEngine;
}

namespace ramses::internal
{
    class TextureResource;
    class IScene;
    class ClientScene;
    class EffectResource;
    class RamsesClientImpl;
    class NodeImpl;
    class SceneConfigImpl;
    class RenderTargetDescriptionImpl;
    class ArrayBufferImpl;
    class Texture2DBufferImpl;
    class SaveFileConfigImpl;

    class SceneImpl final : public ClientObjectImpl
    {
    public:
        SceneImpl(ramses::internal::ClientScene& scene, const SceneConfigImpl& sceneConfig, RamsesClient& ramsesClient);
        ~SceneImpl() override;

        void initializeFrameworkData();
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        void onValidate(ValidationReportImpl& report) const override;

        bool publish(EScenePublicationMode publicationMode = EScenePublicationMode::LocalAndRemote);
        bool unpublish();
        bool isPublished() const;
        sceneId_t getSceneId() const;
        EScenePublicationMode getPublicationModeSetFromSceneConfig() const;

        bool serialize(std::vector<std::byte>& outputBuffer, const SaveFileConfigImpl& config) const;
        bool saveToFile(std::string_view fileName, const SaveFileConfigImpl& config) const;

        LogicEngine* createLogicEngine(std::string_view name);

        PerspectiveCamera*  createPerspectiveCamera(std::string_view name);
        OrthographicCamera* createOrthographicCamera(std::string_view name);
        Appearance*         createAppearance(const Effect& effect, std::string_view name);
        Geometry*    createGeometry(const Effect& effect, std::string_view name);
        Node*               createNode(std::string_view name);
        MeshNode*           createMeshNode(std::string_view name);
        ramses::RenderGroup*  createRenderGroup(std::string_view name);
        ramses::RenderPass*   createRenderPass(std::string_view name);
        ramses::BlitPass*     createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, std::string_view name);
        ramses::PickableObject* createPickableObject(const ArrayBuffer& geometryBuffer, const pickableObjectId_t id, std::string_view name);
        ramses::RenderBuffer* createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, std::string_view name);
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

        DataObject* createDataObject(ramses::EDataType dataType, std::string_view name);

        bool createTransformationDataProvider(const Node& node, dataProviderId_t id);
        bool createTransformationDataConsumer(const Node& node, dataConsumerId_t id);
        bool createDataProvider(const DataObject& dataObject, dataProviderId_t id);
        bool createDataConsumer(const DataObject& dataObject, dataConsumerId_t id);
        bool createTextureProvider(const Texture2D& texture, dataProviderId_t id);
        bool updateTextureProvider(const Texture2D& texture, dataProviderId_t id);
        bool createTextureConsumer(const ramses::TextureSampler& sampler, dataConsumerId_t id);
        bool createTextureConsumer(const TextureSamplerMS& sampler, dataConsumerId_t id);
        bool createTextureConsumer(const TextureSamplerExternal& sampler, dataConsumerId_t id);

        ArrayBuffer* createArrayBuffer(ramses::EDataType dataType, uint32_t maxNumElements, std::string_view name);
        Texture2DBuffer* createTexture2DBuffer (size_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, std::string_view name);

        SceneReference* createSceneReference(sceneId_t referencedScene, std::string_view name);
        bool linkData(SceneReference* providerReference, dataProviderId_t providerId, SceneReference* consumerReference, dataConsumerId_t consumerId);
        bool unlinkData(SceneReference* consumerReference, dataConsumerId_t consumerId);

        bool destroy(SceneObject& object);

        bool setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds);

        bool flush(sceneVersionTag_t sceneVersion);

        bool resetUniformTimeMs();
        int32_t getUniformTimeMs() const;

        template <typename T>
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        ramses::ArrayResource* createArrayResource(uint32_t numElements, const T* arrayData, std::string_view name);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        Texture2D* createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, size_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, std::string_view name);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        Texture3D* createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, size_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, std::string_view name);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        TextureCube* createTextureCube(uint32_t size, ETextureFormat format, size_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, std::string_view name);
        Effect* createEffect(const EffectDescription& effectDesc, std::string_view name);
        std::string getLastEffectErrorMessages() const;

        ramses::ArrayResource* createHLArrayResource(ramses::internal::ManagedResource const& resource, std::string_view name);
        Texture2D* createHLTexture2D(ramses::internal::ManagedResource const& resource, std::string_view name);
        Texture3D* createHLTexture3D(ramses::internal::ManagedResource const& resource, std::string_view name);
        TextureCube* createHLTextureCube(ramses::internal::ManagedResource const& resource, std::string_view name);
        Effect* createHLEffect(ramses::internal::ManagedResource const& resource, std::string_view name);

        const ramses::internal::ClientScene& getIScene() const;
        ramses::internal::ClientScene& getIScene();

        bool containsSceneObject(const SceneObjectImpl& object) const;
        template <typename T> [[nodiscard]] const T* findObjectByName(std::string_view name) const;
        template <typename T> [[nodiscard]] T* findObjectByName(std::string_view name);
        Resource* getResource(resourceId_t rid) const;

        const SceneObject* findObjectById(sceneObjectId_t id) const;
        SceneObject* findObjectById(sceneObjectId_t id);

        SceneObjectRegistry&       getObjectRegistry();
        const SceneObjectRegistry& getObjectRegistry() const;

        void setSceneVersionForNextFlush(sceneVersionTag_t sceneVersion);

        sceneObjectId_t getNextSceneObjectId();

        ramses::internal::StatisticCollectionScene& getStatisticCollection();
        SceneReference* getSceneReference(sceneId_t referencedSceneId);

        RamsesClient& getHlRamsesClient();

        template <typename T>
        void enqueueSceneCommand(T commands);

        Resource* scanForResourceWithHash(ramses::internal::ResourceContentHash hash) const;

        template <typename ObjectType, typename ObjectImplType>
        bool createAndDeserializeObjectImpls(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count);

        void setSceneFileHandle(ramses::internal::SceneFileHandle handle);
        void closeSceneFile();
        ramses::internal::SceneFileHandle getSceneFileHandle() const;

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
            ramses::internal::TextureSampler::ContentType contentType,
            ramses::internal::ResourceContentHash textureResourceHash,    // The sampler stores either a texture, or...
            ramses::internal::MemoryHandle contentHandle,                 // a render target's color buffer, or a texture buffer, or a stream texture
            std::string_view name /*= 0*/);

        template <typename SAMPLER>
        bool createTextureConsumerImpl(const SAMPLER& sampler, dataConsumerId_t id, bool checkDuplicate = true);

        RenderPass* createRenderPassInternal(std::string_view name);

        template <typename T, typename ImplT>
        T& registerCreatedResourceObject(std::unique_ptr<ImplT> resourceImpl);

        void removeAllDataSlotsForNode(const Node& node);

        template <typename OBJECT, typename CONTAINER>
        void removeObjectFromAllContainers(const OBJECT& object);

        template <typename SAMPLER>
        bool destroyTextureSampler(SAMPLER& sampler);

        void markAllChildrenDirty(Node& node);

        bool cameraIsAssignedToRenderPasses(const Camera& camera);

        bool destroyRenderTarget(RenderTarget& renderTarget);
        bool destroyCamera(Camera& camera);
        bool destroyRenderGroup(RenderGroup& group);
        bool destroyMeshNode(MeshNode& mesh);
        bool destroyNode(Node& node);
        bool destroyDataObject(DataObject& dataObject);
        bool destroyResource(Resource& resource);
        bool destroyObject(SceneObject& object);

        using NodeVisibilityPair = std::pair<NodeImpl*, EVisibilityMode>;
        using NodeVisibilityInfoVector = std::vector<NodeVisibilityPair>;

        void applyVisibilityToSubtree(NodeImpl& initialNode, EVisibilityMode initialVisibility);
        void prepareListOfDirtyNodesForHierarchicalVisibility(NodeVisibilityInfoVector& nodesToProcess);
        void applyHierarchicalVisibility();

        bool writeSceneObjectsToStream(ramses::internal::IOutputStream& outputStream, const SaveFileConfigImpl& saveConfig) const;

        bool removeResourceWithIdFromResources(resourceId_t const& id, Resource& resource);

        ramses::internal::ClientScene&          m_scene;
        ramses::internal::SceneCommandBuffer    m_commandBuffer;
        sceneVersionTag_t                       m_nextSceneVersion;
        sceneObjectId_t                         m_lastSceneObjectId;

        SceneObjectRegistry m_objectRegistry;
        std::unordered_multimap <resourceId_t, Resource*> m_resources;

        // This is essentially a local variable only used in the "applyVisibilityToSubtree" method.
        // This is for performance reasons, so we can re-use the same vector each time the method is called.
        NodeVisibilityInfoVector m_dataStackForSubTreeVisibilityApplying;
        EScenePublicationMode m_futurePublicationMode;

        ramses::internal::FlushTime::Clock::time_point m_expirationTimestamp{ ramses::internal::FlushTime::InvalidTimestamp };

        ramses::internal::HashMap<sceneId_t, SceneReference*> m_sceneReferences;

        RamsesClient& m_hlClient;

        std::string m_effectErrorMessages;

        ramses::internal::SceneFileHandle m_sceneFileHandle;

        bool m_sendEffectTimeSync = false;
    };

    // define here to allow inlining
    inline const ramses::internal::ClientScene& SceneImpl::getIScene() const
    {
        return m_scene;
    }

    inline ramses::internal::ClientScene& SceneImpl::getIScene()
    {
        return m_scene;
    }

    template <typename T> T* SceneImpl::findObjectByName(std::string_view name)
    {
        return m_objectRegistry.findObjectByName<T>(name);
    }

    template <typename T> const T* SceneImpl::findObjectByName(std::string_view name) const
    {
        return m_objectRegistry.findObjectByName<T>(name);
    }

    template <typename T>
    void SceneImpl::enqueueSceneCommand(T commands)
    {
        m_commandBuffer.enqueueCommand(std::move(commands));
    }
}
