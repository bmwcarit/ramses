//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneImpl.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EScenePublicationMode.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/SceneReference.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Resource.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-sdk-build-config.h"
#include "ramses-utils.h"

#include "Scene/Scene.h"
#include "CameraNodeImpl.h"
#include "EffectImpl.h"
#include "NodeImpl.h"
#include "AppearanceImpl.h"
#include "GeometryBindingImpl.h"
#include "SerializationContext.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "RenderGroupImpl.h"
#include "RenderPassImpl.h"
#include "RenderBufferImpl.h"
#include "RenderTargetImpl.h"
#include "RenderTargetDescriptionImpl.h"
#include "TextureSamplerImpl.h"
#include "TextureUtils.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "MeshNodeImpl.h"
#include "EffectInputImpl.h"
#include "RamsesFrameworkImpl.h"
#include "Scene/EScenePublicationMode.h"
#include "RamsesClientImpl.h"
#include "SceneConfigImpl.h"
#include "Scene/ClientScene.h"
#include "SceneUtils.h"
#include "DataObjectImpl.h"
#include "BlitPassImpl.h"
#include "ClientCommands/SceneCommandBuffer.h"
#include "ClientCommands/SceneCommandVisitor.h"
#include "RamsesObjectRegistryIterator.h"
#include "SerializationHelper.h"
#include "ArrayBufferImpl.h"
#include "Texture2DBufferImpl.h"
#include "DataSlotUtils.h"
#include "PickableObjectImpl.h"
#include "SceneReferenceImpl.h"
#include "DataTypeUtils.h"
#include "RamsesVersion.h"
#include "Scene/ScenePersistation.h"
#include "AppearanceUtils.h"

#include "Resource/ArrayResource.h"
#include "Resource/TextureResource.h"
#include "Resource/EffectResource.h"

#include "Components/FlushTimeInformation.h"
#include "Components/EffectUniformTime.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/TextureMathUtils.h"
#include "Components/FlushTimeInformation.h"
#include "fmt/format.h"

#include <array>

