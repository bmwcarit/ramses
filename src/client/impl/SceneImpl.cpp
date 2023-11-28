//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SceneImpl.h"

#include "ramses/client/RamsesClient.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Effect.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/SceneReference.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Resource.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/Effect.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LogicObject.h"
#include "ramses/framework/EScenePublicationMode.h"

#include "impl/CameraNodeImpl.h"
#include "impl/EffectImpl.h"
#include "impl/NodeImpl.h"
#include "impl/AppearanceImpl.h"
#include "impl/GeometryImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/RenderTargetImpl.h"
#include "impl/RenderTargetDescriptionImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/TextureUtils.h"
#include "impl/Texture2DImpl.h"
#include "impl/Texture3DImpl.h"
#include "impl/TextureCubeImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/EffectInputImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/SceneConfigImpl.h"
#include "impl/DataObjectImpl.h"
#include "impl/BlitPassImpl.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "impl/SerializationHelper.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/Texture2DBufferImpl.h"
#include "impl/PickableObjectImpl.h"
#include "impl/SceneReferenceImpl.h"
#include "impl/DataTypeUtils.h"
#include "impl/AppearanceUtils.h"
#include "impl/logic/LogicEngineImpl.h"
#include "impl/SaveFileConfigImpl.h"
#include "impl/logic/LogicObjectImpl.h"
#include "impl/SerializationContext.h"

#include "internal/SceneGraph/Scene/Scene.h"
#include "internal/SceneGraph/Scene/ScenePersistation.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/ClientCommands/SceneCommandBuffer.h"
#include "internal/ClientCommands/SceneCommandVisitor.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/PlatformAbstraction/PlatformMath.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Components/EffectUniformTime.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Core/Utils/TextureMathUtils.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/DataSlotUtils.h"
#include "internal/RamsesVersion.h"

#include "ramses-sdk-build-config.h"
#include "fmt/format.h"
#include <array>
#include <unordered_set>

namespace ramses::internal
{
    SceneImpl::SceneImpl(ramses::internal::ClientScene& scene, const SceneConfigImpl& sceneConfig, RamsesClient& ramsesClient)
        : ClientObjectImpl(ramsesClient.impl(), ERamsesObjectType::Scene, scene.getName().c_str())
        , m_scene(scene)
        , m_nextSceneVersion(InvalidSceneVersionTag)
        , m_futurePublicationMode(sceneConfig.getPublicationMode())
        , m_hlClient(ramsesClient)
    {
        LOG_INFO(CONTEXT_CLIENT, "Scene::Scene: sceneId {}, publicationMode {}", scene.getSceneId(), sceneConfig.getPublicationMode() == EScenePublicationMode::LocalAndRemote ? "LocalAndRemote" : "LocalOnly");
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

    bool SceneImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!ClientObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(m_expirationTimestamp.time_since_epoch()).count());

        if (!SerializationHelper::SerializeObjectsInRegistry<SceneObject>(outStream, serializationContext, m_objectRegistry))
            return false;

        outStream << m_lastSceneObjectId.getValue();

