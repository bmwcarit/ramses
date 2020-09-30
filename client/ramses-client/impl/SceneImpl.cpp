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
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/AnimationSystemEnums.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EScenePublicationMode.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4i.h"
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
#include "StreamTextureImpl.h"
#include "AnimationSystemImpl.h"
#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/ActionCollectingAnimationSystem.h"
#include "Animation/AnimationSystemFactory.h"
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

#include "Resource/ArrayResource.h"
#include "Resource/TextureResource.h"
#include "Resource/EffectResource.h"

#include "Components/FlushTimeInformation.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/TextureMathUtils.h"
#include "ResourceDataPoolImpl.h"

#include <array>

namespace ramses
{
    SceneImpl::SceneImpl(ramses_internal::ClientScene& scene, const SceneConfigImpl& sceneConfig, RamsesClient& ramsesClient)
        : ClientObjectImpl(ramsesClient.impl, ERamsesObjectType_Scene, scene.getName().c_str())
        , m_scene(scene)
        , m_nextSceneVersion(InvalidSceneVersionTag)
        , m_futurePublicationMode(sceneConfig.getPublicationMode())
        , m_hlClient(ramsesClient)
    {
        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Scene::Scene: sceneId " << scene.getSceneId()  <<
                 ", publicationMode " << (sceneConfig.getPublicationMode() == EScenePublicationMode_LocalAndRemote ? "LocalAndRemote" : "LocalOnly"));
        getClientImpl().getFramework().getPeriodicLogger().registerStatisticCollectionScene(m_scene.getSceneId(), m_scene.getStatisticCollection());
        const bool enableLocalOnlyOptimization = sceneConfig.getPublicationMode() == EScenePublicationMode_LocalOnly;
        getClientImpl().getClientApplication().createScene(scene, enableLocalOnlyOptimization);
    }

    SceneImpl::~SceneImpl()
    {
        RamsesObjectVector objects;
        m_objectRegistry.getObjectsOfType(objects, ERamsesObjectType_SceneObject);
        for (const auto it : objects)
        {
            delete &RamsesObjectTypeUtils::ConvertTo<SceneObject>(*it);
        }

        // hl resources are gone, so close file and therefor release unneeded ll resources
        if (!m_sceneFilename.empty())
            getClientImpl().getClientApplication().removeResourceFile(m_sceneFilename.c_str());

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

    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, const char*>::value, T>::type* = nullptr>
    T& createImplHelper(SceneImpl& scene, ERamsesObjectType)
    {
        return *new T(scene, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, SceneImpl&, ERamsesObjectType, const char*>::value, T>::type* = nullptr>
    T& createImplHelper(SceneImpl& scene, ERamsesObjectType type)
    {
        return *new T(scene, type, "");
    }
    template <class T, typename std::enable_if<std::is_constructible<T, ramses_internal::ResourceHashUsage, SceneImpl&, const char*>::value, T>::type* = nullptr>
    T& createImplHelper(SceneImpl& scene, ERamsesObjectType)
    {
        return *new T({}, scene, "");
    }

    template <typename ObjectType, typename ObjectImplType>
    status_t SceneImpl::createAndDeserializeObjectImpls(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext, uint32_t count)
    {
        for (uint32_t i = 0u; i < count; ++i)
        {
            ObjectImplType& impl = createImplHelper<ObjectImplType>(*this, TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID);
            ObjectIDType objectID = SerializationHelper::DeserializeObjectID(inStream);
            auto status = impl.deserialize(inStream, serializationContext);
            if (status != StatusOK)
            {
                delete &impl;
                return status;
            }
            ObjectType* object = new ObjectType(impl);
            m_objectRegistry.addObject(*object);

            if (auto resource = RamsesUtils::TryConvert<Resource>(*object))
                m_resourcesById[resource->getResourceId()] = resource;

            if (!serializationContext.registerObjectImpl(&impl, objectID))
            {
                delete object;
                return addErrorEntry("Deserialization of object failed, object data serialized with wrong ID or data in file corrupted.");
            }
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

        std::array<uint32_t, ERamsesObjectType_NUMBER_OF_TYPES> objectCounts = {};

        for (uint32_t i = 0u; i < typesCount; ++i)
        {
            uint32_t count = 0u;
            const ERamsesObjectType type = SerializationHelper::DeserializeObjectTypeAndCount(inStream, count);
            assert(m_objectRegistry.getNumberOfObjects(type) == 0u);
            m_objectRegistry.reserveAdditionalObjectCapacity(type, count);
            objectCounts[type] = count;

            status_t status = StatusOK;
            switch (type)
            {
            case ERamsesObjectType_Node:
                status = createAndDeserializeObjectImpls<Node, NodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_MeshNode:
                status = createAndDeserializeObjectImpls<MeshNode, MeshNodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_RemoteCamera:
                status = createAndDeserializeObjectImpls<RemoteCamera, CameraNodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_PerspectiveCamera:
                status = createAndDeserializeObjectImpls<PerspectiveCamera, CameraNodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_OrthographicCamera:
                status = createAndDeserializeObjectImpls<OrthographicCamera, CameraNodeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_Appearance:
                status = createAndDeserializeObjectImpls<Appearance, AppearanceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_GeometryBinding:
                status = createAndDeserializeObjectImpls<GeometryBinding, GeometryBindingImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_StreamTexture:
                status = createAndDeserializeObjectImpls<StreamTexture, StreamTextureImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_AnimationSystem:
                status = createAndDeserializeObjectImpls<AnimationSystem, AnimationSystemImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_AnimationSystemRealTime:
                status = createAndDeserializeObjectImpls<AnimationSystemRealTime, AnimationSystemImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_RenderGroup:
                status = createAndDeserializeObjectImpls<RenderGroup, RenderGroupImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_RenderPass:
                status = createAndDeserializeObjectImpls<RenderPass, RenderPassImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_BlitPass:
                status = createAndDeserializeObjectImpls<BlitPass, BlitPassImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_PickableObject:
                status = createAndDeserializeObjectImpls<PickableObject, PickableObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_SceneReference:
                status = createAndDeserializeObjectImpls<SceneReference, SceneReferenceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_RenderBuffer:
                status = createAndDeserializeObjectImpls<RenderBuffer, RenderBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_RenderTarget:
                status = createAndDeserializeObjectImpls<RenderTarget, RenderTargetImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_TextureSampler:
                status = createAndDeserializeObjectImpls<TextureSampler, TextureSamplerImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataFloat:
                status = createAndDeserializeObjectImpls<DataFloat, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataVector2f:
                status = createAndDeserializeObjectImpls<DataVector2f, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataVector3f:
                status = createAndDeserializeObjectImpls<DataVector3f, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataVector4f:
                status = createAndDeserializeObjectImpls<DataVector4f, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataMatrix22f:
                status = createAndDeserializeObjectImpls<DataMatrix22f, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataMatrix33f:
                status = createAndDeserializeObjectImpls<DataMatrix33f, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataMatrix44f:
                status = createAndDeserializeObjectImpls<DataMatrix44f, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataInt32:
                status = createAndDeserializeObjectImpls<DataInt32, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataVector2i:
                status = createAndDeserializeObjectImpls<DataVector2i, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataVector3i:
                status = createAndDeserializeObjectImpls<DataVector3i, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataVector4i:
                status = createAndDeserializeObjectImpls<DataVector4i, DataObjectImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_DataBufferObject:
                status = createAndDeserializeObjectImpls<ArrayBuffer, ArrayBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_Texture2DBuffer:
                status = createAndDeserializeObjectImpls<Texture2DBuffer, Texture2DBufferImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_ArrayResource:
                status = createAndDeserializeObjectImpls<ArrayResource, ArrayResourceImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_Texture2D:
                status = createAndDeserializeObjectImpls<Texture2D, Texture2DImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_Texture3D:
                status = createAndDeserializeObjectImpls<Texture3D, Texture3DImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_TextureCube:
                status = createAndDeserializeObjectImpls<TextureCube, TextureCubeImpl>(inStream, serializationContext, count);
                break;
            case ERamsesObjectType_Effect:
                status = createAndDeserializeObjectImpls<Effect, EffectImpl>(inStream, serializationContext, count);
                break;

            default:
                return addErrorEntry("Scene::deserialize failed, unexpected object type in file stream.");
            }

            CHECK_RETURN_ERR(status);
        }

        inStream >> m_lastSceneObjectId.getReference();

        LOG_DEBUG_F(ramses_internal::CONTEXT_PROFILING, ([&](ramses_internal::StringOutputStream& sos) {
                    sos << "SceneImpl::deserialize: HL scene object counts for SceneID " << m_scene.getSceneId() << "\n";
                    for (uint32_t i = 0; i < ERamsesObjectType_NUMBER_OF_TYPES; i++)
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

    status_t SceneImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = ClientObjectImpl::validate(indent, visitedObjects);
        indent += IndentationStep;

        uint32_t objectCount[ERamsesObjectType_NUMBER_OF_TYPES];
        for (uint32_t i = 0u; i < ERamsesObjectType_NUMBER_OF_TYPES; ++i)
        {
            const ERamsesObjectType type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType_SceneObject)
                && !RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType_AnimationObject)
                && RamsesObjectTypeUtils::IsConcreteType(type))
            {
                objectCount[i] = 0u;
                RamsesObjectRegistryIterator iter(getObjectRegistry(), ERamsesObjectType(i));
                while (const RamsesObject* obj = iter.getNext())
                {
                    if (addValidationOfDependentObject(indent, obj->impl, visitedObjects) != StatusOK)
                    {
                        status = getValidationErrorStatus();
                    }
                    ++objectCount[i];
                }
            }
        }

        for (uint32_t i = 0u; i < ERamsesObjectType_NUMBER_OF_TYPES; ++i)
        {
            const ERamsesObjectType type = static_cast<ERamsesObjectType>(i);
            if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType_SceneObject)
                && !RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ERamsesObjectType_AnimationObject)
                && RamsesObjectTypeUtils::IsConcreteType(type))
            {
                ramses_internal::StringOutputStream msg;
                msg << "Number of " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(type) << " instances: " << objectCount[i];
                addValidationMessage(EValidationSeverity_Info, indent, msg.c_str());
            }
        }

        return status;
    }

    RemoteCamera* SceneImpl::createRemoteCamera(const char* name)
    {
        CameraNodeImpl& pimpl = *new CameraNodeImpl(*this, ERamsesObjectType_RemoteCamera, name);
        pimpl.initializeFrameworkData();
        RemoteCamera* newCamera = new RemoteCamera(pimpl);
        registerCreatedObject(*newCamera);

        return newCamera;
    }

    PerspectiveCamera* SceneImpl::createPerspectiveCamera(const char* name)
    {
        CameraNodeImpl& pimpl = *new CameraNodeImpl(*this, ERamsesObjectType_PerspectiveCamera, name);
        pimpl.initializeFrameworkData();
        PerspectiveCamera* newCamera = new PerspectiveCamera(pimpl);
        registerCreatedObject(*newCamera);

        return newCamera;
    }

    OrthographicCamera* SceneImpl::createOrthographicCamera(const char* name)
    {
        CameraNodeImpl& pimpl = *new CameraNodeImpl(*this, ERamsesObjectType_OrthographicCamera, name);
        pimpl.initializeFrameworkData();
        OrthographicCamera* newCamera = new OrthographicCamera(pimpl);
        registerCreatedObject(*newCamera);

        return newCamera;
    }

    Appearance* SceneImpl::createAppearance(const Effect& effect, const char* name)
    {
        if (this != &effect.impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createAppearance failed, effect is not from this scene.");
            return nullptr;
        }

        AppearanceImpl* pimpl = createAppearanceImpl(name);
        pimpl->initializeFrameworkData(effect.impl);
        Appearance* appearance = new Appearance(*pimpl);
        registerCreatedObject(*appearance);

        return appearance;
    }

    AppearanceImpl* SceneImpl::createAppearanceImpl(const char* name)
    {
        return new AppearanceImpl(*this, name);
    }

    StreamTexture* SceneImpl::createStreamTexture(const Texture2D& fallbackTexture, waylandIviSurfaceId_t source, const char* name)
    {
        if (this != &fallbackTexture.impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createStreamTexture failed, fallbackTexture is not from this scene.");
            return nullptr;
        }

        StreamTextureImpl& pimpl = *new StreamTextureImpl( *this, name);
        pimpl.initializeFrameworkData(source, fallbackTexture.impl);
        StreamTexture* texture = new StreamTexture(pimpl);
        registerCreatedObject(*texture);

        return texture;
    }

    GeometryBinding* SceneImpl::createGeometryBinding(const Effect& effect, const char* name)
    {
        if (this != &effect.impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createGeometryBinding failed, effect is not from this scene.");
            return nullptr;
        }

        GeometryBindingImpl& pimpl = *new GeometryBindingImpl(*this, name);
        pimpl.initializeFrameworkData(effect.impl);
        GeometryBinding* geometry = new GeometryBinding(pimpl);
        registerCreatedObject(*geometry);

        return geometry;
    }

    status_t SceneImpl::destroy(SceneObject& object)
    {
        if (!containsSceneObject(object.impl))
        {
            return addErrorEntry("Scene::destroy failed, object is not in this scene.");
        }

        status_t returnStatus = StatusOK;
        switch (object.getType())
        {
        case ERamsesObjectType_RenderTarget:
            returnStatus = destroyRenderTarget(RamsesObjectTypeUtils::ConvertTo<RenderTarget>(object));
            break;
        case ERamsesObjectType_RemoteCamera:
        case ERamsesObjectType_OrthographicCamera:
        case ERamsesObjectType_PerspectiveCamera:
            returnStatus = destroyCamera(RamsesObjectTypeUtils::ConvertTo<Camera>(object));
            break;
        case ERamsesObjectType_RenderGroup:
            returnStatus = destroyRenderGroup(RamsesObjectTypeUtils::ConvertTo<RenderGroup>(object));
            break;
        case ERamsesObjectType_Node:
        case ERamsesObjectType_PickableObject:
            returnStatus = destroyNode(RamsesObjectTypeUtils::ConvertTo<Node>(object));
            break;
        case ERamsesObjectType_MeshNode:
            returnStatus = destroyMeshNode(RamsesObjectTypeUtils::ConvertTo<MeshNode>(object));
            break;
        case ERamsesObjectType_AnimationSystem:
        case ERamsesObjectType_AnimationSystemRealTime:
            returnStatus = destroyAnimationSystem(RamsesObjectTypeUtils::ConvertTo<AnimationSystem>(object));
            break;
        case ERamsesObjectType_DataFloat:
        case ERamsesObjectType_DataVector2f:
        case ERamsesObjectType_DataVector3f:
        case ERamsesObjectType_DataVector4f:
        case ERamsesObjectType_DataMatrix22f:
        case ERamsesObjectType_DataMatrix33f:
        case ERamsesObjectType_DataMatrix44f:
        case ERamsesObjectType_DataInt32:
        case ERamsesObjectType_DataVector2i:
        case ERamsesObjectType_DataVector3i:
        case ERamsesObjectType_DataVector4i:
            returnStatus = destroyDataObject(RamsesObjectTypeUtils::ConvertTo<DataObject>(object));
            break;
        case ERamsesObjectType_TextureSampler:
            destroyTextureSampler(RamsesObjectTypeUtils::ConvertTo<TextureSampler>(object));
            break;
        case ERamsesObjectType_Appearance:
        case ERamsesObjectType_GeometryBinding:
        case ERamsesObjectType_RenderPass:
        case ERamsesObjectType_BlitPass:
        case ERamsesObjectType_RenderBuffer:
        case ERamsesObjectType_StreamTexture:
        case ERamsesObjectType_DataBufferObject:
        case ERamsesObjectType_Texture2DBuffer:
            returnStatus = destroyObject(object);
            break;
        case ERamsesObjectType_SceneReference:
            m_sceneReferences.remove(RamsesObjectTypeUtils::ConvertTo<SceneReference>(object).getReferencedSceneId());
            returnStatus = destroyObject(object);
            break;
        case ERamsesObjectType_Texture2D:
        case ERamsesObjectType_Texture3D:
        case ERamsesObjectType_TextureCube:
        case ERamsesObjectType_Effect:
        case ERamsesObjectType_ArrayResource:
            returnStatus = destroyResource(RamsesObjectTypeUtils::ConvertTo<Resource>(object));
            break;
        default:
            assert(false);
            returnStatus = addErrorEntry("Scene::destroy internal error, cannot destroy object!");
            break;
        }

        return returnStatus;
    }

    status_t SceneImpl::destroyRenderTarget(RenderTarget& renderTarget)
    {
        RamsesObjectRegistryIterator iterator(m_objectRegistry, ERamsesObjectType_RenderPass);
        const RenderPass* renderPass = nullptr;
        while ((renderPass = iterator.getNext<RenderPass>()) != nullptr)
        {
            if (&renderTarget == renderPass->impl.getRenderTarget())
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
            m_objectRegistry.setNodeDirty(node.impl.getChildImpl(i), true);
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
        const ramses_internal::DataInstanceHandle dataRef = dataObject.impl.getDataReference();
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

    status_t SceneImpl::destroyTextureSampler(TextureSampler& sampler)
    {
        const ramses_internal::TextureSamplerHandle& samplerHandle = sampler.impl.getTextureSamplerHandle();
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
        const resourceId_t resId = resource.impl.getResourceId();
        m_resourcesById.remove(resId);

        getClientImpl().onResourceDestroyed(resource);

        return destroyObject(resource);
    }

    status_t SceneImpl::destroyObject(SceneObject& object)
    {
        object.impl.deinitializeFrameworkData();
        m_objectRegistry.removeObject(object);
        delete &object;
        return StatusOK;
    }

    status_t SceneImpl::publish(EScenePublicationMode requestedPublicationMode)
    {
        if (isPublished())
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::publish: ignored, scene is already published").c_str());
        }
        if (requestedPublicationMode == EScenePublicationMode_LocalAndRemote && m_futurePublicationMode == EScenePublicationMode_LocalOnly)
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::publish: Enabled local only optimisations from SceneConfig, cannot remote publish later").c_str());
        }
        if (requestedPublicationMode != EScenePublicationMode_LocalOnly && !getClientImpl().getFramework().isConnected())
        {
            return addErrorEntry((ramses_internal::StringOutputStream() << "Scene(" << m_scene.getSceneId() << ")::publish: failed, have to connect the framework first using RamsesFramework::connect").c_str());
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

    Node* SceneImpl::createNode(const char* name)
    {
        NodeImpl* pimpl = new NodeImpl(*this, ERamsesObjectType_Node, name);
        pimpl->initializeFrameworkData();
        Node* newNode = new Node(*pimpl);
        registerCreatedObject(*newNode);

        return newNode;
    }

    MeshNode* SceneImpl::createMeshNode(const char* name)
    {
        MeshNodeImpl* pimpl = new MeshNodeImpl(*this, name);
        pimpl->initializeFrameworkData();
        MeshNode* newNode = new MeshNode(*pimpl);
        registerCreatedObject(*newNode);

        return newNode;
    }

    RenderGroup* SceneImpl::createRenderGroup(const char* name /*= 0*/)
    {
        RenderGroupImpl& pimpl = *new RenderGroupImpl(*this, name);
        pimpl.initializeFrameworkData();
        RenderGroup* renderGroup = new RenderGroup(pimpl);
        registerCreatedObject(*renderGroup);

        return renderGroup;
    }

    RenderPass* SceneImpl::createRenderPass(const char* name /*= 0*/)
    {
        return createRenderPassInternal(name);
    }

    BlitPass* SceneImpl::createBlitPass(const RenderBuffer& sourceRenderBuffer, const RenderBuffer& destinationRenderBuffer, const char* name)
    {
        if (!containsSceneObject(sourceRenderBuffer.impl))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createBlitPass failed, source render buffer is not from this scene.");
            return nullptr;
        }

        if (!containsSceneObject(destinationRenderBuffer.impl))
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

        BlitPassImpl& pimpl = *new BlitPassImpl(*this, name);
        pimpl.initializeFrameworkData(sourceRenderBuffer.impl, destinationRenderBuffer.impl);
        BlitPass* blitPass = new BlitPass(pimpl);
        registerCreatedObject(*blitPass);

        return blitPass;
    }

    PickableObject* SceneImpl::createPickableObject(const ArrayBuffer& geometryBuffer, const pickableObjectId_t id, const char* name)
    {
        if (!containsSceneObject(geometryBuffer.impl))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                      "Scene(" << m_scene.getSceneId()
                               << ")::createPickableObject failed, geometry buffer is not from this scene.");
            return nullptr;
        }

        if (geometryBuffer.getDataType() != EDataType::Vector3F || 0 != (geometryBuffer.impl.getElementCount() % 3))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                      "Scene(" << m_scene.getSceneId()
                               << ")::createPickableObject failed, geometry buffer has the wrong format.");
            return nullptr;
        }

        PickableObjectImpl& pimpl = *new PickableObjectImpl(*this, name);
        pimpl.initializeFrameworkData(geometryBuffer.impl, id);
        PickableObject* pickableObject = new PickableObject(pimpl);
        registerCreatedObject(*pickableObject);

        return pickableObject;
    }

    RenderBuffer* SceneImpl::createRenderBuffer(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount, const char* name)
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

        if (ERenderBufferAccessMode_WriteOnly != accessMode && 0u != sampleCount)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene(" << m_scene.getSceneId() << ")::createRenderBuffer failed: can not create render buffer with read/write access mode and MSAA sampleCount other than Zero!");
            return nullptr;
        }

        RenderBufferImpl& pimpl = *new RenderBufferImpl(*this, name);
        pimpl.initializeFrameworkData(width, height, bufferType, bufferFormat, accessMode, sampleCount);
        RenderBuffer* buffer = new RenderBuffer(pimpl);
        registerCreatedObject(*buffer);

        return buffer;
    }

    RenderTarget* SceneImpl::createRenderTarget(const RenderTargetDescriptionImpl& rtDesc, const char* name)
    {
        StatusObjectSet visitedObjects;
        if (rtDesc.validate(0u, visitedObjects) != StatusOK)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createRenderTarget failed, RenderTargetDescription is invalid.");
            return nullptr;
        }

        RenderTargetImpl& pimpl = *new RenderTargetImpl(*this, name);
        pimpl.initializeFrameworkData(rtDesc);
        RenderTarget* rt = new RenderTarget(pimpl);
        registerCreatedObject(*rt);

        return rt;
    }

    TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        const Texture2D& texture,
        const char* name /*= 0*/)
    {
        if (this != &texture.impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture 2D is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode_Clamp,
            minSamplingMethod,
            magSamplingMethod,
            anisotropyLevel,
            ERamsesObjectType_Texture2D,
            ramses_internal::TextureSampler::ContentType::ClientTexture,
            texture.impl.getLowlevelResourceHash(),
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
        const char* name /*= 0*/)
    {
        if (this != &texture.impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture 3D is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, wrapRMode, minSamplingMethod, magSamplingMethod, 1u,
            ERamsesObjectType_Texture3D,
            ramses_internal::TextureSampler::ContentType::ClientTexture,
            texture.impl.getLowlevelResourceHash(),
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
        const char* name /*= 0*/)
    {
        if (this != &texture.impl.getSceneImpl())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture Cube is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode_Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType_TextureCube,
            ramses_internal::TextureSampler::ContentType::ClientTexture,
            texture.impl.getLowlevelResourceHash(),
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
        const char* name)
    {
        if (!containsSceneObject(renderBuffer.impl))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer is not from this scene.");
            return nullptr;
        }

        if (ERenderBufferAccessMode_WriteOnly == renderBuffer.impl.getAccessMode())
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, render buffer has access mode write only.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode_Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType_RenderBuffer,
            ramses_internal::TextureSampler::ContentType::RenderBuffer,
            ramses_internal::ResourceContentHash::Invalid(),
            renderBuffer.impl.getRenderBufferHandle().asMemoryHandle(),
            name);
    }

    ramses::TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        uint32_t anisotropyLevel,
        const Texture2DBuffer& textureBuffer,
        const char* name)
    {
        if (!containsSceneObject(textureBuffer.impl))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, texture2D buffer is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode_Clamp, minSamplingMethod, magSamplingMethod, anisotropyLevel,
            ERamsesObjectType_Texture2DBuffer,
            ramses_internal::TextureSampler::ContentType::TextureBuffer,
            ramses_internal::ResourceContentHash::Invalid(),
            textureBuffer.impl.getTextureBufferHandle().asMemoryHandle(),
            name);
    }

    ramses::TextureSampler* SceneImpl::createTextureSampler(
        ETextureAddressMode wrapUMode,
        ETextureAddressMode wrapVMode,
        ETextureSamplingMethod minSamplingMethod,
        ETextureSamplingMethod magSamplingMethod,
        const StreamTexture& streamTexture,
        const char* name)
    {
        if (!containsSceneObject(streamTexture.impl))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureSampler failed, streamTexture is not from this scene.");
            return nullptr;
        }

        return createTextureSamplerImpl(
            wrapUMode, wrapVMode, ETextureAddressMode_Clamp, minSamplingMethod, magSamplingMethod, 1u,
            ERamsesObjectType_StreamTexture,
            ramses_internal::TextureSampler::ContentType::StreamTexture,
            ramses_internal::ResourceContentHash::Invalid(),
            streamTexture.impl.getHandle().asMemoryHandle(),
            name);
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
        const char* name /*= 0*/)
    {
        if (ETextureSamplingMethod_Nearest != magSamplingMethod && ETextureSamplingMethod_Linear != magSamplingMethod)
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

        TextureSamplerImpl& samplerImpl = *new TextureSamplerImpl(*this, name);
        samplerImpl.initializeFrameworkData(samplerStates, samplerType, contentType, textureResourceHash, contentHandle);

        TextureSampler* sampler = new TextureSampler(samplerImpl);
        registerCreatedObject(*sampler);
        return sampler;
    }

    DataFloat* SceneImpl::createDataFloat(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataFloat, name);
        pimpl.initializeFrameworkData();
        DataFloat* dataObject = new DataFloat(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataVector2f* SceneImpl::createDataVector2f(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataVector2f, name);
        pimpl.initializeFrameworkData();
        DataVector2f* dataObject = new DataVector2f(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataVector3f* SceneImpl::createDataVector3f(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataVector3f, name);
        pimpl.initializeFrameworkData();
        DataVector3f* dataObject = new DataVector3f(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataVector4f* SceneImpl::createDataVector4f(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataVector4f, name);
        pimpl.initializeFrameworkData();
        DataVector4f* dataObject = new DataVector4f(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataMatrix22f* SceneImpl::createDataMatrix22f(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataMatrix22f, name);
        pimpl.initializeFrameworkData();
        DataMatrix22f* dataObject = new DataMatrix22f(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataMatrix33f* SceneImpl::createDataMatrix33f(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataMatrix33f, name);
        pimpl.initializeFrameworkData();
        DataMatrix33f* dataObject = new DataMatrix33f(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataMatrix44f* SceneImpl::createDataMatrix44f(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataMatrix44f, name);
        pimpl.initializeFrameworkData();
        DataMatrix44f* dataObject = new DataMatrix44f(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataInt32* SceneImpl::createDataInt32(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataInt32, name);
        pimpl.initializeFrameworkData();
        DataInt32* dataObject = new DataInt32(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataVector2i* SceneImpl::createDataVector2i(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataVector2i, name);
        pimpl.initializeFrameworkData();
        DataVector2i* dataObject = new DataVector2i(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataVector3i* SceneImpl::createDataVector3i(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataVector3i, name);
        pimpl.initializeFrameworkData();
        DataVector3i* dataObject = new DataVector3i(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    DataVector4i* SceneImpl::createDataVector4i(const char* name /* =0 */)
    {
        DataObjectImpl& pimpl = *new DataObjectImpl(*this, ERamsesObjectType_DataVector4i, name);
        pimpl.initializeFrameworkData();
        DataVector4i* dataObject = new DataVector4i(pimpl);
        registerCreatedObject(*dataObject);

        return dataObject;
    }

    status_t SceneImpl::createTransformationDataProvider(const Node& node, dataProviderId_t id)
    {
        if (!containsSceneObject(node.impl))
        {
            return addErrorEntry("Scene::createTransformationDataProvider failed, node is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createTransformationDataProvider failed, duplicate data slot id");
        }

        const ramses_internal::NodeHandle nodeHandle = node.impl.getNodeHandle();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForNode(m_scene, nodeHandle))
        {
            return addErrorEntry("Scene::createTransformationDataProvider failed, Node already has a transformation data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationProvider, internalDataSlotId, nodeHandle, ramses_internal::DataInstanceHandle::Invalid(), ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createTransformationDataConsumer(const Node& node, dataConsumerId_t id)
    {
        if (!containsSceneObject(node.impl))
        {
            return addErrorEntry("Scene::createTransformationDataConsumer failed, Group Node is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createTransformationDataConsumer failed, duplicate data slot id");
        }

        const ramses_internal::NodeHandle nodeHandle = node.impl.getNodeHandle();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForNode(m_scene, nodeHandle))
        {
            return addErrorEntry("Scene::createTransformationDataConsumer failed, Node already has a transformation data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationConsumer, internalDataSlotId, nodeHandle, ramses_internal::DataInstanceHandle::Invalid(), ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createDataProvider(const DataObject& dataObject, dataProviderId_t id)
    {
        if (!containsSceneObject(dataObject.impl))
        {
            return addErrorEntry("Scene::createDataProvider failed, data object is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createDataProvider failed, duplicate data slot id");
        }

        const ramses_internal::DataInstanceHandle dataRef = dataObject.impl.getDataReference();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForDataObject(m_scene, dataRef))
        {
            return addErrorEntry("Scene::createDataProvider failed, data object already has a data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_DataProvider, internalDataSlotId, ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createDataConsumer(const DataObject& dataObject, dataConsumerId_t id)
    {
        if (!containsSceneObject(dataObject.impl))
        {
            return addErrorEntry("Scene::createDataConsumer failed, data object is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createDataConsumer failed, duplicate data slot id");
        }

        const ramses_internal::DataInstanceHandle dataRef = dataObject.impl.getDataReference();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForDataObject(m_scene, dataRef))
        {
            return addErrorEntry("Scene::createDataConsumer failed, data object already has a data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_DataConsumer, internalDataSlotId, ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::createTextureProvider(const Texture2D& texture, dataProviderId_t id)
    {
        if (this != &texture.impl.getSceneImpl())
        {
            return addErrorEntry("Scene::createTextureProvider failed, texture is not from this scene.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createTextureProvider failed, duplicate data slot id");
        }

        const ramses_internal::ResourceContentHash& textureHash = texture.impl.getLowlevelResourceHash();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForTexture(m_scene, textureHash))
        {
            return addErrorEntry("Scene::createTextureProvider failed, texture already has a data slot assigned in this scene");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TextureProvider, internalDataSlotId, ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle::Invalid(), textureHash, ramses_internal::TextureSamplerHandle() });
        return StatusOK;
    }

    status_t SceneImpl::updateTextureProvider(const Texture2D& texture, dataProviderId_t id)
    {
        if (this != &texture.impl.getSceneImpl())
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
                const ramses_internal::ResourceContentHash& textureHash = texture.impl.getLowlevelResourceHash();
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
        if (!containsSceneObject(sampler.impl))
        {
            return addErrorEntry("Scene::createTextureConsumer failed, texture sampler is not from this scene.");
        }

        if (sampler.impl.getTextureType() != ERamsesObjectType_Texture2D)
        {
            return addErrorEntry("Scene::createTextureConsumer failed, only texture sampler using 2D texture can be used for linking.");
        }

        const ramses_internal::DataSlotId internalDataSlotId(id.getValue());
        if (ramses_internal::DataSlotUtils::HasDataSlotId(m_scene, internalDataSlotId))
        {
            return addErrorEntry("Scene::createTextureConsumer failed, duplicate data slot id");
        }

        const ramses_internal::TextureSamplerHandle& samplerHandle = sampler.impl.getTextureSamplerHandle();
        if (ramses_internal::DataSlotUtils::HasDataSlotIdForTextureSampler(m_scene, samplerHandle))
        {
            return addErrorEntry("Scene::createTextureConsumer failed, texture sampler already has a data slot assigned");
        }

        m_scene.allocateDataSlot({ ramses_internal::EDataSlotType_TextureConsumer, internalDataSlotId, ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle::Invalid(), ramses_internal::ResourceContentHash::Invalid(), samplerHandle });
        return StatusOK;
    }

    status_t SceneImpl::setExpirationTimestamp(uint64_t ptpExpirationTimestampInMilliseconds)
    {
        if (ptpExpirationTimestampInMilliseconds == 0 && m_expirationTimestamp != ramses_internal::FlushTime::InvalidTimestamp)
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Scene::setExpirationTimestamp: sceneId " << m_scene.getSceneId() << " disabled expiration checking (ts " << ptpExpirationTimestampInMilliseconds << ")");
        }
        else if (ptpExpirationTimestampInMilliseconds != 0 && m_expirationTimestamp == ramses_internal::FlushTime::InvalidTimestamp)
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Scene::setExpirationTimestamp: sceneId " << m_scene.getSceneId() << " enabled expiration checking (ts " << ptpExpirationTimestampInMilliseconds << ")");
        }
        m_expirationTimestamp = ramses_internal::FlushTime::Clock::time_point(std::chrono::milliseconds{ ptpExpirationTimestampInMilliseconds });
        return StatusOK;
    }

    status_t SceneImpl::flush(sceneVersionTag_t sceneVersion)
    {
        if (m_nextSceneVersion != InvalidSceneVersionTag && sceneVersion == InvalidSceneVersionTag)
        {
            sceneVersion = m_nextSceneVersion;
            m_nextSceneVersion = InvalidSceneVersionTag;
        }

        const ramses_internal::SceneVersionTag sceneVersionInternal(sceneVersion);

        m_commandBuffer.execute(ramses_internal::SceneCommandVisitor(*this));
        applyHierarchicalVisibility();

        const auto timestampOfFlushCall = ramses_internal::FlushTime::Clock::now();
        const ramses_internal::FlushTimeInformation flushTimeInfo { m_expirationTimestamp, timestampOfFlushCall, ramses_internal::FlushTime::Clock::getClockType() };

        getClientImpl().getClientApplication().flush(m_scene.getSceneId(), flushTimeInfo, sceneVersionInternal);

        getClientImpl().updateClientResourceCache();

        getStatisticCollection().statFlushesTriggered.incCounter(1);

        return StatusOK;
    }

    AnimationSystem* SceneImpl::createAnimationSystem(uint32_t flags, const char* name)
    {
        uint32_t creationFlags = ramses_internal::EAnimationSystemFlags_Default;
        if ((flags & EAnimationSystemFlags_ClientSideProcessing) != 0)
        {
            creationFlags |= ramses_internal::EAnimationSystemFlags_FullProcessing;
        }

        AnimationSystemImpl& pimpl = createAnimationSystemImpl(creationFlags, ERamsesObjectType_AnimationSystem, name);
        AnimationSystem* animationSystem = new AnimationSystem(pimpl);
        registerCreatedObject(*animationSystem);
        return animationSystem;
    }

    AnimationSystemRealTime* SceneImpl::createRealTimeAnimationSystem(uint32_t flags, const char* name)
    {
        uint32_t creationFlags = ramses_internal::EAnimationSystemFlags_RealTime;
        if ((flags & EAnimationSystemFlags_ClientSideProcessing) != 0)
        {
            creationFlags |= ramses_internal::EAnimationSystemFlags_FullProcessing;
        }

        AnimationSystemImpl& pimpl = createAnimationSystemImpl(creationFlags, ERamsesObjectType_AnimationSystemRealTime, name);
        AnimationSystemRealTime* animationSystem = new AnimationSystemRealTime(pimpl);
        registerCreatedObject(*animationSystem);
        return animationSystem;
    }

    AnimationSystemImpl& SceneImpl::createAnimationSystemImpl(uint32_t flags, ERamsesObjectType type, const char* name)
    {
        ramses_internal::AnimationSystemFactory animSystemFactory(ramses_internal::EAnimationSystemOwner_Client, &m_scene.getSceneActionCollection());
        ramses_internal::IAnimationSystem* ianimationSystem =
            animSystemFactory.createAnimationSystem(flags, ramses_internal::AnimationSystemSizeInformation());
        AnimationSystemImpl& pimpl = *new AnimationSystemImpl(*this, type, name);
        pimpl.initializeFrameworkData(*ianimationSystem);
        return pimpl;
    }

    status_t SceneImpl::destroyAnimationSystem(AnimationSystem& animationSystem)
    {
        return destroyObject(animationSystem);
    }

    RamsesObjectRegistry& SceneImpl::getObjectRegistry()
    {
        return m_objectRegistry;
    }

    bool SceneImpl::containsSceneObject(const SceneObjectImpl& object) const
    {
        return &object.getSceneImpl() == this;
    }

    void SceneImpl::registerCreatedObject(SceneObject& object)
    {
        m_objectRegistry.addObject(object);
    }

    const RamsesObject* SceneImpl::findObjectByName(const char* name) const
    {
        return m_objectRegistry.findObjectByName(name);
    }

    RamsesObject* SceneImpl::findObjectByName(const char* name)
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
            container->impl.removeIfContained(object.impl);
        }
    }

    void SceneImpl::removeAllDataSlotsForNode(const Node& node)
    {
        const ramses_internal::NodeHandle nodeHandle = node.impl.getNodeHandle();
        const ramses_internal::UInt32 slotHandleCount = m_scene.getDataSlotCount();
        for (ramses_internal::DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
        {
            if (m_scene.isDataSlotAllocated(slotHandle) && m_scene.getDataSlot(slotHandle).attachedNode == nodeHandle)
            {
                m_scene.releaseDataSlot(slotHandle);
            }
        }
    }

    RenderPass* SceneImpl::createRenderPassInternal(const char* name)
    {
        RenderPassImpl& pimpl = *new RenderPassImpl(*this, name);
        pimpl.initializeFrameworkData();
        RenderPass* renderPass = new RenderPass(pimpl);
        registerCreatedObject(*renderPass);

        return renderPass;
    }

    bool SceneImpl::cameraIsAssignedToRenderPasses(const Camera& camera)
    {
        RamsesObjectRegistryIterator iterator(m_objectRegistry, ERamsesObjectType_RenderPass);
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
            if (nodeType == ERamsesObjectType_MeshNode)
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

    ArrayBuffer* SceneImpl::createArrayBuffer(EDataType dataType, uint32_t maxNumElements, const char* name)
    {
        ArrayBufferImpl* pimpl = createArrayBufferImpl(dataType, maxNumElements, name);

        if (nullptr != pimpl)
        {
            ArrayBuffer* buffer = new ArrayBuffer(*pimpl);
            registerCreatedObject(*buffer);
            return buffer;
        }

        return nullptr;
    }

    ArrayBufferImpl* SceneImpl::createArrayBufferImpl(EDataType dataType, uint32_t numElements, const char* name)
    {
        ArrayBufferImpl* pimpl = new ArrayBufferImpl(*this, name);
        pimpl->initializeFrameworkData(dataType, numElements);

        return pimpl;
    }

    Texture2DBuffer* SceneImpl::createTexture2DBuffer(uint32_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, const char* name)
    {
        Texture2DBuffer* buffer = nullptr;
        Texture2DBufferImpl* pimpl = createTexture2DBufferImpl(mipLevels, width, height, textureFormat, name);

        if(nullptr != pimpl)
        {
            buffer = new Texture2DBuffer(*pimpl);
            registerCreatedObject(*buffer);
        }

        return buffer;
    }

    Texture2DBufferImpl* SceneImpl::createTexture2DBufferImpl(uint32_t mipLevels, uint32_t width, uint32_t height, ETextureFormat textureFormat, const char* name)
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

        Texture2DBufferImpl* pimpl = new Texture2DBufferImpl(*this, name);
        pimpl->initializeFrameworkData(mipMapSizes, textureFormat);
        return pimpl;
    }

    ramses_internal::StatisticCollectionScene& SceneImpl::getStatisticCollection()
    {
        return m_scene.getStatisticCollection();
    }

    ramses::SceneReference* SceneImpl::createSceneReference(sceneId_t referencedScene, const char* name)
    {
        if (!referencedScene.isValid())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene::createSceneReference: cannot reference a scene with invalid scene ID.");
            return nullptr;
        }

        const auto scenes = getClientImpl().getListOfScenes();
        for (const auto scene : scenes)
        {
            if (getClientImpl().findSceneReference(scene->getSceneId(), referencedScene))
            {
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene::createSceneReference: there is already a SceneReference with sceneId "
                    << referencedScene << " in master scene " << scene->getSceneId() << ", cannot create another one");
                return nullptr;
            }
        }

        SceneReferenceImpl& pimpl = *new SceneReferenceImpl(*this, name);
        pimpl.initializeFrameworkData(referencedScene);
        SceneReference* sr = new SceneReference(pimpl);
        registerCreatedObject(*sr);
        m_sceneReferences.put(referencedScene, sr);
        return sr;
    }

    status_t SceneImpl::linkData(SceneReference* providerReference, dataProviderId_t providerId, SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        if (!providerReference && !consumerReference)
            return addErrorEntry("Scene::linkData: can't link an object to another object in the same scene");

        if (consumerReference == providerReference)
            return addErrorEntry("Scene::linkData: can't link an object to another object in the same scene reference");

        if ((providerReference && providerReference->impl.getSceneImpl().getSceneId() != getSceneId()) ||
            (consumerReference && consumerReference->impl.getSceneImpl().getSceneId() != getSceneId()))
            return addErrorEntry("Scene::linkData: can't link to object of a scene reference with a different master scene");

        if (providerReference && providerReference->impl.getReportedState() < RendererSceneState::Available)
            return addErrorEntry("Scene::linkData: Provider SceneReference state has to be at least Available");

        if (consumerReference && consumerReference->impl.getReportedState() < RendererSceneState::Available)
            return addErrorEntry("Scene::linkData: Consumer SceneReference state has to be at least Available");

        const auto providerScene = (providerReference ? providerReference->impl.getSceneReferenceHandle() : ramses_internal::SceneReferenceHandle{});
        const auto consumerScene = (consumerReference ? consumerReference->impl.getSceneReferenceHandle() : ramses_internal::SceneReferenceHandle{});
        getIScene().linkData(providerScene, ramses_internal::DataSlotId{ providerId.getValue() }, consumerScene, ramses_internal::DataSlotId{ consumerId.getValue() });

        return StatusOK;
    }

    status_t SceneImpl::unlinkData(SceneReference* consumerReference, dataConsumerId_t consumerId)
    {
        if (consumerReference && consumerReference->impl.getSceneImpl().getSceneId() != getSceneId())
            return addErrorEntry("Scene::unlinkData: can't unlink object of a scene reference with a different master scene");

        const auto consumerScene = (consumerReference ? consumerReference->impl.getSceneReferenceHandle() : ramses_internal::SceneReferenceHandle{});
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

    ArrayResource* SceneImpl::createArrayResource(EDataType type, uint32_t numElements, const void* arrayData, resourceCacheFlag_t cacheFlag, const char* name)
    {
        if (0u == numElements || nullptr == arrayData)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Scene::createArrayResource: Array resource must have element count > 0 and data must not be nullptr!");
            return nullptr;
        }

        ramses_internal::ManagedResource res = getClientImpl().createManagedArrayResource(numElements, type, arrayData, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createArrayResource: failed to create managed array resource");
            return nullptr;
        }
        return createHLArrayResource(res, name);
    }

    ArrayResource* SceneImpl::createHLArrayResource(ramses_internal::ManagedResource const& resource, const char* name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_IndexArray ||
               resource->getTypeID() == ramses_internal::EResourceType_VertexArray);

        const auto arrayRes = resource->convertTo<ramses_internal::ArrayResource>();
        ramses_internal::ResourceHashUsage usage = getClientImpl().getClientApplication().getHashUsage(arrayRes->getHash());

        ArrayResourceImpl& pimpl = *new ArrayResourceImpl(usage, *this, name);
        pimpl.initializeFromFrameworkData(arrayRes->getElementCount(), DataTypeUtils::ConvertDataTypeFromInternal(arrayRes->getElementType()));

        ArrayResource* arrayResHL = new ArrayResource(pimpl);
        registerCreatedResourceObject(*arrayResHL);
        return arrayResHL;
    }

    Texture2D* SceneImpl::createTexture2D(uint32_t width, uint32_t height, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedTexture(ramses_internal::EResourceType_Texture2D, width, height, 1u, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture2D: failed to create managed Texture2D resource");
            return nullptr;
        }
        return createHLTexture2D(res, name);
    }

    Texture2D* SceneImpl::createHLTexture2D(ramses_internal::ManagedResource const& resource, const char* name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_Texture2D);

        const auto texRes = resource->convertTo<ramses_internal::TextureResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        Texture2DImpl& pimpl = *new Texture2DImpl(hashUsage, *this, name);
        pimpl.initializeFromFrameworkData(texRes->getWidth(), texRes->getHeight(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()),
            TextureUtils::GetTextureSwizzleFromInternal(texRes->getTextureSwizzle()));

        Texture2D* texture = new Texture2D(pimpl);
        registerCreatedResourceObject(*texture);
        return texture;
    }

    Texture3D* SceneImpl::createTexture3D(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat format, uint32_t mipMapCount, const MipLevelData mipLevelData[], bool generateMipChain, resourceCacheFlag_t cacheFlag, const char* name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedTexture(ramses_internal::EResourceType_Texture3D, width, height, depth, format, mipMapCount, mipLevelData, generateMipChain, {}, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTexture3D: failed to create managed Texture3D resource");
            return nullptr;
        }
        return createHLTexture3D(res, name);
    }

    Texture3D* SceneImpl::createHLTexture3D(ramses_internal::ManagedResource const& resource, const char* name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_Texture3D);
        const auto texRes = resource->convertTo<ramses_internal::TextureResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        Texture3DImpl& pimpl = *new Texture3DImpl(hashUsage, *this, name);
        pimpl.initializeFromFrameworkData(texRes->getWidth(), texRes->getHeight(), texRes->getDepth(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()));

        Texture3D* texture = new Texture3D(pimpl);
        registerCreatedResourceObject(*texture);
        return texture;
    }

    TextureCube* SceneImpl::createTextureCube(uint32_t size, ETextureFormat format, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], bool generateMipChain, const TextureSwizzle& swizzle, resourceCacheFlag_t cacheFlag, const char* name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedTexture(ramses_internal::EResourceType_TextureCube, size, 1u, 1u, format, mipMapCount, mipLevelData, generateMipChain, swizzle, cacheFlag, name);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createTextureCube: failed to create managed TextureCube resource");
            return nullptr;
        }
        return createHLTextureCube(res, name);
    }

    TextureCube* SceneImpl::createHLTextureCube(ramses_internal::ManagedResource const& resource, const char* name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_TextureCube);

        const auto texRes = resource->convertTo<ramses_internal::TextureResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        TextureCubeImpl& pimpl = *new TextureCubeImpl(hashUsage, *this, name);
        pimpl.initializeFromFrameworkData(texRes->getWidth(),
            TextureUtils::GetTextureFormatFromInternal(texRes->getTextureFormat()),
            TextureUtils::GetTextureSwizzleFromInternal(texRes->getTextureSwizzle()));

        TextureCube* texture = new TextureCube(pimpl);
        registerCreatedResourceObject(*texture);
        return texture;
    }

    Effect* SceneImpl::createEffect(const EffectDescription& effectDesc, resourceCacheFlag_t cacheFlag, const char* name)
    {
        ramses_internal::ManagedResource res = getClientImpl().createManagedEffect(effectDesc, cacheFlag, name, m_effectErrorMessages);
        if (!res)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene::createEffect: failed to create managed effect resource: " << m_effectErrorMessages);
            return nullptr;
        }

        return createHLEffect(res, name);
    }

    Effect* SceneImpl::createHLEffect(ramses_internal::ManagedResource const& resource, const char* name)
    {
        assert(resource->getTypeID() == ramses_internal::EResourceType_Effect);

        const auto effectRes = resource->convertTo<ramses_internal::EffectResource>();
        ramses_internal::ResourceHashUsage hashUsage = getClientImpl().getClientApplication().getHashUsage(resource->getHash());

        ramses::EffectImpl& pimpl = *new ramses::EffectImpl(hashUsage, *this, name);
        pimpl.initializeFromFrameworkData(effectRes->getUniformInputs(), effectRes->getAttributeInputs());

        Effect* effect = new Effect(pimpl);
        registerCreatedResourceObject(*effect);
        return effect;
    }

    void SceneImpl::registerCreatedResourceObject(Resource& resource)
    {
        registerCreatedObject(resource);
        const resourceId_t resId = resource.getResourceId();
        m_resourcesById.put(resId, &resource);
    }

    std::string SceneImpl::getLastEffectErrorMessages() const
    {
        return m_effectErrorMessages;
    }

    ramses::Resource* SceneImpl::getResource(resourceId_t rid) const
    {
        auto resource = m_resourcesById.get(rid);
        return resource ? *resource : nullptr;
    }

    Resource* SceneImpl::scanForResourceWithHash(ramses_internal::ResourceContentHash hash) const
    {
        for (const auto& res : m_resourcesById)
        {
            if (hash == res.value->impl.getLowlevelResourceHash())
                return res.value;
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

    status_t SceneImpl::saveToFile(const char* fileName, bool compress) const
    {
        ramses_internal::File outputFile(fileName);
        ramses_internal::BinaryFileOutputStream outputStream(outputFile);
        if (!outputFile.isOpen())
            return addErrorEntry("Scene::saveToFile failed, could not open file for writing.");

        ramses_internal::RamsesVersion::WriteToStream(outputStream, ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH);

        ramses_internal::UInt bytesForVersion = 0;
        if (!outputFile.getPos(bytesForVersion))
            return addErrorEntry("Scene::saveToFile failed, error getting save file position.");

        // reserve space for offset to SceneObjects and LL-Objects
        const uint64_t bytesForOffsets = sizeof(uint64_t) * 2u;
        const uint64_t offsetSceneObjectsStart = bytesForVersion + bytesForOffsets;

        if (!outputFile.seek(static_cast<ramses_internal::Int>(offsetSceneObjectsStart), ramses_internal::File::SeekOrigin::BeginningOfFile))
            return addErrorEntry("Scene::saveToFile failed, error seeking file.");

        const status_t status = writeSceneObjectsToStream(outputStream);

        ramses_internal::UInt offsetLLResourcesStart = 0;
        if (!outputFile.getPos(offsetLLResourcesStart))
            return addErrorEntry("Scene::saveToFile failed, error getting save file position.");

        ResourceObjects resources;
        resources.reserve(m_resourcesById.size());
        for (auto const& res : m_resourcesById)
            resources.push_back(res.value);
        getClientImpl().writeLowLevelResourcesToStream(resources, outputStream, compress);

        if (!outputFile.seek(bytesForVersion, ramses_internal::File::SeekOrigin::BeginningOfFile))
            return addErrorEntry("Scene::saveToFile failed, error seeking file.");

        outputStream << static_cast<uint64_t>(offsetSceneObjectsStart);
        outputStream << static_cast<uint64_t>(offsetLLResourcesStart);

        if (!outputFile.close())
            return addErrorEntry("Scene::saveToFile failed, close file failed.");

        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "Scene::saveToFile:  '" << fileName << "'.");

        return status;
    }

    bool SceneImpl::saveResources(std::string const& fileName, bool compress) const
    {
        ResourceObjects resources;
        resources.reserve(m_resourcesById.size());
        for (auto entry : m_resourcesById)
            resources.push_back(entry.value);

        return getClientImpl().getResourceDataPool().impl.saveResourceDataFile(fileName, resources, compress);
    }

    void SceneImpl::setSceneFileName(std::string const& sceneFilename)
    {
        m_sceneFilename = sceneFilename;
    }

}