namespace ramses
{
    SceneImpl::SceneImpl(ramses_internal::ClientScene& scene, const SceneConfigImpl& sceneConfig, RamsesClient& ramsesClient)
        : ClientObjectImpl(ramsesClient.m_impl, ERamsesObjectType::Scene, scene.getName().c_str())
        , m_scene(scene)
        , m_nextSceneVersion(InvalidSceneVersionTag)
        , m_futurePublicationMode(sceneConfig.getPublicationMode())
        , m_hlClient(ramsesClient)
    {
        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Scene::Scene: sceneId " << scene.getSceneId()  <<
                 ", publicationMode " << (sceneConfig.getPublicationMode() == EScenePublicationMode::LocalAndRemote ? "LocalAndRemote" : "LocalOnly"));
        getClientImpl().getFramework().getPeriodicLogger().registerStatisticCollectionScene(m_scene.getSceneId(), m_scene.getStatisticCollection());
        const bool enableLocalOnlyOptimization = sceneConfig.getPublicationMode() == EScenePublicationMode::LocalOnly;
        getClientImpl().getClientApplication().createScene(scene, enableLocalOnlyOptimization);
    }

    SceneImpl::~SceneImpl()
    {
        LOG_INFO(CONTEXT_CLIENT, "SceneImpl::~SceneImpl");
        closeSceneFile();
        getClientImpl().getFramework().getPeriodicLogger().removeStatisticCollectionScene(m_scene.getSceneId());
    }

    void SceneImpl::initializeFrameworkData()
    {
    }

    void SceneImpl::deinitializeFrameworkData()
    {
    }

    status_t SceneImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(ClientObjectImpl::serialize(outStream, serializationContext));

        outStream << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(m_expirationTimestamp.time_since_epoch()).count());

        CHECK_RETURN_ERR(SerializationHelper::SerializeObjectsInRegistry<SceneObject>(outStream, serializationContext, m_objectRegistry));

        outStream << m_lastSceneObjectId.getValue();

        return StatusOK;
    }

    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType)
    {
        return std::make_unique<T>(scene, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, ERamsesObjectType, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType type)
    {
        return std::make_unique<T>(scene, type, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, ramses_internal::ResourceHashUsage, SceneImpl&, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType)
    {
        return std::make_unique<T>(ramses_internal::ResourceHashUsage{}, scene, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, ERamsesObjectType, EDataType, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType type)
    {
        return std::make_unique<T>(scene, type, EDataType::Int32, "");
    }

    template <typename ObjectType, typename ObjectImplType>
    status_t SceneImpl::createAndDeserializeObjectImpls(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count)
    {
        for (uint32_t i = 0u; i < count; ++i)
        {
            auto impl = createImplHelper<ObjectImplType>(*this, TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID);
            ObjectIDType objectID = SerializationHelper::DeserializeObjectID(inStream);
            auto status = impl->deserialize(inStream, serializationContext);
            if (status != StatusOK)
                return status;

            serializationContext.registerObjectImpl(impl.get(), objectID);

            auto& object = m_objectRegistry.createAndRegisterObject<ObjectType>(std::move(impl));

            if constexpr (std::is_base_of_v<Resource, ObjectType>)
                m_resources.insert({ object.getResourceId(), &object });
        }

        return StatusOK;
    }

    status_t SceneImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(ClientObjectImpl::deserialize(inStream, serializationContext));

        uint64_t expirationTS = 0u;
        inStream >> expirationTS;
        m_expirationTimestamp = ramses_internal::FlushTime::Clock::time_point(std::chrono::milliseconds(expirationTS));

        uint32_t totalCount = 0u;
        uint32_t typesCount = 0u;
        SerializationHelper::DeserializeNumberOfObjectTypes(inStream, totalCount, typesCount);
        serializationContext.resize(totalCount, m_scene.getNodeCount());
        m_objectRegistry.reserveAdditionalGeneralCapacity(totalCount);

        std::array<uint32_t, static_cast<size_t>(ERamsesObjectType::NUMBER_OF_TYPES)> objectCounts = {};

        for (uint32_t i = 0u; i < typesCount; ++i)
        {
            uint32_t count = 0u;
            const ERamsesObjectType type = SerializationHelper::DeserializeObjectTypeAndCount(inStream, count);
            assert(m_objectRegistry.getNumberOfObjects(type) == 0u);
            m_objectRegistry.reserveAdditionalObjectCapacity(type, count);
            objectCounts[i] = count;

            status_t status = StatusOK;
            switch (type)
            {
            case ERamsesObjectType::Node:
                status = createAndDeserializeObjectImpls<Node, NodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::MeshNode:
                status = createAndDeserializeObjectImpls<MeshNode, MeshNodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::PerspectiveCamera:
                status = createAndDeserializeObjectImpls<PerspectiveCamera, CameraNodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::OrthographicCamera:
                status = createAndDeserializeObjectImpls<OrthographicCamera, CameraNodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::Appearance:
                status = createAndDeserializeObjectImpls<Appearance, AppearanceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::GeometryBinding:
                status = createAndDeserializeObjectImpls<GeometryBinding, GeometryBindingImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderGroup:
                status = createAndDeserializeObjectImpls<RenderGroup, RenderGroupImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderPass:
                status = createAndDeserializeObjectImpls<RenderPass, RenderPassImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::BlitPass:
                status = createAndDeserializeObjectImpls<BlitPass, BlitPassImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::PickableObject:
                status = createAndDeserializeObjectImpls<PickableObject, PickableObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::SceneReference:
                status = createAndDeserializeObjectImpls<SceneReference, SceneReferenceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderBuffer:
                status = createAndDeserializeObjectImpls<RenderBuffer, RenderBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderTarget:
                status = createAndDeserializeObjectImpls<RenderTarget, RenderTargetImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::TextureSampler:
                status = createAndDeserializeObjectImpls<TextureSampler, TextureSamplerImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::TextureSamplerMS:
                status = createAndDeserializeObjectImpls<TextureSamplerMS, TextureSamplerImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::TextureSamplerExternal:
                status = createAndDeserializeObjectImpls<TextureSamplerExternal, TextureSamplerImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::DataObject:
                status = createAndDeserializeObjectImpls<DataObject, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::ArrayBufferObject:
                status = createAndDeserializeObjectImpls<ArrayBuffer, ArrayBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::Texture2DBuffer:
                status = createAndDeserializeObjectImpls<Texture2DBuffer, Texture2DBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::ArrayResource:
                status = createAndDeserializeObjectImpls<ArrayResource, ArrayResourceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::Texture2D:
                status = createAndDeserializeObjectImpls<Texture2D, Texture2DImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::Texture3D:
                status = createAndDeserializeObjectImpls<Texture3D, Texture3DImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::TextureCube:
                status = createAndDeserializeObjectImpls<TextureCube, TextureCubeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::Effect:
                status = createAndDeserializeObjectImpls<Effect, EffectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::NUMBER_OF_TYPES:
            case ERamsesObjectType::Invalid:
            case ERamsesObjectType::ClientObject:
            case ERamsesObjectType::RamsesObject:
            case ERamsesObjectType::SceneObject:
            case ERamsesObjectType::Resource:
            case ERamsesObjectType::Camera:
            case ERamsesObjectType::Client:
            case ERamsesObjectType::Scene:
                return addErrorEntry("Scene::deserialize failed, unexpected object type in file stream.");
            }

            CHECK_RETURN_ERR(status);
        }

        inStream >> m_lastSceneObjectId.getReference();

        LOG_DEBUG_F(ramses_internal::CONTEXT_PROFILING, ([&](ramses_internal::StringOutputStream& sos) {
                    sos << "SceneImpl::deserialize: HL scene object counts for SceneID " << m_scene.getSceneId() << "\n";
                    for (uint32_t i = 0; i < objectCounts.size(); i++)
                    {
                        if (objectCounts[i] > 0)
                        {
                            sos << "  " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(static_cast<ERamsesObjectType>(i)) << " count: " << objectCounts[i] << "\n";
                        }
                    }
                }));

        CHECK_RETURN_ERR(serializationContext.resolveDependencies());

        return StatusOK;
    }

    status_t SceneImpl::validate() const
    {
        status_t status = ClientObjectImpl::validate();

        std::array<uint32_t, static_cast<size_t>(ERamsesObjectType::NUMBER_OF_TYPES)> objectCount;
        for (uint32_t i = 0u; i < objectCount.size(); ++i)
        {
            const ERamsesObjectType type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType::SceneObject)
                && RamsesObjectTypeUtils::IsConcreteType(type))
            {
                objectCount[i] = 0u;
                RamsesObjectRegistryIterator iter(getObjectRegistry(), ERamsesObjectType(i));
                while (const RamsesObject* obj = iter.getNext())
                {
                    status = std::max(status, addValidationOfDependentObject(obj->m_impl));
                    ++objectCount[i];
                }
            }
        }

        for (uint32_t i = 0u; i < objectCount.size(); ++i)
        {
            const ERamsesObjectType type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType::SceneObject)
                && RamsesObjectTypeUtils::IsConcreteType(type))
            {
                ramses_internal::StringOutputStream msg;
                msg << "Number of " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(type) << " instances: " << objectCount[i];
                addValidationMessage(EValidationSeverity::Info, msg.c_str());
            }
        }

        // special validation (see SceneImpl::createTextureConsumer(const TextureSamplerExternal&, dataConsumerId_t)),
        // duplicate IDs are temporarily allowed but validation still reports them as errors
        // TODO vaclav do this properly
        std::unordered_set<ramses_internal::DataSlotId> texConsumerIds;
        for (const auto& it : m_scene.getDataSlots())
        {
            if (it.second->type == ramses_internal::EDataSlotType_TextureConsumer)
            {
                const auto consumerId = it.second->id;
                if (texConsumerIds.count(consumerId) > 0u)
                    status = addValidationMessage(EValidationSeverity::Error,
                        fmt::format("Duplicate texture consumer ID '{}' is not allowed and will result in unknown behavior when linking on renderer", consumerId.getValue()));
                texConsumerIds.insert(consumerId);
            }
        }

        return status;
    }

    PerspectiveCamera* SceneImpl::createPerspectiveCamera(std::string_view name)
    {
        auto pimpl = std::make_unique<CameraNodeImpl>(*this, ERamsesObjectType::PerspectiveCamera, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<PerspectiveCamera>(std::move(pimpl));
    }

    OrthographicCamera* SceneImpl::createOrthographicCamera(std::string_view name)
    {
        auto pimpl = std::make_unique<CameraNodeImpl>(*this, ERamsesObjectType::OrthographicCamera, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<OrthographicCamera>(std::move(pimpl));
    }

    Appearance* SceneImpl::createAppearance(const Effect& effect, std::string_view name)
    {
        if (this != &effect.m_impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createAppearance failed, effect is not from this scene.");
            return nullptr;
        }

        auto pimpl = std::make_unique<AppearanceImpl>(*this, name);
        pimpl->initializeFrameworkData(effect.m_impl);

        return &m_objectRegistry.createAndRegisterObject<Appearance>(std::move(pimpl));
    }

    GeometryBinding* SceneImpl::createGeometryBinding(const Effect& effect, std::string_view name)
    {
        if (this != &effect.m_impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createGeometryBinding failed, effect is not from this scene.");
            return nullptr;
        }

        auto pimpl = std::make_unique<GeometryBindingImpl>(*this, name);
        pimpl->initializeFrameworkData(effect.m_impl);

        return &m_objectRegistry.createAndRegisterObject<GeometryBinding>(std::move(pimpl));
    }

    status_t SceneImpl::destroy(SceneObject& object)
    {
        if (!containsSceneObject(object.m_impl))
            return addErrorEntry("Scene::destroy failed, object is not in this scene.");

        status_t returnStatus = StatusOK;
        switch (object.getType())
        {
        case ERamsesObjectType::RenderTarget:
            returnStatus = destroyRenderTarget(RamsesObjectTypeUtils::ConvertTo<RenderTarget>(object));
            break;
        case ERamsesObjectType::OrthographicCamera:
        case ERamsesObjectType::PerspectiveCamera:
            returnStatus = destroyCamera(RamsesObjectTypeUtils::ConvertTo<Camera>(object));
            break;
        case ERamsesObjectType::RenderGroup:
            returnStatus = destroyRenderGroup(RamsesObjectTypeUtils::ConvertTo<RenderGroup>(object));
            break;
        case ERamsesObjectType::Node:
        case ERamsesObjectType::PickableObject:
            returnStatus = destroyNode(RamsesObjectTypeUtils::ConvertTo<Node>(object));
            break;
        case ERamsesObjectType::MeshNode:
            returnStatus = destroyMeshNode(RamsesObjectTypeUtils::ConvertTo<MeshNode>(object));
            break;
        case ERamsesObjectType::DataObject:
            returnStatus = destroyDataObject(RamsesObjectTypeUtils::ConvertTo<DataObject>(object));
            break;
        case ERamsesObjectType::TextureSampler:
            destroyTextureSampler(RamsesObjectTypeUtils::ConvertTo<TextureSampler>(object));
            break;
        case ERamsesObjectType::TextureSamplerMS:
            destroyTextureSampler(RamsesObjectTypeUtils::ConvertTo<TextureSamplerMS>(object));
            break;
        case ERamsesObjectType::TextureSamplerExternal:
            destroyTextureSampler(RamsesObjectTypeUtils::ConvertTo<TextureSamplerExternal>(object));
            break;
        case ERamsesObjectType::Appearance:
        case ERamsesObjectType::GeometryBinding:
        case ERamsesObjectType::RenderPass:
        case ERamsesObjectType::BlitPass:
        case ERamsesObjectType::RenderBuffer:
        case ERamsesObjectType::ArrayBufferObject:
        case ERamsesObjectType::Texture2DBuffer:
            returnStatus = destroyObject(object);
            break;
        case ERamsesObjectType::SceneReference: {
            auto& sceneReference = RamsesObjectTypeUtils::ConvertTo<SceneReference>(object);
            LOG_INFO_P(ramses_internal::CONTEXT_CLIENT, "Scene::destroySceneReference: (master {} / ref {})", getSceneId(), sceneReference.getReferencedSceneId());
            m_sceneReferences.remove(sceneReference.getReferencedSceneId());
            returnStatus = destroyObject(object);
            break;
        }
        case ERamsesObjectType::Texture2D:
        case ERamsesObjectType::Texture3D:
        case ERamsesObjectType::TextureCube:
        case ERamsesObjectType::Effect:
        case ERamsesObjectType::ArrayResource:
            returnStatus = destroyResource(RamsesObjectTypeUtils::ConvertTo<Resource>(object));
            break;
        case ERamsesObjectType::Invalid:
        case ERamsesObjectType::NUMBER_OF_TYPES:
        case ERamsesObjectType::ClientObject:
        case ERamsesObjectType::RamsesObject:
        case ERamsesObjectType::SceneObject:
        case ERamsesObjectType::Resource:
        case ERamsesObjectType::Camera:
        case ERamsesObjectType::Client:
        case ERamsesObjectType::Scene:
            assert(false);
            returnStatus = addErrorEntry("Scene::destroy internal error, cannot destroy object!");
            break;
        }

        return returnStatus;
    }

    status_t SceneImpl::destroyRenderTarget(RenderTarget& renderTarget)
    {
        RamsesObjectRegistryIterator iterator(m_objectRegistry, ERamsesObjectType::RenderPass);
        const RenderPass* renderPass = nullptr;
        while ((renderPass = iterator.getNext<RenderPass>()) != nullptr)
        {
            if (&renderTarget == renderPass->m_impl.getRenderTarget())
            {
                return addErrorEntry("Scene::destroy can not destroy render target while it is still assigned to a render pass!");
            }
        }
        return destroyObject(renderTarget);
    }

    status_t SceneImpl::destroyCamera(Camera& camera)
    {
        if (cameraIsAssignedToRenderPasses(camera))
        {
            return addErrorEntry("Scene::destroy can not destroy camera while it is still assigned to a render pass!");
        }

        return destroyNode(camera);
    }

    status_t SceneImpl::destroyRenderGroup(RenderGroup& group)
    {
        removeObjectFromAllContainers<RenderGroup, RenderPass>(group);
        removeObjectFromAllContainers<RenderGroup, RenderGroup>(group);

        return destroyObject(group);
    }

    status_t SceneImpl::destroyMeshNode(MeshNode& mesh)
    {
        removeObjectFromAllContainers<MeshNode, RenderGroup>(mesh);
        return destroyNode(mesh);
    }

    void SceneImpl::markAllChildrenDirty(Node& node)
    {
        for (uint32_t i = 0u; i < node.getChildCount(); ++i)
        {
            m_objectRegistry.setNodeDirty(node.m_impl.getChildImpl(i), true);
        }
    }

    status_t SceneImpl::destroyNode(Node& node)
    {
        markAllChildrenDirty(node);

        // with this also all pointers from children to this parent are deleted
        node.removeAllChildren();

        // with this also the pointer from parent to this child is deleted
        if (node.hasParent())
        {
            node.removeParent();
        }

        // with this also the attached transformation data slots for this node are deleted
        removeAllDataSlotsForNode(node);

        return destroyObject(node);
    }

    status_t SceneImpl::destroyDataObject(DataObject& dataObject)
    {
        const ramses_internal::DataInstanceHandle dataRef = dataObject.m_impl.getDataReference();
        const uint32_t slotHandleCount = m_scene.getDataSlotCount();
        for (ramses_internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) &&
                m_scene.getDataSlot(slotHandle).attachedDataReference == dataRef)
            {
                m_scene.releaseDataSlot(slotHandle);
            }
        }

        return destroyObject(dataObject);
    }

    template <typename SAMPLER>
    status_t SceneImpl::destroyTextureSampler(SAMPLER& sampler)
    {
        const ramses_internal::TextureSamplerHandle& samplerHandle = sampler.m_impl.getTextureSamplerHandle();
        const uint32_t slotHandleCount = m_scene.getDataSlotCount();
        for (ramses_internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) &&
                m_scene.getDataSlot(slotHandle).attachedTextureSampler == samplerHandle)
            {
                m_scene.releaseDataSlot(slotHandle);
            }
        }

        return destroyObject(sampler);
    }

    status_t SceneImpl::destroyResource(Resource& resource)
    {
        const resourceId_t resId = resource.m_impl.getResourceId();
        const bool found = removeResourceWithIdFromResources(resId, resource);
        if (!found)
            assert(false);

        return destroyObject(resource);
    }

    status_t SceneImpl::destroyObject(SceneObject& object)
    {
        object.m_impl.deinitializeFrameworkData();
        m_objectRegistry.destroyAndUnregisterObject(object);
        return StatusOK;
    }

    status_t SceneImpl::publish(EScenePublicationMode requestedPublicationMode)
    {
        if (isPublished())
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::publish: ignored, scene is already published").c_str());
        }
        if (requestedPublicationMode == EScenePublicationMode::LocalAndRemote && m_futurePublicationMode == EScenePublicationMode::LocalOnly)
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::publish: Enabled local only optimisations from SceneConfig, cannot remote publish later").c_str());
        }
        if (requestedPublicationMode != EScenePublicationMode::LocalOnly && !getClientImpl().getFramework().isConnected())
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::publish(LocalAndRemote): Scene is only published locally until framework is connected (RamsesFramework::connect)");
        }
        const ramses_internal::EScenePublicationMode internalMode = SceneUtils::GetScenePublicationModeInternal(requestedPublicationMode);
        getClientImpl().getClientApplication().publishScene(m_scene.getSceneId(), internalMode);
        return StatusOK;
    }

    status_t SceneImpl::unpublish()
    {
        if (!isPublished())
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::unpublish ignored, scene is not published.").c_str());
        }

        getClientImpl().getClientApplication().unpublishScene(m_scene.getSceneId());
        return StatusOK;
    }

    bool SceneImpl::isPublished() const
    {
        return getClientImpl().getClientApplication().isScenePublished(m_scene.getSceneId());
    }

    sceneId_t SceneImpl::getSceneId() const
    {
        return sceneId_t(m_scene.getSceneId().getValue());
    }

    EScenePublicationMode SceneImpl::getPublicationModeSetFromSceneConfig() const
    {
        return m_futurePublicationMode;
    }

    Node* SceneImpl::createNode(std::string_view name)
    {
        auto pimpl = std::make_unique<NodeImpl>(*this, ERamsesObjectType::Node, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<Node>(std::move(pimpl));
    }

    MeshNode* SceneImpl::createMeshNode(std::string_view name)
    {
        auto pimpl = std::make_unique<MeshNodeImpl>(*this, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<MeshNode>(std::move(pimpl));
    }

    RenderGroup* SceneImpl::createRenderGroup(std::string_view name)
    {
        auto pimpl = std::make_unique<RenderGroupImpl>(*this, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<RenderGroup>(std::move(pimpl));
    }

    RenderPass* SceneImpl::createRenderPass(std::string_view name /*= {}*/)
    {
        return createRenderPassInternal(name);
    }

    BlitPass* SceneImpl::createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, std::string_view name)
    {
        if (!containsSceneObject(sourceRenderBuffer.m_impl))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createBlitPass failed, source render buffer is not from this scene.");
            return nullptr;
        }

        if (!containsSceneObject(destinationRenderBuffer.m_impl))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createBlitPass failed, destination render buffer is not from this scene.");
            return nullptr;
        }

        if (sourceRenderBuffer.getBufferType() != destinationRenderBuffer.getBufferType())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createBlitPass failed, source and destination buffers have different buffer types");
            return nullptr;
        }

        if (sourceRenderBuffer.getBufferFormat() != destinationRenderBuffer.getBufferFormat())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createBlitPass failed, source and destination buffers have different buffer formats");
            return nullptr;
        }

        if (sourceRenderBuffer.getHeight() != destinationRenderBuffer.getHeight() ||
            sourceRenderBuffer.getWidth() != destinationRenderBuffer.getWidth())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createBlitPass failed, source and destination buffers have different dimensions");
            return nullptr;
        }

        if (&sourceRenderBuffer == &destinationRenderBuffer)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createBlitPass failed, source and destination cannot be the same buffer");
            return nullptr;
        }

        auto pimpl = std::make_unique<BlitPassImpl>(*this, name);
        pimpl->initializeFrameworkData(sourceRenderBuffer.m_impl, destinationRenderBuffer.m_impl);

        return &m_objectRegistry.createAndRegisterObject<BlitPass>(std::move(pimpl));
    }

    PickableObject* SceneImpl::createPickableObject(const ArrayBuffer& geometryBuffer, const pickableObjectId_t id, std::string_view name)
    {
        if (!containsSceneObject(geometryBuffer.m_impl))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                      "Scene(" << m_scene.getSceneId()
                               << ")::createPickableObject failed, geometry buffer is not from this scene.");
            return nullptr;
        }

        if (geometryBuffer.getDataType() != EDataType::Vector3F || 0 != (geometryBuffer.m_impl.getElementCount() % 3))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                      "Scene(" << m_scene.getSceneId()
                               << ")::createPickableObject failed, geometry buffer has the wrong format.");
            return nullptr;
        }

        auto pimpl = std::make_unique<PickableObjectImpl>(*this, name);
        pimpl->initializeFrameworkData(geometryBuffer.m_impl, id);

        return &m_objectRegistry.createAndRegisterObject<PickableObject>(std::move(pimpl));
    }

    RenderBuffer* SceneImpl::createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, std::string_view name)
    {
        if (0 == width || 0 == height)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createRenderBuffer failed: cannot create a render buffer with 0 width and/or height!");
            return nullptr;
        }

        if (!TextureUtils::IsRenderBufferTypeCompatibleWithFormat(bufferType, bufferFormat))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createRenderBuffer failed: render buffer format incompatible with its type!");
            return nullptr;
        }

        auto pimpl = std::make_unique<RenderBufferImpl>(*this, name);
        pimpl->initializeFrameworkData(width, height, bufferType, bufferFormat, accessMode, sampleCount);

        return &m_objectRegistry.createAndRegisterObject<RenderBuffer>(std::move(pimpl));
    }

    RenderTarget* SceneImpl::createRenderTarget(const RenderTargetDescriptionImpl& rtDesc, std::string_view name)
    {
        if (rtDesc.validate() != StatusOK)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createRenderTarget failed, RenderTargetDescription is invalid.");
            return nullptr;
        }

        auto pimpl = std::make_unique<RenderTargetImpl>(*this, name);
        pimpl->initializeFrameworkData(rtDesc);

        return &m_objectRegistry.createAndRegisterObject<RenderTarget>(std::move(pimpl));
    }

    TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        const Texture2D& texture,
        std::string_view name /*= {}*/)
    {
        if (this != &texture.m_impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture 2D is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode::Clamp,
            minSamplingMethod,
            magSamplingMethod,
            anisotropyLevel,
            ERamsesObjectType::Texture2D,
            ramses_internal::TextureSampler::ContentType::ClientTexture,
            texture.m_impl.getLowlevelResourceHash(),
            ramses_internal::InvalidMemoryHandle,
            name);
    }

    TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureAddressMode wrapRMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const Texture3D& texture,
        std::string_view name /*= {}*/)
    {
        if (this != &texture.m_impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture 3D is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, wrapRMode, minSamplingMethod, magSamplingMethod, 1u,
            ERamsesObjectType::Texture3D,
            ramses_internal::TextureSampler::ContentType::ClientTexture,
            texture.m_impl.getLowlevelResourceHash(),
            ramses_internal::InvalidMemoryHandle,
            name);
    }

    TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        const TextureCube& texture,
        std::string_view name /*= {}*/)
    {
        if (this != &texture.m_impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture Cube is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode::Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType::TextureCube,
            ramses_internal::TextureSampler::ContentType::ClientTexture,
            texture.m_impl.getLowlevelResourceHash(),
            ramses_internal::InvalidMemoryHandle,
            name);
    }

    TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod ,
        uint32_t anisotropyLevel,
        const RenderBuffer& renderBuffer,
        std::string_view name)
    {
        if (renderBuffer.getSampleCount() > 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, cannot create this type of TextureSampler with multisampled RenderBuffer.");
            return nullptr;
        }

        if (!containsSceneObject(renderBuffer.m_impl))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer is not from this scene.");
            return nullptr;
        }

        if (ERenderBufferAccessMode::WriteOnly == renderBuffer.m_impl.getAccessMode())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer has access mode write only.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode::Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType::RenderBuffer,
            ramses_internal::TextureSampler::ContentType::RenderBuffer,
            ramses_internal::ResourceContentHash::Invalid(),
            renderBuffer.m_impl.getRenderBufferHandle().asMemoryHandle(),
            name);
    }

    ramses::TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        const Texture2DBuffer& textureBuffer,
        std::string_view name)
    {
        if (!containsSceneObject(textureBuffer.m_impl))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture2D buffer is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode::Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType::Texture2DBuffer,
            ramses_internal::TextureSampler::ContentType::TextureBuffer,
            ramses_internal::ResourceContentHash::Invalid(),
            textureBuffer.m_impl.getTextureBufferHandle().asMemoryHandle(),
            name);
    }

    ramses::TextureSamplerMS* SceneImpl::createTextureSamplerMS(const RenderBuffer& renderBuffer, std::string_view name)
    {
        if (!containsSceneObject(renderBuffer.m_impl))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer is not from this scene.");
            return nullptr;
        }

        if (ERenderBufferAccessMode::WriteOnly == renderBuffer.m_impl.getAccessMode())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer has access mode write only.");
            return nullptr;
        }

        auto samplerImpl = std::make_unique<TextureSamplerImpl>(*this, ERamsesObjectType::TextureSamplerMS, name);
        samplerImpl->initializeFrameworkData(
            {},
            ERamsesObjectType::RenderBuffer,
            ramses_internal::TextureSampler::ContentType::RenderBufferMS,
            ramses_internal::ResourceContentHash::Invalid(),
            renderBuffer.m_impl.getRenderBufferHandle().asMemoryHandle());

        return &m_objectRegistry.createAndRegisterObject<TextureSamplerMS>(std::move(samplerImpl));
    }

    ramses::TextureSamplerExternal* SceneImpl::createTextureSamplerExternal(ETextureSamplingMethod minSamplingMethod, ETextureSamplingMethod magSamplingMethod, std::string_view name)
    {
        if (ETextureSamplingMethod::Nearest != magSamplingMethod && ETextureSamplingMethod::Linear != magSamplingMethod)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSamplerExternal failed, mag sampling method must be set to Nearest or Linear.");
            return nullptr;
        }

        //Restrictions in spec section 3.7.14, https://registry.khronos.org/OpenGL/extensions/OES/OES_EGL_image_external.txt
        //According to spec min filtering can only be linear or nearest
        if (ETextureSamplingMethod::Nearest != minSamplingMethod && ETextureSamplingMethod::Linear != minSamplingMethod)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSamplerExternal failed, min sampling method must be set to Nearest or Linear for external textures.");
            return nullptr;
        }

        //According to spec clamp to edge so the only allowed wrap mode
        constexpr ETextureAddressMode wrapUMode = ETextureAddressMode::Clamp;
        constexpr ETextureAddressMode wrapVMode = ETextureAddressMode::Clamp;

        ramses_internal::TextureSamplerStates samplerStates(
            TextureUtils::GetTextureAddressModeInternal(wrapUMode),
            TextureUtils::GetTextureAddressModeInternal(wrapVMode),
            ramses_internal::EWrapMethod::Clamp,
            TextureUtils::GetTextureSamplingInternal(minSamplingMethod),
            TextureUtils::GetTextureSamplingInternal(magSamplingMethod)
            );

        auto samplerImpl = std::make_unique<TextureSamplerImpl>(*this, ERamsesObjectType::TextureSamplerExternal, name);
        samplerImpl->initializeFrameworkData(
            samplerStates,
            ERamsesObjectType::TextureSamplerExternal,
            ramses_internal::TextureSampler::ContentType::ExternalTexture,
            ramses_internal::ResourceContentHash::Invalid(),
            ramses_internal::InvalidMemoryHandle);

        return &m_objectRegistry.createAndRegisterObject<TextureSamplerExternal>(std::move(samplerImpl));
    }

    ramses::TextureSampler* SceneImpl::createTextureSamplerImpl(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureAddressMode wrapRMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        ERamsesObjectType samplerType,
        ramses_internal::TextureSampler::ContentType contentType,
        ramses_internal::ResourceContentHash textureResourceHash,
        ramses_internal::MemoryHandle contentHandle,
        std::string_view name /*= {}*/)
    {
        if (ETextureSamplingMethod::Nearest != magSamplingMethod && ETextureSamplingMethod::Linear != magSamplingMethod)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, mag sampling method must be set to Nearest or Linear.");
            return nullptr;
        }

        if (anisotropyLevel < 1)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, anisotropyLevel must be at least 1.");
            return nullptr;
        }

        ramses_internal::TextureSamplerStates samplerStates(
            TextureUtils::GetTextureAddressModeInternal(wrapUMode),
            TextureUtils::GetTextureAddressModeInternal(wrapVMode),
            TextureUtils::GetTextureAddressModeInternal(wrapRMode),
            TextureUtils::GetTextureSamplingInternal(minSamplingMethod),
            TextureUtils::GetTextureSamplingInternal(magSamplingMethod),
            anisotropyLevel
            );

        auto samplerImpl = std::make_unique<TextureSamplerImpl>(*this, ERamsesObjectType::TextureSampler, name);
        samplerImpl->initializeFrameworkData(samplerStates, samplerType, contentType, textureResourceHash, contentHandle);

        return &m_objectRegistry.createAndRegisterObject<TextureSampler>(std::move(samplerImpl));
    }

    DataObject* SceneImpl::createDataObject(EDataType dataType, std::string_view name /*= {}*/)
    {
        if (!IsDataObjectDataType(dataType))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createDataObject data type is not supported, see IsDataObjectDataType.");
            return nullptr;
        }

        auto pimpl = std::make_unique<DataObjectImpl>(*this, ERamsesObjectType::DataObject, dataType, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<DataObject>(std::move(pimpl));
    }

    status_t SceneImpl::createTransformationDataProvider(const Node& node, dataProviderId_t id)
    {
        if (!containsSceneObject(node.m_impl))
        {
            return addErrorEntry("Scene::createTransformationDataProvider failed, node is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createTransformationDataProvider failed, duplicate data slot id");
        }

        const ramses_internal::NodeHandle nodeHandle = node.m_impl.getNodeHandle();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForNode(m_scene, nodeHandle))
        {
            return addErrorEntry("Scene::createTransformationDataProvider failed, Node already has a transformation data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationProvider, internalDataSlotId, nodeHandle, ramses_internal::DataInstanceHandle::Invalid(), ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createTransformationDataConsumer(const Node& node, dataConsumerId_t id)
    {
        if (!containsSceneObject(node.m_impl))
        {
            return addErrorEntry("Scene::createTransformationDataConsumer failed, Group Node is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createTransformationDataConsumer failed, duplicate data slot id");
        }

        const ramses_internal::NodeHandle nodeHandle = node.m_impl.getNodeHandle();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForNode(m_scene, nodeHandle))
        {
            return addErrorEntry("Scene::createTransformationDataConsumer failed, Node already has a transformation data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationConsumer, internalDataSlotId, nodeHandle, ramses_internal::DataInstanceHandle::Invalid(), ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createDataProvider(const DataObject& dataObject, dataProviderId_t id)
    {
        if (!containsSceneObject(dataObject.m_impl))
        {
            return addErrorEntry("Scene::createDataProvider failed, data object is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createDataProvider failed, duplicate data slot id");
        }

        const ramses_internal::DataInstanceHandle dataRef = dataObject.m_impl.getDataReference();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForDataObject(m_scene, dataRef))
        {
            return addErrorEntry("Scene::createDataProvider failed, data object already has a data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_DataProvider, internalDataSlotId, ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createDataConsumer(const DataObject& dataObject, dataConsumerId_t id)
    {
        if (!containsSceneObject(dataObject.m_impl))
        {
            return addErrorEntry("Scene::createDataConsumer failed, data object is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createDataConsumer failed, duplicate data slot id");
        }

        const ramses_internal::DataInstanceHandle dataRef = dataObject.m_impl.getDataReference();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForDataObject(m_scene, dataRef))
        {
            return addErrorEntry("Scene::createDataConsumer failed, data object already has a data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_DataConsumer, internalDataSlotId, ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createTextureProvider(const Texture2D& texture, dataProviderId_t id)
    {
        if (this != &texture.m_impl.getSceneImpl())
        {
            return addErrorEntry("Scene::createTextureProvider failed, texture is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createTextureProvider failed, duplicate data slot id");
        }

        const ramses_internal::ResourceContentHash& textureHash = texture.m_impl.getLowlevelResourceHash();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForTexture(m_scene, textureHash))
        {
            return addErrorEntry("Scene::createTextureProvider failed, texture already has a data slot assigned in this scene");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TextureProvider, internalDataSlotId, ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle::Invalid(), textureHash, ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::updateTextureProvider(const Texture2D& texture, dataProviderId_t id)
    {
        if (this != &texture.m_impl.getSceneImpl())
        {
            return addErrorEntry("Scene::updateTextureProvider failed, texture is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (!ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::updateTextureProvider failed, provider has not been created before.");
        }

        const ramses_internal::UInt32 slotHandleCount = m_scene.getDataSlotCount();
        for (ramses_internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) && m_scene.getDataSlot(slotHandle).id == internalDataSlotId)
            {
                const ramses_internal::ResourceContentHash& textureHash = texture.m_impl.getLowlevelResourceHash();
                if (m_scene.getDataSlot(slotHandle).attachedTexture != textureHash)
                {
                    m_scene.setDataSlotTexture(slotHandle, textureHash);
                    break;
                }
            }
        }

        return StatusOK;
    }

    status_t SceneImpl::createTextureConsumer(const TextureSampler& sampler, dataConsumerId_t id)
    {
        if (sampler.m_impl.getTextureType() != ERamsesObjectType::Texture2D)
        {
            return addErrorEntry("Scene::createTextureConsumer failed, only texture sampler using 2D texture can be used for linking..");
        }

        return createTextureConsumerImpl(sampler, id);
    }

    status_t SceneImpl::createTextureConsumer(const TextureSamplerMS& sampler, dataConsumerId_t id)
    {
        return createTextureConsumerImpl(sampler, id);
    }

    status_t SceneImpl::createTextureConsumer(const TextureSamplerExternal& sampler, dataConsumerId_t id)
    {
        // Allow duplicate consumer ID for external samplers (special need for ramses composer).
        // This will NOT work properly if consumers with same ID get to renderer side and attempt to be linked
        // (or if one is already linked and another one with same ID is created).
        // TODO vaclav make this proper, allow it in general and handle duplicates on renderer side

        return createTextureConsumerImpl(sampler, id, false);
    }

    template <typename SAMPLER>
    status_t SceneImpl::createTextureConsumerImpl(const SAMPLER& sampler, dataConsumerId_t id, bool checkDuplicate)
    {
        if (!containsSceneObject(sampler.m_impl))
        {
            return addErrorEntry("Scene::createTextureConsumer failed, texture sampler is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (checkDuplicate)
        {
            if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
                return addErrorEntry("Scene::createTextureConsumer failed, duplicate data slot id");
        }

        const ramses_internal::TextureSamplerHandle& samplerHandle = sampler.m_impl.getTextureSamplerHandle();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForTextureSampler(m_scene, samplerHandle))
        {
            return addErrorEntry("Scene::createTextureConsumer failed, texture sampler already has a data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TextureConsumer, internalDataSlotId, ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle::Invalid(), ramses_internal::ResourceContentHash::Invalid(), samplerHandle });
        return StatusOK;
    }

    status_t SceneImpl::setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds)
    {
        m_expirationTimestamp = ramses_internal::FlushTime::Clock::time_point(std::chrono::milliseconds{ ptpExpirationTimestampInMilliseconds });
        return StatusOK;
    }

    status_t SceneImpl::flush(sceneVersionTag_t sceneVersion)
    {
        const auto timestampOfFlushCall = m_sendEffectTimeSync ? getIScene().getEffectTimeSync() :  ramses_internal::FlushTime::Clock::now();

        LOG_DEBUG_P(CONTEXT_CLIENT, "Scene::flush: sceneVersion {}, prevSceneVersion {}, syncFlushTime {}", sceneVersion, m_nextSceneVersion, ramses_internal::asMilliseconds(timestampOfFlushCall));

        if (m_nextSceneVersion != InvalidSceneVersionTag && sceneVersion == InvalidSceneVersionTag)
        {
            sceneVersion = m_nextSceneVersion;
            m_nextSceneVersion = InvalidSceneVersionTag;
        }

        const ramses_internal::SceneVersionTag sceneVersionInternal(sceneVersion);

        m_commandBuffer.execute(ramses_internal::SceneCommandVisitor(*this));
        applyHierarchicalVisibility();

        const ramses_internal::FlushTimeInformation flushTimeInfo { m_expirationTimestamp, timestampOfFlushCall, ramses_internal::FlushTime::Clock::getClockType(), m_sendEffectTimeSync };
        m_sendEffectTimeSync          = false;

        if (!getClientImpl().getClientApplication().flush(m_scene.getSceneId(), flushTimeInfo, sceneVersionInternal))
            return addErrorEntry("Scene::flush: Flushing scene failed, consult logs for more details.");
        getStatisticCollection().statFlushesTriggered.incCounter(1);

        return StatusOK;
    }

    status_t SceneImpl::resetUniformTimeMs()
    {
        const auto now   = ramses_internal::FlushTime::Clock::now();
        const auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        LOG_INFO_P(CONTEXT_CLIENT, "Scene({})::resetUniformTimeMs: {}", getSceneId(), nowMs.time_since_epoch().count());
        m_sendEffectTimeSync = true;
        getIScene().setEffectTimeSync(now);
        return StatusOK;
    }

    int32_t SceneImpl::getUniformTimeMs() const
    {
        return ramses_internal::EffectUniformTime::GetMilliseconds(getIScene().getEffectTimeSync());
    }

    RamsesObjectRegistry& SceneImpl::getObjectRegistry()
    {
        return m_objectRegistry;
    }

    bool SceneImpl::containsSceneObject(const SceneObjectImpl& object) const
    {
        return &object.getSceneImpl() == this;
    }

    const RamsesObject* SceneImpl::findObjectByName(std::string_view name) const
    {
        return m_objectRegistry.findObjectByName(name);
    }

    RamsesObject* SceneImpl::findObjectByName(std::string_view name)
    {
        return m_objectRegistry.findObjectByName(name);
    }

    const SceneObject* SceneImpl::findObjectById(sceneObjectId_t id) const
    {
        return m_objectRegistry.findObjectById(id);
    }

    SceneObject* SceneImpl::findObjectById(sceneObjectId_t id)
    {
        return m_objectRegistry.findObjectById(id);
    }

    const RamsesObjectRegistry& SceneImpl::getObjectRegistry() const
    {
        return m_objectRegistry;
    }

    template <typename OBJECT, typename CONTAINER>
    void SceneImpl::removeObjectFromAllContainers(const OBJECT& object)
    {
        const ERamsesObjectType type = TYPE_ID_OF_RAMSES_OBJECT<CONTAINER>::ID;
        RamsesObjectRegistryIterator iterator(m_objectRegistry, type);
        CONTAINER* container = nullptr;
        while ((container = iterator.getNextNonConst<CONTAINER>()) != nullptr)
        {
            container->m_impl.removeIfContained(object.m_impl);
        }
    }

    void SceneImpl::removeAllDataSlotsForNode(const Node& node)
    {
        const ramses_internal::NodeHandle nodeHandle = node.m_impl.getNodeHandle();
        const ramses_internal::UInt32 slotHandleCount = m_scene.getDataSlotCount();
        for (ramses_internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) && m_scene.getDataSlot(slotHandle).attachedNode == nodeHandle)
            {
                m_scene.releaseDataSlot(slotHandle);
            }
        }
    }

    RenderPass* SceneImpl::createRenderPassInternal(std::string_view name)
    {
        auto pimpl = std::make_unique<RenderPassImpl>(*this, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<RenderPass>(std::move(pimpl));
    }

    bool SceneImpl::cameraIsAssignedToRenderPasses(const Camera& camera)
    {
        RamsesObjectRegistryIterator iterator(m_objectRegistry, ERamsesObjectType::RenderPass);
        const RenderPass* renderPass = nullptr;
        while ((renderPass = iterator.getNext<RenderPass>()) != nullptr)
        {
            if (renderPass->getCamera() == &camera)
            {
                return true;
            }
        }

        return false;
    }

    void SceneImpl::applyVisibilityToSubtree(NodeImpl& initialNode, EVisibilityMode initialVisibility)
    {
        assert(m_dataStackForSubTreeVisibilityApplying.empty());
        m_dataStackForSubTreeVisibilityApplying.emplace_back(&initialNode, initialVisibility);

        while (!m_dataStackForSubTreeVisibilityApplying.empty())
        {
            NodeVisibilityPair pair = m_dataStackForSubTreeVisibilityApplying.back();
            NodeImpl& node = *pair.first;
            EVisibilityMode visibilityToApply = pair.second;
            m_dataStackForSubTreeVisibilityApplying.pop_back();

            visibilityToApply = std::min(visibilityToApply, node.getVisibility());

            const ERamsesObjectType nodeType = node.getType();
            if (nodeType == ERamsesObjectType::MeshNode)
            {
                MeshNodeImpl& meshNode = static_cast<MeshNodeImpl&>(node);
                const EVisibilityMode currentVisibility = meshNode.getFlattenedVisibility();

                if (currentVisibility != visibilityToApply)
                {
                    meshNode.setFlattenedVisibility(visibilityToApply);
                }
            }

            const uint32_t numberOfChildren = node.getChildCount();
            for (uint32_t i = 0; i < numberOfChildren; i++)
            {
                NodeImpl& child = node.getChildImpl(i);
                m_dataStackForSubTreeVisibilityApplying.emplace_back(&child, visibilityToApply);
            }
        }

        assert(m_dataStackForSubTreeVisibilityApplying.empty());
    }

    void SceneImpl::prepareListOfDirtyNodesForHierarchicalVisibility(NodeVisibilityInfoVector& nodesToProcess)
    {
        const NodeImplSet& dirtyNodes = m_objectRegistry.getDirtyNodes();
        nodesToProcess.reserve(dirtyNodes.size());

        for (auto node : dirtyNodes)
        {
            assert(node != nullptr);

            // find a visibility state of the branch this dirty node is in
            // while doing that also check if any ancestor is already dirty,
            // in that case this node does not need processing
            bool needsToBeProcessed = true;
            EVisibilityMode parentVisibility = EVisibilityMode::Visible;
            NodeImpl* currParent = node->getParentImpl();
            while ((parentVisibility != EVisibilityMode::Off) && currParent && needsToBeProcessed)
            {
                parentVisibility = std::min(parentVisibility, currParent->getVisibility());
                needsToBeProcessed = !dirtyNodes.contains(currParent);
                currParent = currParent->getParentImpl();
            }

            if (needsToBeProcessed)
            {
                NodeVisibilityPair nodeInfo;
                nodeInfo.first = node;
                nodeInfo.second = parentVisibility;
                nodesToProcess.push_back(nodeInfo);
            }
        }
    }

    void SceneImpl::applyHierarchicalVisibility()
    {
        if (m_objectRegistry.getDirtyNodes().size() == 0u)
        {
            return;
        }

        NodeVisibilityInfoVector nodesToProcess;
        prepareListOfDirtyNodesForHierarchicalVisibility(nodesToProcess);

        for (auto& nodeInfo : nodesToProcess)
        {
            applyVisibilityToSubtree(*nodeInfo.first, nodeInfo.second);
        }

        m_objectRegistry.clearDirtyNodes();
    }

    void SceneImpl::setSceneVersionForNextFlush(sceneVersionTag_t sceneVersion)
    {
        assert(m_nextSceneVersion == InvalidSceneVersionTag);
        m_nextSceneVersion = sceneVersion;
    }

    ArrayBuffer* SceneImpl::createArrayBuffer(EDataType dataType, uint32_t maxNumElements, std::string_view name)
    {
        if (!IsArrayResourceDataType(dataType))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createArrayBuffer failed: incompatible data type");
            return nullptr;
        }

        auto pimpl = std::make_unique<ArrayBufferImpl>(*this, name);
        pimpl->initializeFrameworkData(dataType, maxNumElements);

        return &m_objectRegistry.createAndRegisterObject<ArrayBuffer>(std::move(pimpl));
    }

    Texture2DBuffer* SceneImpl::createTexture2DBuffer(uint32_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, std::string_view name)
    {
        if (IsFormatCompressed(TextureUtils::GetTextureFormatInternal(textureFormat)))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture2DBuffer failed, cannot create texture buffers with compressed texture format.");
            return nullptr;
        }

        // More than one mips have size 1x1 -> error
        const uint32_t maxMipCount = ramses_internal::TextureMathUtils::GetMipLevelCount(width, height, 1u);
        if (mipLevels > maxMipCount)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture2DBuffer failed, mipLevels too large for the provided texture size.");
            return nullptr;
        }

        ramses_internal::MipMapDimensions mipMapSizes;

        uint32_t currentWidth = width;
        uint32_t currentHeight = height;
        for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
        {
            mipMapSizes.push_back({ currentWidth, currentHeight });

            currentWidth = std::max<uint32_t>(1, currentWidth / 2);
            currentHeight = std::max<uint32_t>(1, currentHeight / 2);
        }

        auto pimpl = std::make_unique<Texture2DBufferImpl>(*this, name);
        pimpl->initializeFrameworkData(mipMapSizes, textureFormat);

        return &m_objectRegistry.createAndRegisterObject<Texture2DBuffer>(std::move(pimpl));
    }

    ramses_internal::StatisticCollectionScene& SceneImpl::getStatisticCollection()
    {
        return m_scene.getStatisticCollection();
    }

    ramses::SceneReference* SceneImpl::createSceneReference(sceneId_t referencedScene, std::string_view name)
    {
        if (!referencedScene.isValid())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene::createSceneReference: cannot reference a scene with invalid scene ID.");
            return nullptr;
        }

        if (referencedScene == getSceneId())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene::createSceneReference: cannot self reference.");
            return nullptr;
        }

        for (const auto& scene : getClientImpl().getListOfScenes())
        {
            if (getClientImpl().findSceneReference(scene->getSceneId(), referencedScene) != nullptr)
            {
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene::createSceneReference: there is already a SceneReference with sceneId "
                    << referencedScene << " in master scene " << scene->getSceneId() << ", cannot create another one");
                return nullptr;
            }
        }

        LOG_INFO_P(ramses_internal::CONTEXT_CLIENT, "Scene::createSceneReference: creating scene reference (master {} / ref {})", getSceneId(), referencedScene);

        auto pimpl = std::make_unique<SceneReferenceImpl>(*this, name);
        pimpl->initializeFrameworkData(referencedScene);
        auto& sr = m_objectRegistry.createAndRegisterObject<SceneReference>(std::move(pimpl));
        m_sceneReferences.put(referencedScene, &sr);

        return &sr;
    }

    status_t SceneImpl::linkData(SceneReference* providerReference, dataProviderId_t providerId, SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        if (!providerReference && !consumerReference)
            return addErrorEntry("Scene::linkData: can't link an object to another object in the same scene");

        if (consumerReference == providerReference)
            return addErrorEntry("Scene::linkData: can't link an object to another object in the same scene reference");

        if ((providerReference != nullptr && providerReference->m_impl.getSceneImpl().getSceneId() != getSceneId()) ||
            (consumerReference != nullptr && consumerReference->m_impl.getSceneImpl().getSceneId() != getSceneId()))
            return addErrorEntry("Scene::linkData: can't link to object of a scene reference with a different master scene");

        if (providerReference && providerReference->m_impl.getReportedState() < RendererSceneState::Ready)
            return addErrorEntry("Scene::linkData: Provider SceneReference state has to be at least Ready");

        if (consumerReference && consumerReference->m_impl.getReportedState() < RendererSceneState::Ready)
            return addErrorEntry("Scene::linkData: Consumer SceneReference state has to be at least Ready");

        const auto providerScene = (providerReference ? providerReference->m_impl.getSceneReferenceHandle() : ramses_internal::SceneReferenceHandle{});
        const auto consumerScene = (consumerReference ? consumerReference->m_impl.getSceneReferenceHandle() : ramses_internal::SceneReferenceHandle{});
        getIScene().linkData(providerScene, ramses_internal::DataSlotId{ providerId.getValue() }, consumerScene, ramses_internal::DataSlotId{ consumerId.getValue() });

        return StatusOK;
    }

    status_t SceneImpl::unlinkData(SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        if (consumerReference != nullptr && consumerReference->m_impl.getSceneImpl().getSceneId() != getSceneId())
            return addErrorEntry("Scene::unlinkData: can't unlink object of a scene reference with a different master scene");

        const auto consumerScene = (consumerReference ? consumerReference->m_impl.getSceneReferenceHandle() : ramses_internal::SceneReferenceHandle{});
        getIScene().unlinkData(consumerScene, ramses_internal::DataSlotId{ consumerId.getValue() });

        return StatusOK;
    }

    SceneReference* SceneImpl::getSceneReference(sceneId_t referencedSceneId)
    {
        auto it = m_sceneReferences.find(referencedSceneId);
        return (it != m_sceneReferences.end()) ? it->value : nullptr;
    }

    RamsesClient& SceneImpl::getHlRamsesClient()
    {
        return m_hlClient;
    }

    sceneObjectId_t SceneImpl::getNextSceneObjectId()
    {
        ++m_lastSceneObjectId.getReference();
        return sceneObjectId_t{ m_lastSceneObjectId};
    }

    template <typename T>
    ArrayResource* SceneImpl::createArrayResource(uint32_t numElements, const T* arrayData, resourceCacheFlag_t cacheFlag, std::string_view name)
    {
        if (0u == numElements || nullptr == arrayData)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene::createArrayResource: Array resource must have element count > 0 and data must not be nullptr!");
            return nullptr;
        }

        ramses_internal::ManagedResource res = getClientImpl().createManagedArrayResource(numElements, GetEDataType<T>(), arrayData, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createArrayResource: failed to create managed array resource");
            return nullptr;
        }
        return createHLArrayResource(res, name);
    }

    ArrayResource* SceneImpl::createHLArrayResource(ramses_internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_IndexArray ||
               resource->getTypeID() == ramses_internal::EResourceType_VertexArray);

        const auto arrayRes = resource->convertTo<ramses_internal::ArrayResource>();
        ramses_internal::ResourceHashUsage usage = getClientImpl().getClientApplication().getHashUsage(arrayRes->getHash());

        auto pimpl = std::make_unique<ArrayResourceImpl>(usage, *this, name);
        pimpl->initializeFromFrameworkData(arrayRes->getElementCount(), DataTypeUtils::ConvertDataTypeFromInternal(arrayRes->getElementType()));

        return &registerCreatedResourceObject<ArrayResource>(std::move(pimpl));
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    Texture2D* SceneImpl::createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, std::string_view name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedTexture(ramses_internal::EResourceType_Texture2D, width, height, 1u, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture2D: failed to create managed Texture2D resource");
            return nullptr;
        }
        return createHLTexture2D(res, name);
    }

    Texture2D* SceneImpl::createHLTexture2D(ramses_internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_Texture2D);

        const auto texRes = resource->convertTo<ramses_internal::TextureResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<Texture2DImpl>(hashUsage, *this, name);
        pimpl->initializeFromFrameworkData(texRes->getWidth(), texRes->getHeight(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()),
            TextureUtils::GetTextureSwizzleFromInternal(texRes->getTextureSwizzle()));

        return &registerCreatedResourceObject<Texture2D>(std::move(pimpl));
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    Texture3D* SceneImpl::createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, std::string_view name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedTexture(ramses_internal::EResourceType_Texture3D, width, height, depth, format, mipMapCount, mipLevelData, generateMipChain, {}, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture3D: failed to create managed Texture3D resource");
            return nullptr;
        }
        return createHLTexture3D(res, name);
    }

    Texture3D* SceneImpl::createHLTexture3D(ramses_internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_Texture3D);
        const auto texRes = resource->convertTo<ramses_internal::TextureResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<Texture3DImpl>(hashUsage, *this, name);
        pimpl->initializeFromFrameworkData(texRes->getWidth(), texRes->getHeight(), texRes->getDepth(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()));

        return &registerCreatedResourceObject<Texture3D>(std::move(pimpl));
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    TextureCube* SceneImpl::createTextureCube(uint32_t size, ETextureFormat format, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, std::string_view name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedTexture(ramses_internal::EResourceType_TextureCube, size, 1u, 1u, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureCube: failed to create managed TextureCube resource");
            return nullptr;
        }
        return createHLTextureCube(res, name);
    }

    TextureCube* SceneImpl::createHLTextureCube(ramses_internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_TextureCube);

        const auto texRes = resource->convertTo<ramses_internal::TextureResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<TextureCubeImpl>(hashUsage, *this, name);
        pimpl->initializeFromFrameworkData(texRes->getWidth(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()),
            TextureUtils::GetTextureSwizzleFromInternal(texRes->getTextureSwizzle()));

        return &registerCreatedResourceObject<TextureCube>(std::move(pimpl));
    }

    Effect* SceneImpl::createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, std::string_view name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedEffect(effectDesc, cacheFlag, name, m_effectErrorMessages);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createEffect: failed to create managed effect resource: " << m_effectErrorMessages);
            return nullptr;
        }

        return createHLEffect(res, name);
    }

    Effect* SceneImpl::createHLEffect(ramses_internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_Effect);

        const auto effectRes = resource->convertTo<ramses_internal::EffectResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<EffectImpl>(hashUsage, *this, name);
        std::optional<EDrawMode> gsInputType;
        if (effectRes->getGeometryShaderInputType() != ramses_internal::EDrawMode::NUMBER_OF_ELEMENTS)
            gsInputType = AppearanceUtils::GetDrawModeFromInternal(effectRes->getGeometryShaderInputType());
        pimpl->initializeFromFrameworkData(effectRes->getUniformInputs(), effectRes->getAttributeInputs(), gsInputType);

        return &registerCreatedResourceObject<Effect>(std::move(pimpl));
    }

    template <typename T, typename ImplT>
    T& SceneImpl::registerCreatedResourceObject(std::unique_ptr<ImplT> resourceImpl)
    {
        static_assert(std::is_base_of_v<Resource, T>, "Meant for Resource instances only");
        T& resource = m_objectRegistry.createAndRegisterObject<T, ImplT>(std::move(resourceImpl));

        const resourceId_t resId = resource.getResourceId();
        m_resources.insert({ resId, &resource });

        return resource;
    }

    std::string SceneImpl::getLastEffectErrorMessages() const
    {
        return m_effectErrorMessages;
    }

    ramses::Resource* SceneImpl::getResource(resourceId_t rid) const
    {
        const auto range = m_resources.equal_range(rid);
        return (range.first != range.second) ? range.first->second : nullptr;
    }

    Resource* SceneImpl::scanForResourceWithHash(ramses_internal::ResourceContentHash hash) const
    {
        for (const auto& res : m_resources)
        {
            if (hash == res.second->m_impl.getLowlevelResourceHash())
                return res.second;
        }

        return nullptr;
    }

    status_t SceneImpl::writeSceneObjectsToStream(ramses_internal::IOutputStream& outputStream) const
    {
        ramses_internal::ScenePersistation::WriteSceneMetadataToStream(outputStream, getIScene());
        ramses_internal::ScenePersistation::WriteSceneToStream(outputStream, getIScene());

        SerializationContext serializationContext;
        return serialize(outputStream, serializationContext);
    }

    status_t SceneImpl::saveToFile(std::string_view fileName, bool compress) const
    {
        if (fileName.empty())
            return addErrorEntry("Scene::saveToFile failed: empty filename");

        LOG_INFO_P(CONTEXT_CLIENT, "Scene::saveToFile: filename '{}', compress {}", fileName, compress);

        ramses_internal::File outputFile(fileName);
        ramses_internal::BinaryFileOutputStream outputStream(outputFile);
        if (!outputFile.isOpen())
            return addErrorEntry(fmt::format("Scene::saveToFile failed, could not open file for writing: '{}'", fileName));

        const EFeatureLevel featureLevel = m_hlClient.m_impl.getFramework().getFeatureLevel();
        ramses_internal::RamsesVersion::WriteToStream(outputStream, ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, featureLevel);

        ramses_internal::UInt bytesForVersion = 0;
        if (!outputFile.getPos(bytesForVersion))
            return addErrorEntry(fmt::format("Scene::saveToFile failed, error getting save file position: '{}'", fileName));

        // reserve space for offset to SceneObjects and LL-Objects
        const uint64_t bytesForOffsets = sizeof(uint64_t) * 2u;
        const uint64_t offsetSceneObjectsStart = bytesForVersion + bytesForOffsets;

        if (!outputFile.seek(static_cast<ramses_internal::Int>(offsetSceneObjectsStart), ramses_internal::File::SeekOrigin::BeginningOfFile))
            return addErrorEntry(fmt::format("Scene::saveToFile failed, error seeking file: '{}'", fileName));

        const status_t status = writeSceneObjectsToStream(outputStream);

        ramses_internal::UInt offsetLLResourcesStart = 0;
        if (!outputFile.getPos(offsetLLResourcesStart))
            return addErrorEntry(fmt::format("Scene::saveToFile failed, error getting save file position: '{}'", fileName));

        ResourceObjects resources;
        resources.reserve(m_resources.size());
        for (auto const& res : m_resources)
            resources.push_back(res.second);
        getClientImpl().writeLowLevelResourcesToStream(resources, outputStream, compress);

        if (!outputFile.seek(bytesForVersion, ramses_internal::File::SeekOrigin::BeginningOfFile))
            return addErrorEntry(fmt::format("Scene::saveToFile failed, error seeking file: '{}'", fileName));

        outputStream << static_cast<uint64_t>(offsetSceneObjectsStart);
        outputStream << static_cast<uint64_t>(offsetLLResourcesStart);

        if (!outputFile.close())
            return addErrorEntry(fmt::format("Scene::saveToFile failed, close file failed: '{}'", fileName));

        LOG_INFO_P(ramses_internal::CONTEXT_CLIENT, "Scene::saveToFile: done writing '{}'", fileName);

        return status;
    }

    void SceneImpl::setSceneFileHandle(ramses_internal::SceneFileHandle handle)
    {
        m_sceneFileHandle = handle;
    }

    void SceneImpl::closeSceneFile()
    {
        if (!m_sceneFileHandle.isValid())
            return;

        getClientImpl().getClientApplication().removeResourceFile(m_sceneFileHandle);
        LOG_INFO(CONTEXT_CLIENT, "SceneImpl::closeSceneFile closed: " << m_sceneFileHandle);
        m_sceneFileHandle = ramses_internal::SceneFileHandle::Invalid();
    }

    ramses_internal::SceneFileHandle SceneImpl::getSceneFileHandle() const
    {
        return m_sceneFileHandle;
    }

    bool SceneImpl::removeResourceWithIdFromResources(resourceId_t const& id, Resource& resource)
    {
        auto range = m_resources.equal_range(id);
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == &resource)
            {
                m_resources.erase(it);
                return true;
            }
        }

        return false;
    }

    void SceneImpl::updateResourceId(resourceId_t const& oldId, Resource& resourceWithNewId)
    {
        removeResourceWithIdFromResources(oldId, resourceWithNewId);
        m_resources.insert({ resourceWithNewId.getResourceId(), &resourceWithNewId });
    }

    template ArrayResource* SceneImpl::createArrayResource<uint16_t>(uint32_t, const uint16_t*, resourceCacheFlag_t, std::string_view);
    template ArrayResource* SceneImpl::createArrayResource<uint32_t>(uint32_t, const uint32_t*, resourceCacheFlag_t, std::string_view);
    template ArrayResource* SceneImpl::createArrayResource<float>(uint32_t, const float*, resourceCacheFlag_t, std::string_view);
    template ArrayResource* SceneImpl::createArrayResource<vec2f>(uint32_t, const vec2f*, resourceCacheFlag_t, std::string_view);
    template ArrayResource* SceneImpl::createArrayResource<vec3f>(uint32_t, const vec3f*, resourceCacheFlag_t, std::string_view);
    template ArrayResource* SceneImpl::createArrayResource<vec4f>(uint32_t, const vec4f*, resourceCacheFlag_t, std::string_view);
    template ArrayResource* SceneImpl::createArrayResource<Byte>(uint32_t, const Byte*, resourceCacheFlag_t, std::string_view);
}