        return true;
    }

    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType /*unused*/)
    {
        return std::make_unique<T>(scene, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, ERamsesObjectType, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType type)
    {
        return std::make_unique<T>(scene, type, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, ramses::internal::ResourceHashUsage, SceneImpl&, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType /*unused*/)
    {
        return std::make_unique<T>(ramses::internal::ResourceHashUsage{}, scene, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, ERamsesObjectType, ramses::EDataType, std::string_view>::value, T>::type* = nullptr>
    std::unique_ptr<T> createImplHelper(SceneImpl& scene, ERamsesObjectType type)
    {
        return std::make_unique<T>(scene, type, ramses::EDataType::Int32, "");
    }

    template <typename ObjectType, typename ObjectImplType>
    bool SceneImpl::createAndDeserializeObjectImpls(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count)
    {
        for (uint32_t i = 0u; i < count; ++i)
        {
            auto impl = createImplHelper<ObjectImplType>(*this, TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID);
            ObjectIDType objectID = SerializationHelper::DeserializeObjectID(inStream);
            if (!impl->deserialize(inStream, serializationContext))
                return false;

            serializationContext.registerObjectImpl(impl.get(), objectID);

            auto& object = m_objectRegistry.createAndRegisterObject<ObjectType>(std::move(impl));

            if constexpr (std::is_base_of_v<Resource, ObjectType>)
                m_resources.insert({ object.getResourceId(), &object });
        }

        return true;
    }

    bool SceneImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!ClientObjectImpl::deserialize(inStream, serializationContext))
            return false;

        uint64_t expirationTS = 0u;
        inStream >> expirationTS;
        m_expirationTimestamp = ramses::internal::FlushTime::Clock::time_point(std::chrono::milliseconds(expirationTS));

        uint32_t totalCount = 0u;
        uint32_t typesCount = 0u;
        SerializationHelper::DeserializeNumberOfObjectTypes(inStream, totalCount, typesCount);
        serializationContext.resize(totalCount, m_scene.getNodeCount());
        m_objectRegistry.reserveAdditionalGeneralCapacity(totalCount);

        std::array<uint32_t, RamsesObjectTypeCount> objectCounts = {};

        for (uint32_t i = 0u; i < typesCount; ++i)
        {
            uint32_t count = 0u;
            const ERamsesObjectType type = SerializationHelper::DeserializeObjectTypeAndCount(inStream, count);
            assert(m_objectRegistry.getNumberOfObjects(type) == 0u);
            m_objectRegistry.reserveAdditionalObjectCapacity(type, count);
            objectCounts[i] = count;

            bool status = true;
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
            case ERamsesObjectType::Geometry:
                status = createAndDeserializeObjectImpls<Geometry, GeometryImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderGroup:
                status = createAndDeserializeObjectImpls<ramses::RenderGroup, RenderGroupImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderPass:
                status = createAndDeserializeObjectImpls<ramses::RenderPass, RenderPassImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::BlitPass:
                status = createAndDeserializeObjectImpls<ramses::BlitPass, BlitPassImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::PickableObject:
                status = createAndDeserializeObjectImpls<ramses::PickableObject, PickableObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::SceneReference:
                status = createAndDeserializeObjectImpls<ramses::SceneReference, SceneReferenceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderBuffer:
                status = createAndDeserializeObjectImpls<ramses::RenderBuffer, RenderBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::RenderTarget:
                status = createAndDeserializeObjectImpls<ramses::RenderTarget, RenderTargetImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::TextureSampler:
                status = createAndDeserializeObjectImpls<ramses::TextureSampler, TextureSamplerImpl>(inStream, serializationContext, count);
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
            case ERamsesObjectType::ArrayBuffer:
                status = createAndDeserializeObjectImpls<ArrayBuffer, ArrayBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::Texture2DBuffer:
                status = createAndDeserializeObjectImpls<Texture2DBuffer, Texture2DBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::ArrayResource:
                status = createAndDeserializeObjectImpls<ramses::ArrayResource, ArrayResourceImpl>(inStream, serializationContext, count);
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
            case ERamsesObjectType::LogicEngine:
                status = createAndDeserializeObjectImpls<LogicEngine, LogicEngineImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType::Invalid:
            case ERamsesObjectType::ClientObject:
            case ERamsesObjectType::RamsesObject:
            case ERamsesObjectType::SceneObject:
            case ERamsesObjectType::Resource:
            case ERamsesObjectType::Camera:
            case ERamsesObjectType::Client:
            case ERamsesObjectType::Scene:
            case ERamsesObjectType::LogicObject:
                getErrorReporting().set("Scene::deserialize failed, unexpected object type in file stream.");
                return false;
            }

            if (!status)
                return false;
        }

        inStream >> m_lastSceneObjectId.getReference();

        LOG_DEBUG_F(CONTEXT_PROFILING, ([&](ramses::internal::StringOutputStream& sos) {
                    sos << "SceneImpl::deserialize: HL scene object counts for SceneID " << m_scene.getSceneId() << "\n";
                    for (uint32_t i = 0; i < objectCounts.size(); i++)
                    {
                        if (objectCounts[i] > 0)
                        {
                            sos << "  " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(static_cast<ERamsesObjectType>(i)) << " count: " << objectCounts[i] << "\n";
                        }
                    }
                }));

        return serializationContext.resolveDependencies();
    }

    void SceneImpl::onValidate(ValidationReportImpl& report) const
    {
        for (size_t i = 0u; i < RamsesObjectTypeCount; ++i)
        {
            const auto type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType::SceneObject)
                && RamsesObjectTypeUtils::IsConcreteType(type))
            {
                SceneObjectRegistryIterator iter(getObjectRegistry(), ERamsesObjectType(i));
                while (const auto* obj = iter.getNext())
                {
                    obj->impl().validate(report);
                }
            }
        }

        // special validation (see SceneImpl::createTextureConsumer(const TextureSamplerExternal&, dataConsumerId_t)),
        // duplicate IDs are temporarily allowed but validation still reports them as errors
        // TODO vaclav do this properly
        std::unordered_set<ramses::internal::DataSlotId> texConsumerIds;
        for (const auto& it : m_scene.getDataSlots())
        {
            if (it.second->type == ramses::internal::EDataSlotType::TextureConsumer)
            {
                const auto consumerId = it.second->id;
                if (texConsumerIds.count(consumerId) > 0u)
                {
                    report.add(
                        EIssueType::Error,
                        fmt::format("Duplicate texture consumer ID '{}' is not allowed and will result in unknown behavior when linking on renderer", consumerId.getValue())
                        , &getRamsesObject());
                }
                texConsumerIds.insert(consumerId);
            }
        }
    }

    LogicEngine* SceneImpl::createLogicEngine(std::string_view name)
    {
        auto pimpl = std::make_unique<LogicEngineImpl>(*this, name);
        return &m_objectRegistry.createAndRegisterObject<LogicEngine>(std::move(pimpl));
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
        if (this != &effect.impl().getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createAppearance failed, effect is not from this scene.");
            return nullptr;
        }

        auto pimpl = std::make_unique<AppearanceImpl>(*this, name);
        pimpl->initializeFrameworkData(effect.impl());

        return &m_objectRegistry.createAndRegisterObject<Appearance>(std::move(pimpl));
    }

    Geometry* SceneImpl::createGeometry(const Effect& effect, std::string_view name)
    {
        if (this != &effect.impl().getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createGeometry failed, effect is not from this scene.");
            return nullptr;
        }

        auto pimpl = std::make_unique<GeometryImpl>(*this, name);
        pimpl->initializeFrameworkData(effect.impl());

        return &m_objectRegistry.createAndRegisterObject<Geometry>(std::move(pimpl));
    }

    bool SceneImpl::destroy(SceneObject& object)
    {
        if (!containsSceneObject(object.impl()))
        {
            getErrorReporting().set("Scene::destroy failed, object is not in this scene.", *this);
            return false;
        }

        switch (object.getType())
        {
        case ERamsesObjectType::RenderTarget:
            return destroyRenderTarget(RamsesObjectTypeUtils::ConvertTo<ramses::RenderTarget>(object));
        case ERamsesObjectType::OrthographicCamera:
        case ERamsesObjectType::PerspectiveCamera:
            return destroyCamera(RamsesObjectTypeUtils::ConvertTo<ramses::Camera>(object));
        case ERamsesObjectType::RenderGroup:
            return destroyRenderGroup(RamsesObjectTypeUtils::ConvertTo<ramses::RenderGroup>(object));
        case ERamsesObjectType::Node:
        case ERamsesObjectType::PickableObject:
            return destroyNode(RamsesObjectTypeUtils::ConvertTo<Node>(object));
        case ERamsesObjectType::MeshNode:
            return destroyMeshNode(RamsesObjectTypeUtils::ConvertTo<MeshNode>(object));
        case ERamsesObjectType::DataObject:
            return destroyDataObject(RamsesObjectTypeUtils::ConvertTo<DataObject>(object));
        case ERamsesObjectType::TextureSampler:
            return destroyTextureSampler(RamsesObjectTypeUtils::ConvertTo<ramses::TextureSampler>(object));
        case ERamsesObjectType::TextureSamplerMS:
            return destroyTextureSampler(RamsesObjectTypeUtils::ConvertTo<TextureSamplerMS>(object));
        case ERamsesObjectType::TextureSamplerExternal:
            return destroyTextureSampler(RamsesObjectTypeUtils::ConvertTo<TextureSamplerExternal>(object));
        case ERamsesObjectType::Appearance:
        case ERamsesObjectType::Geometry:
        case ERamsesObjectType::RenderPass:
        case ERamsesObjectType::BlitPass:
        case ERamsesObjectType::RenderBuffer:
        case ERamsesObjectType::ArrayBuffer:
        case ERamsesObjectType::Texture2DBuffer:
        case ERamsesObjectType::LogicEngine:
            return destroyObject(object);
        case ERamsesObjectType::LogicObject:
            getErrorReporting().set("Scene::destroy cannot destroy logic object, use LogicEngine::destroy to destroy logic objects.", *this);
            return false;
        case ERamsesObjectType::SceneReference: {
            auto& sceneReference = RamsesObjectTypeUtils::ConvertTo<ramses::SceneReference>(object);
            LOG_INFO(CONTEXT_CLIENT, "Scene::destroySceneReference: (master {} / ref {})", getSceneId(), sceneReference.getReferencedSceneId());
            m_sceneReferences.remove(sceneReference.getReferencedSceneId());
            return destroyObject(object);
        }
        case ERamsesObjectType::Texture2D:
        case ERamsesObjectType::Texture3D:
        case ERamsesObjectType::TextureCube:
        case ERamsesObjectType::Effect:
        case ERamsesObjectType::ArrayResource:
            return destroyResource(RamsesObjectTypeUtils::ConvertTo<Resource>(object));
        case ERamsesObjectType::Invalid:
        case ERamsesObjectType::ClientObject:
        case ERamsesObjectType::RamsesObject:
        case ERamsesObjectType::SceneObject:
        case ERamsesObjectType::Resource:
        case ERamsesObjectType::Camera:
        case ERamsesObjectType::Client:
        case ERamsesObjectType::Scene:
            getErrorReporting().set("Scene::destroy internal error, cannot destroy object!");
            break;
        }

        assert(false);
        return false;
    }

    bool SceneImpl::destroyRenderTarget(ramses::RenderTarget& renderTarget)
    {
        SceneObjectRegistryIterator iterator(m_objectRegistry, ERamsesObjectType::RenderPass);
        const ramses::RenderPass* renderPass = nullptr;
        while ((renderPass = iterator.getNext<ramses::RenderPass>()) != nullptr)
        {
            if (&renderTarget == renderPass->impl().getRenderTarget())
            {
                getErrorReporting().set("Scene::destroy can not destroy render target while it is still assigned to a render pass!", *this);
                return false;
            }
        }
        return destroyObject(renderTarget);
    }

    bool SceneImpl::destroyCamera(ramses::Camera& camera)
    {
        if (cameraIsAssignedToRenderPasses(camera))
        {
            getErrorReporting().set("Scene::destroy can not destroy camera while it is still assigned to a render pass!", *this);
            return false;
        }

        return destroyNode(camera);
    }

    bool SceneImpl::destroyRenderGroup(ramses::RenderGroup& group)
    {
        removeObjectFromAllContainers<ramses::RenderGroup, ramses::RenderPass>(group);
        removeObjectFromAllContainers<ramses::RenderGroup, ramses::RenderGroup>(group);

        return destroyObject(group);
    }

    bool SceneImpl::destroyMeshNode(MeshNode& mesh)
    {
        removeObjectFromAllContainers<MeshNode, ramses::RenderGroup>(mesh);
        return destroyNode(mesh);
    }

    void SceneImpl::markAllChildrenDirty(Node& node)
    {
        for (size_t i = 0u; i < node.getChildCount(); ++i)
        {
            m_objectRegistry.setNodeDirty(node.impl().getChildImpl(i), true);
        }
    }

    bool SceneImpl::destroyNode(Node& node)
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

    bool SceneImpl::destroyDataObject(DataObject& dataObject)
    {
        const ramses::internal::DataInstanceHandle dataRef = dataObject.impl().getDataReference();
        const uint32_t slotHandleCount = m_scene.getDataSlotCount();
        for (ramses::internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
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
    bool SceneImpl::destroyTextureSampler(SAMPLER& sampler)
    {
        const ramses::internal::TextureSamplerHandle& samplerHandle = sampler.impl().getTextureSamplerHandle();
        const uint32_t slotHandleCount = m_scene.getDataSlotCount();
        for (ramses::internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) &&
                m_scene.getDataSlot(slotHandle).attachedTextureSampler == samplerHandle)
            {
                m_scene.releaseDataSlot(slotHandle);
            }
        }

        return destroyObject(sampler);
    }

    bool SceneImpl::destroyResource(Resource& resource)
    {
        const resourceId_t resId = resource.impl().getResourceId();
        const bool found = removeResourceWithIdFromResources(resId, resource);
        if (!found)
            assert(false);

        return destroyObject(resource);
    }

    bool SceneImpl::destroyObject(SceneObject& object)
    {
        object.impl().deinitializeFrameworkData();
        m_objectRegistry.destroyAndUnregisterObject(object);
        return true;
    }

    bool SceneImpl::publish(EScenePublicationMode requestedPublicationMode)
    {
        if (isPublished())
        {
            getErrorReporting().set((ramses::internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::publish: ignored, scene is already published").c_str(), *this);
            return false;
        }
        if (requestedPublicationMode == EScenePublicationMode::LocalAndRemote && m_futurePublicationMode == EScenePublicationMode::LocalOnly)
        {
            getErrorReporting().set((ramses::internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::publish: Enabled local only optimisations from SceneConfig, cannot remote publish later").c_str(), *this);
            return false;
        }
        if (requestedPublicationMode != EScenePublicationMode::LocalOnly && !getClientImpl().getFramework().isConnected())
        {
            LOG_INFO(CONTEXT_CLIENT, "Scene({})::publish(LocalAndRemote): Scene is only published locally until framework is connected (RamsesFramework::connect)", m_scene.getSceneId());
        }
        getClientImpl().getClientApplication().publishScene(m_scene.getSceneId(), requestedPublicationMode);
        return true;
    }

    bool SceneImpl::unpublish()
    {
        if (!isPublished())
        {
            getErrorReporting().set((ramses::internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::unpublish ignored, scene is not published.").c_str(), *this);
            return false;
        }

        getClientImpl().getClientApplication().unpublishScene(m_scene.getSceneId());
        return true;
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

    ramses::RenderGroup* SceneImpl::createRenderGroup(std::string_view name)
    {
        auto pimpl = std::make_unique<RenderGroupImpl>(*this, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<ramses::RenderGroup>(std::move(pimpl));
    }

    ramses::RenderPass* SceneImpl::createRenderPass(std::string_view name /*= {}*/)
    {
        return createRenderPassInternal(name);
    }

    ramses::BlitPass* SceneImpl::createBlitPass(const ramses::RenderBuffer& sourceRenderBuffer, const ramses::RenderBuffer& destinationRenderBuffer, std::string_view name)
    {
        if (!containsSceneObject(sourceRenderBuffer.impl()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createBlitPass failed, source render buffer is not from this scene.", m_scene.getSceneId());
            return nullptr;
        }

        if (!containsSceneObject(destinationRenderBuffer.impl()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createBlitPass failed, destination render buffer is not from this scene.", m_scene.getSceneId());
            return nullptr;
        }

        if (sourceRenderBuffer.getBufferFormat() != destinationRenderBuffer.getBufferFormat())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createBlitPass failed, source and destination buffers have different buffer formats", m_scene.getSceneId());
            return nullptr;
        }

        if (sourceRenderBuffer.getHeight() != destinationRenderBuffer.getHeight() ||
            sourceRenderBuffer.getWidth() != destinationRenderBuffer.getWidth())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createBlitPass failed, source and destination buffers have different dimensions", m_scene.getSceneId());
            return nullptr;
        }

        if (&sourceRenderBuffer == &destinationRenderBuffer)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createBlitPass failed, source and destination cannot be the same buffer", m_scene.getSceneId());
            return nullptr;
        }

        auto pimpl = std::make_unique<BlitPassImpl>(*this, name);
        pimpl->initializeFrameworkData(sourceRenderBuffer.impl(), destinationRenderBuffer.impl());

        return &m_objectRegistry.createAndRegisterObject<ramses::BlitPass>(std::move(pimpl));
    }

    ramses::PickableObject* SceneImpl::createPickableObject(const ArrayBuffer& geometryBuffer, const pickableObjectId_t id, std::string_view name)
    {
        if (!containsSceneObject(geometryBuffer.impl()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createPickableObject failed, geometry buffer is not from this scene.", m_scene.getSceneId());
            return nullptr;
        }

        if (geometryBuffer.getDataType() != ramses::EDataType::Vector3F || 0 != (geometryBuffer.impl().getElementCount() % 3))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createPickableObject failed, geometry buffer has the wrong format.", m_scene.getSceneId());
            return nullptr;
        }

        auto pimpl = std::make_unique<PickableObjectImpl>(*this, name);
        pimpl->initializeFrameworkData(geometryBuffer.impl(), id);

        return &m_objectRegistry.createAndRegisterObject<ramses::PickableObject>(std::move(pimpl));
    }

    ramses::RenderBuffer* SceneImpl::createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, std::string_view name)
    {
        if (0 == width || 0 == height)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene({})::createRenderBuffer failed: cannot create a render buffer with 0 width and/or height!", m_scene.getSceneId());
            return nullptr;
        }

        auto pimpl = std::make_unique<RenderBufferImpl>(*this, name);
        pimpl->initializeFrameworkData(width, height, bufferFormat, accessMode, sampleCount);

        return &m_objectRegistry.createAndRegisterObject<ramses::RenderBuffer>(std::move(pimpl));
    }

    ramses::RenderTarget* SceneImpl::createRenderTarget(const RenderTargetDescriptionImpl& rtDesc, std::string_view name)
    {
        ValidationReportImpl report;
        rtDesc.validate(report);
        if (report.hasError())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createRenderTarget failed, RenderTargetDescription is invalid: {}", report.toString());
            return nullptr;
        }

        auto pimpl = std::make_unique<RenderTargetImpl>(*this, name);
        pimpl->initializeFrameworkData(rtDesc);

        return &m_objectRegistry.createAndRegisterObject<ramses::RenderTarget>(std::move(pimpl));
    }

    ramses::TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        const Texture2D& texture,
        std::string_view name /*= {}*/)
    {
        if (this != &texture.impl().getSceneImpl())
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
            ramses::internal::TextureSampler::ContentType::ClientTexture,
            texture.impl().getLowlevelResourceHash(),
            ramses::internal::InvalidMemoryHandle,
            name);
    }

    ramses::TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureAddressMode wrapRMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const Texture3D& texture,
        std::string_view name /*= {}*/)
    {
        if (this != &texture.impl().getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture 3D is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, wrapRMode, minSamplingMethod, magSamplingMethod, 1u,
            ERamsesObjectType::Texture3D,
            ramses::internal::TextureSampler::ContentType::ClientTexture,
            texture.impl().getLowlevelResourceHash(),
            ramses::internal::InvalidMemoryHandle,
            name);
    }

    ramses::TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        const TextureCube& texture,
        std::string_view name /*= {}*/)
    {
        if (this != &texture.impl().getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture Cube is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode::Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType::TextureCube,
            ramses::internal::TextureSampler::ContentType::ClientTexture,
            texture.impl().getLowlevelResourceHash(),
            ramses::internal::InvalidMemoryHandle,
            name);
    }

    ramses::TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod ,
        uint32_t anisotropyLevel,
        const ramses::RenderBuffer& renderBuffer,
        std::string_view name)
    {
        if (renderBuffer.getSampleCount() > 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, cannot create this type of TextureSampler with multisampled RenderBuffer.");
            return nullptr;
        }

        if (!containsSceneObject(renderBuffer.impl()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer is not from this scene.");
            return nullptr;
        }

        if (ERenderBufferAccessMode::WriteOnly == renderBuffer.impl().getAccessMode())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer has access mode write only.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode::Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType::RenderBuffer,
            ramses::internal::TextureSampler::ContentType::RenderBuffer,
            ramses::internal::ResourceContentHash::Invalid(),
            renderBuffer.impl().getRenderBufferHandle().asMemoryHandle(),
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
        if (!containsSceneObject(textureBuffer.impl()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture2D buffer is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode::Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType::Texture2DBuffer,
            ramses::internal::TextureSampler::ContentType::TextureBuffer,
            ramses::internal::ResourceContentHash::Invalid(),
            textureBuffer.impl().getTextureBufferHandle().asMemoryHandle(),
            name);
    }

    ramses::TextureSamplerMS* SceneImpl::createTextureSamplerMS(const ramses::RenderBuffer& renderBuffer, std::string_view name)
    {
        if (!containsSceneObject(renderBuffer.impl()))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer is not from this scene.");
            return nullptr;
        }

        if (ERenderBufferAccessMode::WriteOnly == renderBuffer.impl().getAccessMode())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer has access mode write only.");
            return nullptr;
        }

        auto samplerImpl = std::make_unique<TextureSamplerImpl>(*this, ERamsesObjectType::TextureSamplerMS, name);
        samplerImpl->initializeFrameworkData(
            {},
            ERamsesObjectType::RenderBuffer,
            ramses::internal::TextureSampler::ContentType::RenderBufferMS,
            ramses::internal::ResourceContentHash::Invalid(),
            renderBuffer.impl().getRenderBufferHandle().asMemoryHandle());

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
        ramses::internal::TextureSamplerStates samplerStates(
            ETextureAddressMode::Clamp,
            ETextureAddressMode::Clamp,
            ETextureAddressMode::Clamp,
            minSamplingMethod,
            magSamplingMethod
            );

        auto samplerImpl = std::make_unique<TextureSamplerImpl>(*this, ERamsesObjectType::TextureSamplerExternal, name);
        samplerImpl->initializeFrameworkData(
            samplerStates,
            ERamsesObjectType::TextureSamplerExternal,
            ramses::internal::TextureSampler::ContentType::ExternalTexture,
            ramses::internal::ResourceContentHash::Invalid(),
            ramses::internal::InvalidMemoryHandle);

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
        ramses::internal::TextureSampler::ContentType contentType,
        ramses::internal::ResourceContentHash textureResourceHash,
        ramses::internal::MemoryHandle contentHandle,
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

        ramses::internal::TextureSamplerStates samplerStates(
            wrapUMode,
            wrapVMode,
            wrapRMode,
            minSamplingMethod,
            magSamplingMethod,
            anisotropyLevel
            );

        auto samplerImpl = std::make_unique<TextureSamplerImpl>(*this, ERamsesObjectType::TextureSampler, name);
        samplerImpl->initializeFrameworkData(samplerStates, samplerType, contentType, textureResourceHash, contentHandle);

        return &m_objectRegistry.createAndRegisterObject<ramses::TextureSampler>(std::move(samplerImpl));
    }

    DataObject* SceneImpl::createDataObject(ramses::EDataType dataType, std::string_view name /*= {}*/)
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

    bool SceneImpl::createTransformationDataProvider(const Node& node, dataProviderId_t id)
    {
        if (!containsSceneObject(node.impl()))
        {
            getErrorReporting().set("Scene::createTransformationDataProvider failed, node is not from this scene.", *this);
            return false;
        }

        const ramses::internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses::internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            getErrorReporting().set("Scene::createTransformationDataProvider failed, duplicate data slot id", *this);
            return false;
        }

        const ramses::internal::NodeHandle nodeHandle = node.impl().getNodeHandle();
        if (ramses::internal::DataSlotUtils::HasDataSlotIdForNode(m_scene, nodeHandle))
        {
            getErrorReporting().set("Scene::createTransformationDataProvider failed, Node already has a transformation data slot assigned", *this);
            return false;
        }

        m_scene.allocateDataSlot({ ramses::internal::EDataSlotType::TransformationProvider, internalDataSlotId, nodeHandle, ramses::internal::DataInstanceHandle::Invalid(), ramses::internal::ResourceContentHash::Invalid(), ramses::internal::TextureSamplerHandle() }, {});
        return true;
    }

    bool SceneImpl::createTransformationDataConsumer(const Node& node, dataConsumerId_t id)
    {
        if (!containsSceneObject(node.impl()))
        {
            getErrorReporting().set("Scene::createTransformationDataConsumer failed, Group Node is not from this scene.", *this);
            return false;
        }

        const ramses::internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses::internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            getErrorReporting().set("Scene::createTransformationDataConsumer failed, duplicate data slot id", *this);
            return false;
        }

        const ramses::internal::NodeHandle nodeHandle = node.impl().getNodeHandle();
        if (ramses::internal::DataSlotUtils::HasDataSlotIdForNode(m_scene, nodeHandle))
        {
            getErrorReporting().set("Scene::createTransformationDataConsumer failed, Node already has a transformation data slot assigned", *this);
            return false;
        }

        m_scene.allocateDataSlot({ ramses::internal::EDataSlotType::TransformationConsumer, internalDataSlotId, nodeHandle, ramses::internal::DataInstanceHandle::Invalid(), ramses::internal::ResourceContentHash::Invalid(), ramses::internal::TextureSamplerHandle() }, {});
        return true;
    }

    bool SceneImpl::createDataProvider(const DataObject& dataObject, dataProviderId_t id)
    {
        if (!containsSceneObject(dataObject.impl()))
        {
            getErrorReporting().set("Scene::createDataProvider failed, data object is not from this scene.", *this);
            return false;
        }

        const ramses::internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses::internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            getErrorReporting().set("Scene::createDataProvider failed, duplicate data slot id", *this);
            return false;
        }

        const ramses::internal::DataInstanceHandle dataRef = dataObject.impl().getDataReference();
        if (ramses::internal::DataSlotUtils::HasDataSlotIdForDataObject(m_scene, dataRef))
        {
            getErrorReporting().set("Scene::createDataProvider failed, data object already has a data slot assigned", *this);
            return false;
        }

        m_scene.allocateDataSlot({ ramses::internal::EDataSlotType::DataProvider, internalDataSlotId, ramses::internal::NodeHandle(), dataRef, ramses::internal::ResourceContentHash::Invalid(), ramses::internal::TextureSamplerHandle() }, {});
        return true;
    }

    bool SceneImpl::createDataConsumer(const DataObject& dataObject, dataConsumerId_t id)
    {
        if (!containsSceneObject(dataObject.impl()))
        {
            getErrorReporting().set("Scene::createDataConsumer failed, data object is not from this scene.", *this);
            return false;
        }

        const ramses::internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses::internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            getErrorReporting().set("Scene::createDataConsumer failed, duplicate data slot id", *this);
            return false;
        }

        const ramses::internal::DataInstanceHandle dataRef = dataObject.impl().getDataReference();
        if (ramses::internal::DataSlotUtils::HasDataSlotIdForDataObject(m_scene, dataRef))
        {
            getErrorReporting().set("Scene::createDataConsumer failed, data object already has a data slot assigned", *this);
            return false;
        }

        m_scene.allocateDataSlot({ ramses::internal::EDataSlotType::DataConsumer, internalDataSlotId, ramses::internal::NodeHandle(), dataRef, ramses::internal::ResourceContentHash::Invalid(), ramses::internal::TextureSamplerHandle() }, {});
        return true;
    }

    bool SceneImpl::createTextureProvider(const Texture2D& texture, dataProviderId_t id)
    {
        if (this != &texture.impl().getSceneImpl())
        {
            getErrorReporting().set("Scene::createTextureProvider failed, texture is not from this scene.", *this);
            return false;
        }

        const ramses::internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses::internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            getErrorReporting().set("Scene::createTextureProvider failed, duplicate data slot id", *this);
            return false;
        }

        const ramses::internal::ResourceContentHash& textureHash = texture.impl().getLowlevelResourceHash();
        if (ramses::internal::DataSlotUtils::HasDataSlotIdForTexture(m_scene, textureHash))
        {
            getErrorReporting().set("Scene::createTextureProvider failed, texture already has a data slot assigned in this scene", *this);
            return false;
        }

        m_scene.allocateDataSlot({ ramses::internal::EDataSlotType::TextureProvider, internalDataSlotId, ramses::internal::NodeHandle(), ramses::internal::DataInstanceHandle::Invalid(), textureHash, ramses::internal::TextureSamplerHandle() }, {});
        return true;
    }

    bool SceneImpl::updateTextureProvider(const Texture2D& texture, dataProviderId_t id)
    {
        if (this != &texture.impl().getSceneImpl())
        {
            getErrorReporting().set("Scene::updateTextureProvider failed, texture is not from this scene.", *this);
            return false;
        }

        const ramses::internal::DataSlotId internalDataSlotId(id.getValue());
        if (!ramses::internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            getErrorReporting().set("Scene::updateTextureProvider failed, provider has not been created before.", *this);
            return false;
        }

        const uint32_t slotHandleCount = m_scene.getDataSlotCount();
        for (ramses::internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) && m_scene.getDataSlot(slotHandle).id == internalDataSlotId)
            {
                const ramses::internal::ResourceContentHash& textureHash = texture.impl().getLowlevelResourceHash();
                if (m_scene.getDataSlot(slotHandle).attachedTexture != textureHash)
                {
                    m_scene.setDataSlotTexture(slotHandle, textureHash);
                    break;
                }
            }
        }

        return true;
    }

    bool SceneImpl::createTextureConsumer(const ramses::TextureSampler& sampler, dataConsumerId_t id)
    {
        if (sampler.impl().getTextureType() != ERamsesObjectType::Texture2D)
        {
            getErrorReporting().set("Scene::createTextureConsumer failed, only texture sampler using 2D texture can be used for linking..", *this);
            return false;
        }

        return createTextureConsumerImpl(sampler, id);
    }

    bool SceneImpl::createTextureConsumer(const TextureSamplerMS& sampler, dataConsumerId_t id)
    {
        return createTextureConsumerImpl(sampler, id);
    }

    bool SceneImpl::createTextureConsumer(const TextureSamplerExternal& sampler, dataConsumerId_t id)
    {
        // Allow duplicate consumer ID for external samplers (special need for ramses composer).
        // This will NOT work properly if consumers with same ID get to renderer side and attempt to be linked
        // (or if one is already linked and another one with same ID is created).
        // TODO vaclav make this proper, allow it in general and handle duplicates on renderer side

        return createTextureConsumerImpl(sampler, id, false);
    }

    template <typename SAMPLER>
    bool SceneImpl::createTextureConsumerImpl(const SAMPLER& sampler, dataConsumerId_t id, bool checkDuplicate)
    {
        if (!containsSceneObject(sampler.impl()))
        {
            getErrorReporting().set("Scene::createTextureConsumer failed, texture sampler is not from this scene.", *this);
            return false;
        }

        const ramses::internal::DataSlotId internalDataSlotId(id.getValue());
        if (checkDuplicate)
        {
            if (ramses::internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
            {
                getErrorReporting().set("Scene::createTextureConsumer failed, duplicate data slot id", *this);
                return false;
            }
        }

        const ramses::internal::TextureSamplerHandle& samplerHandle = sampler.impl().getTextureSamplerHandle();
        if (ramses::internal::DataSlotUtils::HasDataSlotIdForTextureSampler(m_scene, samplerHandle))
        {
            getErrorReporting().set("Scene::createTextureConsumer failed, texture sampler already has a data slot assigned", *this);
            return false;
        }

        m_scene.allocateDataSlot({ ramses::internal::EDataSlotType::TextureConsumer, internalDataSlotId, ramses::internal::NodeHandle(), ramses::internal::DataInstanceHandle::Invalid(), ramses::internal::ResourceContentHash::Invalid(), samplerHandle }, {});
        return true;
    }

    bool SceneImpl::setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds)
    {
        m_expirationTimestamp = ramses::internal::FlushTime::Clock::time_point(std::chrono::milliseconds{ ptpExpirationTimestampInMilliseconds });
        return true;
    }

    bool SceneImpl::flush(sceneVersionTag_t sceneVersion)
    {
        const auto timestampOfFlushCall = m_sendEffectTimeSync ? getIScene().getEffectTimeSync() :  ramses::internal::FlushTime::Clock::now();

        LOG_DEBUG(CONTEXT_CLIENT, "Scene::flush: sceneVersion {}, prevSceneVersion {}, syncFlushTime {}", sceneVersion, m_nextSceneVersion, ramses::internal::asMilliseconds(timestampOfFlushCall));

        if (m_nextSceneVersion != InvalidSceneVersionTag && sceneVersion == InvalidSceneVersionTag)
        {
            sceneVersion = m_nextSceneVersion;
            m_nextSceneVersion = InvalidSceneVersionTag;
        }

        const ramses::internal::SceneVersionTag sceneVersionInternal(sceneVersion);

        m_commandBuffer.execute(ramses::internal::SceneCommandVisitor(*this));
        applyHierarchicalVisibility();

        if (m_scene.haveResourcesChanged())
        {
            const auto maybeIssue = validateRenderBufferDependingObjects();
            if (maybeIssue)
            {
                getErrorReporting().set(fmt::format("Scene::flush: Flushing scene failed: {}", maybeIssue->message), maybeIssue->object);
                return false;
            }
        }

        const ramses::internal::FlushTimeInformation flushTimeInfo { m_expirationTimestamp, timestampOfFlushCall, ramses::internal::FlushTime::Clock::getClockType(), m_sendEffectTimeSync };
        m_sendEffectTimeSync = false;
        if (!getClientImpl().getClientApplication().flush(m_scene.getSceneId(), flushTimeInfo, sceneVersionInternal))
        {
            getErrorReporting().set("Scene::flush: Flushing scene failed, consult logs for more details.", *this);
            return false;
        }
        getStatisticCollection().statFlushesTriggered.incCounter(1);

        return true;
    }

    bool SceneImpl::resetUniformTimeMs()
    {
        const auto now   = ramses::internal::FlushTime::Clock::now();
        const auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        LOG_INFO(CONTEXT_CLIENT, "Scene({})::resetUniformTimeMs: {}", getSceneId(), nowMs.time_since_epoch().count());
        m_sendEffectTimeSync = true;
        getIScene().setEffectTimeSync(now);
        return true;
    }

    int32_t SceneImpl::getUniformTimeMs() const
    {
        return ramses::internal::EffectUniformTime::GetMilliseconds(getIScene().getEffectTimeSync());
    }

    SceneObjectRegistry& SceneImpl::getObjectRegistry()
    {
        return m_objectRegistry;
    }

    bool SceneImpl::containsSceneObject(const SceneObjectImpl& object) const
    {
        return &object.getSceneImpl() == this;
    }

    const SceneObject* SceneImpl::findObjectById(sceneObjectId_t id) const
    {
        return m_objectRegistry.findObjectById(id);
    }

    SceneObject* SceneImpl::findObjectById(sceneObjectId_t id)
    {
        return m_objectRegistry.findObjectById(id);
    }

    const SceneObjectRegistry& SceneImpl::getObjectRegistry() const
    {
        return m_objectRegistry;
    }

    template <typename OBJECT, typename CONTAINER>
    void SceneImpl::removeObjectFromAllContainers(const OBJECT& object)
    {
        const ERamsesObjectType type = TYPE_ID_OF_RAMSES_OBJECT<CONTAINER>::ID;
        SceneObjectRegistryIterator iterator(m_objectRegistry, type);
        CONTAINER* container = nullptr;
        while ((container = iterator.getNextNonConst<CONTAINER>()) != nullptr)
        {
            container->impl().removeIfContained(object.impl());
        }
    }

    void SceneImpl::removeAllDataSlotsForNode(const Node& node)
    {
        const ramses::internal::NodeHandle nodeHandle = node.impl().getNodeHandle();
        const uint32_t slotHandleCount = m_scene.getDataSlotCount();
        for (ramses::internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) && m_scene.getDataSlot(slotHandle).attachedNode == nodeHandle)
            {
                m_scene.releaseDataSlot(slotHandle);
            }
        }
    }

    ramses::RenderPass* SceneImpl::createRenderPassInternal(std::string_view name)
    {
        auto pimpl = std::make_unique<RenderPassImpl>(*this, name);
        pimpl->initializeFrameworkData();

        return &m_objectRegistry.createAndRegisterObject<ramses::RenderPass>(std::move(pimpl));
    }

    bool SceneImpl::cameraIsAssignedToRenderPasses(const ramses::Camera& camera)
    {
        SceneObjectRegistryIterator iterator(m_objectRegistry, ERamsesObjectType::RenderPass);
        const ramses::RenderPass* renderPass = nullptr;
        while ((renderPass = iterator.getNext<ramses::RenderPass>()) != nullptr)
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
                auto& meshNode = static_cast<MeshNodeImpl&>(node);
                const EVisibilityMode currentVisibility = meshNode.getFlattenedVisibility();

                if (currentVisibility != visibilityToApply)
                {
                    meshNode.setFlattenedVisibility(visibilityToApply);
                }
            }

            const size_t numberOfChildren = node.getChildCount();
            for (size_t i = 0; i < numberOfChildren; i++)
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

    std::optional<Issue> SceneImpl::validateRenderBufferDependingObjects() const
    {
        SceneObjectRegistryIterator itRT{ m_objectRegistry, ERamsesObjectType::RenderTarget };
        SceneObjectRegistryIterator itBP{ m_objectRegistry, ERamsesObjectType::BlitPass };

        ValidationReportImpl report;
        for (auto* registryIter : { &itRT, &itBP })
        {
            while (const auto* obj = registryIter->getNext())
            {
                obj->impl().onValidate(report);
                if (report.hasError())
                {
                    // get last error reported
                    const auto& issues = report.getIssues();
                    const auto it = std::find_if(issues.crbegin(), issues.crend(), [](const auto& issue) { return issue.type == EIssueType::Error; });
                    assert(it != issues.crend());
                    return *it;
                }
            }
        }

        return std::nullopt;
    }

    void SceneImpl::setSceneVersionForNextFlush(sceneVersionTag_t sceneVersion)
    {
        assert(m_nextSceneVersion == InvalidSceneVersionTag);
        m_nextSceneVersion = sceneVersion;
    }

    ArrayBuffer* SceneImpl::createArrayBuffer(ramses::EDataType dataType, size_t maxNumElements, std::string_view name)
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

    Texture2DBuffer* SceneImpl::createTexture2DBuffer(size_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, std::string_view name)
    {
        if (IsFormatCompressed(TextureUtils::GetTextureFormatInternal(textureFormat)))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture2DBuffer failed, cannot create texture buffers with compressed texture format.");
            return nullptr;
        }

        // More than one mips have size 1x1 -> error
        const size_t maxMipCount = ramses::internal::TextureMathUtils::GetMipLevelCount(width, height, 1u);
        if (mipLevels > maxMipCount)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture2DBuffer failed, mipLevels too large for the provided texture size.");
            return nullptr;
        }

        ramses::internal::MipMapDimensions mipMapSizes;

        uint32_t currentWidth = width;
        uint32_t currentHeight = height;
        for (size_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
        {
            mipMapSizes.push_back({ currentWidth, currentHeight });

            currentWidth = std::max<uint32_t>(1, currentWidth / 2);
            currentHeight = std::max<uint32_t>(1, currentHeight / 2);
        }

        auto pimpl = std::make_unique<Texture2DBufferImpl>(*this, name);
        pimpl->initializeFrameworkData(mipMapSizes, textureFormat);

        return &m_objectRegistry.createAndRegisterObject<Texture2DBuffer>(std::move(pimpl));
    }

    ramses::internal::StatisticCollectionScene& SceneImpl::getStatisticCollection()
    {
        return m_scene.getStatisticCollection();
    }

    ramses::SceneReference* SceneImpl::createSceneReference(sceneId_t referencedScene, std::string_view name)
    {
        if (!referencedScene.isValid())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createSceneReference: cannot reference a scene with invalid scene ID.");
            return nullptr;
        }

        if (referencedScene == getSceneId())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createSceneReference: cannot self reference.");
            return nullptr;
        }

        for (const auto& scene : getClientImpl().getListOfScenes())
        {
            if (getClientImpl().findSceneReference(scene->getSceneId(), referencedScene) != nullptr)
            {
                LOG_ERROR(CONTEXT_CLIENT, "Scene::createSceneReference: there is already a SceneReference with sceneId {} in master scene {}, cannot create another one",
                    referencedScene, scene->getSceneId());
                return nullptr;
            }
        }

        LOG_INFO(CONTEXT_CLIENT, "Scene::createSceneReference: creating scene reference (master {} / ref {})", getSceneId(), referencedScene);

        auto pimpl = std::make_unique<SceneReferenceImpl>(*this, name);
        pimpl->initializeFrameworkData(referencedScene);
        auto& sr = m_objectRegistry.createAndRegisterObject<ramses::SceneReference>(std::move(pimpl));
        m_sceneReferences.put(referencedScene, &sr);

        return &sr;
    }

    bool SceneImpl::linkData(ramses::SceneReference* providerReference, dataProviderId_t providerId, ramses::SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        if (!providerReference && !consumerReference)
        {
            getErrorReporting().set("Scene::linkData: can't link an object to another object in the same scene", *this);
            return false;
        }

        if (consumerReference == providerReference)
        {
            getErrorReporting().set("Scene::linkData: can't link an object to another object in the same scene reference", *this);
            return false;
        }

        if ((providerReference != nullptr && providerReference->impl().getSceneImpl().getSceneId() != getSceneId()) ||
            (consumerReference != nullptr && consumerReference->impl().getSceneImpl().getSceneId() != getSceneId()))
        {
            getErrorReporting().set("Scene::linkData: can't link to object of a scene reference with a different master scene", *this);
            return false;
        }

        if (providerReference && providerReference->impl().getReportedState() < RendererSceneState::Ready)
        {
            getErrorReporting().set("Scene::linkData: Provider SceneReference state has to be at least Ready", *this);
            return false;
        }

        if (consumerReference && consumerReference->impl().getReportedState() < RendererSceneState::Ready)
        {
            getErrorReporting().set("Scene::linkData: Consumer SceneReference state has to be at least Ready", *this);
            return false;
        }

        const auto providerScene = (providerReference ? providerReference->impl().getSceneReferenceHandle() : ramses::internal::SceneReferenceHandle{});
        const auto consumerScene = (consumerReference ? consumerReference->impl().getSceneReferenceHandle() : ramses::internal::SceneReferenceHandle{});
        getIScene().linkData(providerScene, ramses::internal::DataSlotId{ providerId.getValue() }, consumerScene, ramses::internal::DataSlotId{ consumerId.getValue() });

        return true;
    }

    bool SceneImpl::unlinkData(ramses::SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        if (consumerReference != nullptr && consumerReference->impl().getSceneImpl().getSceneId() != getSceneId())
        {
            getErrorReporting().set("Scene::unlinkData: can't unlink object of a scene reference with a different master scene");
            return false;
        }

        const auto consumerScene = (consumerReference ? consumerReference->impl().getSceneReferenceHandle() : ramses::internal::SceneReferenceHandle{});
        getIScene().unlinkData(consumerScene, ramses::internal::DataSlotId{ consumerId.getValue() });

        return true;
    }

    ramses::SceneReference* SceneImpl::getSceneReference(sceneId_t referencedSceneId)
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
    ramses::ArrayResource* SceneImpl::createArrayResource(size_t numElements, const T* arrayData, std::string_view name)
    {
        if (0u == numElements || nullptr == arrayData)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createArrayResource: Array resource must have element count > 0 and data must not be nullptr!");
            return nullptr;
        }

        ramses::internal::ManagedResource res = getClientImpl().createManagedArrayResource(numElements, GetEDataType<T>(), arrayData, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createArrayResource: failed to create managed array resource");
            return nullptr;
        }
        return createHLArrayResource(res, name);
    }

    ramses::ArrayResource* SceneImpl::createHLArrayResource(ramses::internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses::internal::EResourceType::IndexArray ||
               resource->getTypeID() == ramses::internal::EResourceType::VertexArray);

        const auto arrayRes = resource->convertTo<ramses::internal::ArrayResource>();
        ramses::internal::ResourceHashUsage usage = getClientImpl().getClientApplication().getHashUsage(arrayRes->getHash());

        auto pimpl = std::make_unique<ArrayResourceImpl>(usage, *this, name);
        pimpl->initializeFromFrameworkData(arrayRes->getElementCount(), DataTypeUtils::ConvertDataTypeFromInternal(arrayRes->getElementType()));

        return &registerCreatedResourceObject<ramses::ArrayResource>(std::move(pimpl));
    }

    Texture2D* SceneImpl::createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, const std::vector<MipLevelData>& mipLevelData, bool generateMipChain, const TextureSwizzle& swizzle, std::string_view name)
    {
        ramses::internal::ManagedResource res = getClientImpl().createManagedTexture(
            ramses::internal::EResourceType::Texture2D, width, height, 1u, format, mipLevelData, generateMipChain, swizzle, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture2D: failed to create managed Texture2D resource");
            return nullptr;
        }
        return createHLTexture2D(res, name);
    }

    Texture2D* SceneImpl::createHLTexture2D(ramses::internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses::internal::EResourceType::Texture2D);

        const auto texRes = resource->convertTo<ramses::internal::TextureResource>();
        ramses::internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<Texture2DImpl>(hashUsage, *this, name);
        pimpl->initializeFromFrameworkData(texRes->getWidth(), texRes->getHeight(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()),
            TextureUtils::GetTextureSwizzleFromInternal(texRes->getTextureSwizzle()));

        return &registerCreatedResourceObject<Texture2D>(std::move(pimpl));
    }

    Texture3D* SceneImpl::createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, const std::vector<MipLevelData>& mipLevelData, bool generateMipChain, std::string_view name)
    {
        ramses::internal::ManagedResource res = getClientImpl().createManagedTexture(
            ramses::internal::EResourceType::Texture3D, width, height, depth, format, mipLevelData, generateMipChain, {}, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture3D: failed to create managed Texture3D resource");
            return nullptr;
        }
        return createHLTexture3D(res, name);
    }

    Texture3D* SceneImpl::createHLTexture3D(ramses::internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses::internal::EResourceType::Texture3D);
        const auto texRes = resource->convertTo<ramses::internal::TextureResource>();
        ramses::internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<Texture3DImpl>(hashUsage, *this, name);
        pimpl->initializeFromFrameworkData(texRes->getWidth(), texRes->getHeight(), texRes->getDepth(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()));

        return &registerCreatedResourceObject<Texture3D>(std::move(pimpl));
    }

    TextureCube* SceneImpl::createTextureCube(uint32_t size, ETextureFormat format, const std::vector<CubeMipLevelData>& mipLevelData, bool generateMipChain, const TextureSwizzle& swizzle, std::string_view name)
    {
        ramses::internal::ManagedResource res = getClientImpl().createManagedTexture(
            ramses::internal::EResourceType::TextureCube, size, 1u, 1u, format, mipLevelData, generateMipChain, swizzle, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureCube: failed to create managed TextureCube resource");
            return nullptr;
        }
        return createHLTextureCube(res, name);
    }

    TextureCube* SceneImpl::createHLTextureCube(ramses::internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses::internal::EResourceType::TextureCube);

        const auto texRes = resource->convertTo<ramses::internal::TextureResource>();
        ramses::internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<TextureCubeImpl>(hashUsage, *this, name);
        pimpl->initializeFromFrameworkData(texRes->getWidth(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()),
            TextureUtils::GetTextureSwizzleFromInternal(texRes->getTextureSwizzle()));

        return &registerCreatedResourceObject<TextureCube>(std::move(pimpl));
    }

    Effect* SceneImpl::createEffect(const EffectDescription& effectDesc, std::string_view name)
    {
        ramses::internal::ManagedResource res = getClientImpl().createManagedEffect(effectDesc, name, m_effectErrorMessages);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createEffect: failed to create managed effect resource: {}", m_effectErrorMessages);
            return nullptr;
        }

        return createHLEffect(res, name);
    }

    Effect* SceneImpl::createHLEffect(ramses::internal::ManagedResource const& resource, std::string_view name)
    {
        assert(resource->getTypeID() == ramses::internal::EResourceType::Effect);

        const auto effectRes = resource->convertTo<ramses::internal::EffectResource>();
        ramses::internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        auto pimpl = std::make_unique<EffectImpl>(hashUsage, *this, name);
        pimpl->initializeFromFrameworkData(effectRes->getUniformInputs(), effectRes->getAttributeInputs(), effectRes->getGeometryShaderInputType());

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

    Resource* SceneImpl::scanForResourceWithHash(ramses::internal::ResourceContentHash hash) const
    {
        for (const auto& res : m_resources)
        {
            if (hash == res.second->impl().getLowlevelResourceHash())
                return res.second;
        }

        return nullptr;
    }

    bool SceneImpl::writeSceneObjectsToStream(ramses::internal::IOutputStream& outputStream, const SaveFileConfigImpl& saveConfig) const
    {
        ramses::internal::ScenePersistation::WriteSceneMetadataToStream(outputStream, getIScene());
        ramses::internal::ScenePersistation::WriteSceneToStream(outputStream, getIScene());

        SerializationContext serializationContext{saveConfig};
        return serialize(outputStream, serializationContext);
    }

    bool SceneImpl::serialize(std::vector<std::byte>& outputBuffer, const SaveFileConfigImpl& config) const
    {
        ramses::internal::BinaryOutputStream outputStream;
        const EFeatureLevel featureLevel = m_hlClient.impl().getFramework().getFeatureLevel();
        ramses::internal::RamsesVersion::WriteToStream(outputStream, ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, featureLevel);
        outputStream << config.getExporterVersion();
        outputStream << config.getMetadataString();

        const auto headerOffset = outputStream.getSize();

        // reserve space for offset to SceneObjects and LL-Objects
        outputStream << static_cast<uint64_t>(0);
        outputStream << static_cast<uint64_t>(0);
        const uint64_t offsetSceneObjectsStart = outputStream.getSize();
        const auto status = writeSceneObjectsToStream(outputStream, config);

        const auto offsetLLResourcesStart = outputStream.getSize();
        ResourceObjects resources;
        resources.reserve(m_resources.size());
        for (auto const& res : m_resources)
            resources.push_back(res.second);
        getClientImpl().writeLowLevelResourcesToStream(resources, outputStream, config.getCompressionEnabled());

        outputBuffer = outputStream.release();
        outputStream << static_cast<uint64_t>(offsetSceneObjectsStart);
        outputStream << static_cast<uint64_t>(offsetLLResourcesStart);
        const auto offsets = outputStream.release();

        assert(offsets.size() == 2*sizeof(uint64_t));
        std::copy(offsets.begin(), offsets.end(), outputBuffer.begin() + static_cast<ptrdiff_t>(headerOffset));

        return status;
    }

    bool SceneImpl::saveToFile(std::string_view fileName, const SaveFileConfigImpl& config)
    {
        if (fileName.empty())
        {
            getErrorReporting().set("Scene::saveToFile failed: empty filename", *this);
            return false;
        }

        LOG_INFO(CONTEXT_CLIENT, "Scene::saveToFile: filename '{}', compress {}", fileName, config.getCompressionEnabled());

        LOG_INFO(CONTEXT_CLIENT, "Scene::saveToFile: updating LogicEngine instances before saving to file");
        SceneObjectRegistryIterator leIter{ m_objectRegistry, ramses::ERamsesObjectType::LogicEngine };
        while (auto le = leIter.getNextNonConst<ramses::LogicEngine>())
        {
            if (!le->update())
            {
                getErrorReporting().set(fmt::format("Scene::saveToFile failed due to failed logic engine update: {}", getErrorReporting().getError()->message), le);
                return false;
            }
        }

        LOG_INFO(CONTEXT_CLIENT, "Scene::saveToFile: flushing scene before saving to file");
        if (!flush(sceneVersionTag_t{}))
        {
            getErrorReporting().set(fmt::format("Scene::saveToFile failed due to failed scene flush: {}", getErrorReporting().getError()->message), *this);
            return false;
        }

        std::vector<std::byte> outputBuffer;
        if (!serialize(outputBuffer, config))
            return false;

        ramses::internal::File outputFile(fileName);
        if (!outputFile.open(ramses::internal::File::Mode::WriteNewBinary))
        {
            getErrorReporting().set(fmt::format("Scene::saveToFile failed, could not open file for writing: '{}'", fileName), *this);
            return false;
        }

        if (!outputFile.write(outputBuffer.data(), outputBuffer.size()))
        {
            getErrorReporting().set(fmt::format("Scene::saveToFile failed, write failed: '{}'", fileName), *this);
            return false;
        }

        if (!outputFile.close())
        {
            getErrorReporting().set(fmt::format("Scene::saveToFile failed, close file failed: '{}'", fileName), *this);
            return false;
        }

        LOG_INFO(CONTEXT_CLIENT, "Scene::saveToFile: done writing '{}'", fileName);

        return true;
    }

    void SceneImpl::setSceneFileHandle(ramses::internal::SceneFileHandle handle)
    {
        m_sceneFileHandle = handle;
    }

    void SceneImpl::closeSceneFile()
    {
        if (!m_sceneFileHandle.isValid())
            return;

        getClientImpl().getClientApplication().removeResourceFile(m_sceneFileHandle);
        LOG_INFO(CONTEXT_CLIENT, "SceneImpl::closeSceneFile closed: {}", m_sceneFileHandle);
        m_sceneFileHandle = ramses::internal::SceneFileHandle::Invalid();
    }

    ramses::internal::SceneFileHandle SceneImpl::getSceneFileHandle() const
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

    template ramses::ArrayResource* SceneImpl::createArrayResource<uint16_t>(size_t, const uint16_t*, std::string_view);
    template ramses::ArrayResource* SceneImpl::createArrayResource<uint32_t>(size_t, const uint32_t*, std::string_view);
    template ramses::ArrayResource* SceneImpl::createArrayResource<float>(size_t, const float*, std::string_view);
    template ramses::ArrayResource* SceneImpl::createArrayResource<vec2f>(size_t, const vec2f*, std::string_view);
    template ramses::ArrayResource* SceneImpl::createArrayResource<vec3f>(size_t, const vec3f*, std::string_view);
    template ramses::ArrayResource* SceneImpl::createArrayResource<vec4f>(size_t, const vec4f*, std::string_view);
    template ramses::ArrayResource* SceneImpl::createArrayResource<std::byte>(size_t, const std::byte*, std::string_view);
}
