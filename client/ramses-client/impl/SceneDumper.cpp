//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneDumper.h"

#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/SceneObject.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Resource.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Camera.h"

#include "Texture2DBufferImpl.h"
#include "GeometryBindingImpl.h"
#include "Texture2DBufferImpl.h"
#include "ObjectIteratorImpl.h"
#include "StreamTextureImpl.h"
#include "RamsesClientImpl.h"
#include "RenderBufferImpl.h"
#include "RenderTargetImpl.h"
#include "RenderGroupImpl.h"
#include "AppearanceImpl.h"
#include "RenderPassImpl.h"
#include "CameraNodeImpl.h"
#include "MeshNodeImpl.h"
#include "ResourceImpl.h"
#include "BlitPassImpl.h"
#include "EffectImpl.h"
#include "SceneImpl.h"

#include "Scene/ClientScene.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/BlitPass.h"

namespace ramses
{
    template <>
    ramses_internal::TextureSamplerHandle SceneDumper::GetHandle(const TextureSamplerImpl& impl)
    {
        return impl.getTextureSamplerHandle();
    }

    template <> ramses_internal::RenderBufferHandle SceneDumper::GetHandle(const RenderBufferImpl& impl)
    {
        return impl.getRenderBufferHandle();
    }

    template <> ramses_internal::TextureBufferHandle SceneDumper::GetHandle(const Texture2DBufferImpl& impl)
    {
        return impl.getTextureBufferHandle();
    }

    template <> ramses_internal::StreamTextureHandle SceneDumper::GetHandle(const StreamTextureImpl& impl)
    {
        return impl.getHandle();
    }

    template <class ObjectType, class ObjectImplType, class HandleType>
    void SceneDumper::setupMap(ramses_internal::HashMap<HandleType, const ObjectImplType*>& map)
    {
        map.clear();
        RamsesObjectRegistryIterator iterator(m_objectRegistry, TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID);
        while (const ObjectType* object = iterator.getNext<ObjectType>())
        {
            map.put(GetHandle<ObjectImplType, HandleType>(object->impl), &object->impl);
        }
    }

    SceneDumper::SceneDumper(const ramses::SceneImpl& scene)
        : m_client(scene.getClientImpl())
        , m_objectRegistry(scene.getObjectRegistry())
    {
    }

    void SceneDumper::dumpUnrequiredObjects(ramses_internal::StringOutputStream& output)
    {
        setupMaps();
        markAllObjectsOfTypeAsRequired(ERamsesObjectType_AnimationObject);
        markAllObjectsOfTypeAsRequired(ERamsesObjectType_AnimationSystem);
        markAllObjectsOfTypeAsRequired(ERamsesObjectType_AnimationSystemRealTime);

        RenderPassSet requiredRenderPasses = markRequiredScreenRenderPasses();
        while (requiredRenderPasses.size() > 0)
        {
            output << "SceneDumper::dumpUnrequiredObject number of render passes: " << requiredRenderPasses.size() << "\n";

            RenderGroupSet requiredRenderGroups = markRequiredRenderGroups(requiredRenderPasses);
            MeshNodeSet    requiredMeshNodes    = markRequiredMeshNodes(requiredRenderGroups);
            CameraNodeSet  requiredCameraNodes  = markRequiredCameras(requiredRenderPasses);
            markAllParentNodesAsRequired(requiredMeshNodes);
            markAllParentNodesAsRequired(requiredCameraNodes);
            AppearanceSet     requiredAppearances     = markRequiredAppearance(requiredMeshNodes);
            TextureSamplerSet requiredTextureSamplers = markRequiredTextureSampler(requiredAppearances);
            markRequiredEffects(requiredAppearances);
            markRequiredTextures(requiredTextureSamplers);
            GeometryBindingSet requiredGeometryBindings = markRequiredGeometryBindings(requiredMeshNodes);
            markRequiredVertexAndIndexBuffers(requiredGeometryBindings);
            markRequiredRenderTargets(requiredRenderPasses);
            RenderBufferSet requiredRenderBuffers = markRequiredRenderBuffer(requiredTextureSamplers);
            markRequiredTextureBuffer(requiredTextureSamplers);
            markRequiredStreamTextures(requiredTextureSamplers);

            requiredRenderPasses = getRequiredRenderPasses(requiredRenderBuffers);
        }

        outputNotRequiredObjects(output);
    }

    void SceneDumper::setupMaps()
    {
        setupMap<TextureSampler, TextureSamplerImpl, ramses_internal::TextureSamplerHandle>(m_textureSamplerHandleToObjectMap);
        setupMap<RenderBuffer, RenderBufferImpl, ramses_internal::RenderBufferHandle>(m_renderBufferHandleToObjectMap);
        setupMap<Texture2DBuffer, Texture2DBufferImpl, ramses_internal::TextureBufferHandle>(m_textureBufferHandleToObjectMap);
        setupMap<StreamTexture, StreamTextureImpl, ramses_internal::StreamTextureHandle>(m_streamTextureHandleToObjectMap);

        setupResourceMap();
        setupRenderPassMap();
        setupRenderBufferSetMap();
    }

    void SceneDumper::setupResourceMap()
    {
        m_resourceContentHashToObjectMap.clear();

        ObjectIteratorImpl iter(m_objectRegistry, ERamsesObjectType_Resource);
        auto resourceAsObject = iter.getNext();
        while (resourceAsObject)
        {
            const ResourceImpl* resource = &RamsesObjectTypeUtils::ConvertTo<Resource>(*resourceAsObject).impl;
            m_resourceContentHashToObjectMap.put(resource->getLowlevelResourceHash(), resource);
            resourceAsObject = iter.getNext();
        }
    }

    void SceneDumper::setupRenderPassMap()
    {
        m_destinationRenderBufferHandleToRenderPassSetMap.clear();
        RamsesObjectRegistryIterator renderPassIterator(m_objectRegistry, ERamsesObjectType_RenderPass);
        while (const RenderPass* renderPass = renderPassIterator.getNext<RenderPass>())
        {
            const RenderTarget* renderTarget = renderPass->getRenderTarget();
            if (nullptr != renderTarget)
            {
                const ramses_internal::ClientScene& clientScene = renderTarget->impl.getIScene();

                const ramses_internal::RenderTargetHandle renderTargetHandle =
                    renderTarget->impl.getRenderTargetHandle();
                uint32_t renderBufferCount = clientScene.getRenderTargetRenderBufferCount(renderTargetHandle);

                for (uint32_t i = 0; i < renderBufferCount; i++)
                {
                    const ramses_internal::RenderBufferHandle renderBufferHandle =
                        clientScene.getRenderTargetRenderBuffer(renderTargetHandle, i);

                    m_destinationRenderBufferHandleToRenderPassSetMap[renderBufferHandle].put(&renderPass->impl);
                }
            }
        }
    }

    void SceneDumper::setupRenderBufferSetMap()
    {
        m_blitPassDestinationRenderBufferToSourceRenderBuffersMap.clear();
        RamsesObjectRegistryIterator blitPassIterator(m_objectRegistry, ERamsesObjectType_BlitPass);
        while (const BlitPass* blitPass = blitPassIterator.getNext<BlitPass>())
        {
            const ramses_internal::ClientScene& clientScene = blitPass->impl.getIScene();
            const ramses_internal::BlitPassHandle blitPassHandle = blitPass->impl.getBlitPassHandle();
            const ramses_internal::BlitPass& blitPassData = clientScene.getBlitPass(blitPassHandle);

            const RenderBufferImpl** sourceRenderBuffer = m_renderBufferHandleToObjectMap.get(blitPassData.sourceRenderBuffer);
            if (nullptr != sourceRenderBuffer)
            {
                m_blitPassDestinationRenderBufferToSourceRenderBuffersMap[blitPassData.destinationRenderBuffer].put(
                    *sourceRenderBuffer);
            }
            else
            {
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                          "SceneDumper::setupRenderBufferSetMap Could not lookup render buffer handle: "
                              << blitPassData.sourceRenderBuffer << " !!!");
                assert(false);
            }
        }
    }

    void SceneDumper::markAllObjectsOfTypeAsRequired(ERamsesObjectType objectType)
    {
        RamsesObjectVector objects;
        m_objectRegistry.getObjectsOfType(objects, objectType);
        for (auto object : objects)
        {
            m_requiredObjects.put(&object->impl);
        }
    }

    bool SceneDumper::addToRequiredObjects(const RamsesObjectImpl& object)
    {
        if (!m_requiredObjects.contains(&object))
        {
            m_requiredObjects.put(&object);
            return true;
        }
        else
        {
            return false;
        }
    }

    SceneDumper::RenderPassSet SceneDumper::markRequiredScreenRenderPasses()
    {
        RenderPassSet requiredRenderPasses;

        RamsesObjectRegistryIterator objectIterator(m_objectRegistry, ERamsesObjectType_RenderPass);
        while (const RenderPass* renderPass = objectIterator.getNext<RenderPass>())
        {
            if (renderPass->isEnabled() && nullptr == renderPass->getRenderTarget())
            {
                if (addToRequiredObjects(renderPass->impl))
                {
                    requiredRenderPasses.put(&renderPass->impl);
                }
            }
        }
        return requiredRenderPasses;
    }

    SceneDumper::RenderPassSet SceneDumper::getRequiredRenderPasses(const RenderBufferSet& requiredRenderBuffers)
    {
        RenderPassSet requiredRenderPasses;

        for (auto renderBuffer : requiredRenderBuffers)
        {
            const RenderPassSet* renderPassSet =
                m_destinationRenderBufferHandleToRenderPassSetMap.get(renderBuffer->getRenderBufferHandle());
            if (nullptr != renderPassSet)
            {
                for (auto renderPass : *renderPassSet)
                {
                    if (addToRequiredObjects(*renderPass))
                    {
                        requiredRenderPasses.put(renderPass);
                    }
                }
            }
        }
        return requiredRenderPasses;
    }

    void SceneDumper::addRenderBufferRecursive(const RenderBufferImpl& renderBuffer,
                                               RenderBufferSet&        requiredRenderBuffers)
    {
        if (addToRequiredObjects(renderBuffer))
        {
            requiredRenderBuffers.put(&renderBuffer);

            const RenderBufferSet* sourceRenderBufferSet =
                m_blitPassDestinationRenderBufferToSourceRenderBuffersMap.get(renderBuffer.getRenderBufferHandle());

            if (nullptr != sourceRenderBufferSet)
            {
                for (auto sourceRenderBuffer : *sourceRenderBufferSet)
                {
                    assert(nullptr != sourceRenderBuffer);
                    addRenderBufferRecursive(*sourceRenderBuffer, requiredRenderBuffers);
                }
            }
        }
    }

    SceneDumper::RenderGroupSet SceneDumper::markRequiredRenderGroups(const RenderPassSet& requiredRenderPasses)
    {
        RenderGroupSet requiredRenderGroups;
        for (auto renderPass : requiredRenderPasses)
        {
            const RenderGroupVector& renderGroups = renderPass->getAllRenderGroups();
            for (auto renderGroup : renderGroups)
            {
                addRenderGroupRecursive(*renderGroup, requiredRenderGroups);
            }
        }
        return requiredRenderGroups;
    }

    SceneDumper::MeshNodeSet SceneDumper::markRequiredMeshNodes(const RenderGroupSet& requiredRenderGroups)
    {
        SceneDumper::MeshNodeSet requiredMeshNodes;
        for (auto renderGroup : requiredRenderGroups)
        {
            const MeshNodeImplVector& meshNodes = renderGroup->getAllMeshes();

            for (auto meshNode : meshNodes)
            {
                if (addToRequiredObjects(*meshNode))
                {
                    requiredMeshNodes.put(meshNode);
                }
            }
        }
        return requiredMeshNodes;
    }

    SceneDumper::AppearanceSet SceneDumper::markRequiredAppearance(const MeshNodeSet& requiredMeshNodes)
    {
        SceneDumper::AppearanceSet requiredAppearances;
        for (auto meshNode : requiredMeshNodes)
        {
            const AppearanceImpl* appearance = meshNode->getAppearanceImpl();

            if (nullptr != appearance)
            {
                if (addToRequiredObjects(*appearance))
                {
                    requiredAppearances.put(appearance);
                }
            }
        }
        return requiredAppearances;
    }

    SceneDumper::TextureSamplerSet SceneDumper::markRequiredTextureSampler(const AppearanceSet& requiredAppearances)
    {
        SceneDumper::TextureSamplerSet requiredTextureSamplers;

        for (auto appearance : requiredAppearances)
        {
            ramses_internal::DataInstanceHandle uniformHandle = appearance->getUniformDataInstance();
            const ramses_internal::ClientScene& scene         = appearance->getIScene();
            ramses_internal::DataLayoutHandle   layoutHandle  = scene.getLayoutOfDataInstance(uniformHandle);
            uint32_t                            fieldCount    = scene.getDataLayout(layoutHandle).getFieldCount();
            for (uint32_t field = 0; field < fieldCount; field++)
            {
                ramses_internal::DataFieldHandle fieldHandle(field);
                const auto fieldDataType = scene.getDataLayout(layoutHandle).getField(fieldHandle).dataType;
                if (IsTextureSamplerType(fieldDataType))
                {
                    ramses_internal::TextureSamplerHandle textureSamplerHandle =
                        scene.getDataTextureSamplerHandle(uniformHandle, fieldHandle);

                    if (ramses_internal::TextureSamplerHandle::Invalid() != textureSamplerHandle)
                    {
                        const TextureSamplerImpl** textureSampler =
                            m_textureSamplerHandleToObjectMap.get(textureSamplerHandle);

                        if (nullptr != textureSampler)
                        {
                            assert(nullptr != *textureSampler);
                            if (addToRequiredObjects(**textureSampler))
                            {
                                requiredTextureSamplers.put(*textureSampler);
                            }
                        }
                        else
                        {
                            LOG_ERROR(
                                ramses_internal::CONTEXT_CLIENT,
                                "SceneDumper::markRequiredTextureSampler Could not lookup texture sampler handle: "
                                    << textureSamplerHandle << " !!!");
                            assert(false);
                        }
                    }
                }
            }
        }
        return requiredTextureSamplers;
    }

    void SceneDumper::markRequiredEffects(const AppearanceSet& requiredAppearances)
    {
        for (auto appearance : requiredAppearances)
        {
            const EffectImpl* effect = appearance->getEffectImpl();
            if (effect)
            {
                addToRequiredObjects(*effect);
            }
        }
    }

    void SceneDumper::markRequiredTextures(const TextureSamplerSet& requiredTextureSamplers)
    {
        ramses_internal::HashSet<ramses_internal::ResourceContentHash> requiredTextureResourceHashes;
        for (auto textureSampler : requiredTextureSamplers)
        {
            ramses_internal::TextureSamplerHandle textureSamplerHandle = textureSampler->getTextureSamplerHandle();
            if (ramses_internal::TextureSamplerHandle::Invalid() != textureSamplerHandle)
            {
                ramses_internal::ResourceContentHash textureResourceHash =
                    textureSampler->getIScene().getTextureSampler(textureSamplerHandle).textureResource;
                if (ramses_internal::ResourceContentHash::Invalid() != textureResourceHash)
                {
                    requiredTextureResourceHashes.put(textureResourceHash);
                }
            }
        }
        markRequiredResourcesFromHash(requiredTextureResourceHashes);
    }

    SceneDumper::GeometryBindingSet SceneDumper::markRequiredGeometryBindings(const MeshNodeSet& requiredMeshNodes)
    {
        SceneDumper::GeometryBindingSet requiredGeometryBindings;
        for (auto meshNode : requiredMeshNodes)
        {
            const GeometryBindingImpl* geometryBinding = meshNode->getGeometryBindingImpl();

            if (nullptr != geometryBinding)
            {
                if (addToRequiredObjects(*geometryBinding))
                {
                    requiredGeometryBindings.put(geometryBinding);
                }
            }
        }
        return requiredGeometryBindings;
    }

    void SceneDumper::markRequiredVertexAndIndexBuffers(const GeometryBindingSet& requiredGeometryBindings)
    {
        ResourceContentHashSet requiredBufferResourceHashes;
        for (auto geometryBinding : requiredGeometryBindings)
        {
            const ramses_internal::ClientScene& iscene             = geometryBinding->getIScene();
            ramses_internal::DataInstanceHandle dataInstanceHandle = geometryBinding->getAttributeDataInstance();

            ramses_internal::DataLayoutHandle layoutHandle = geometryBinding->getAttributeDataLayout();

            if (ramses_internal::DataLayoutHandle::Invalid() != layoutHandle)
            {
                const uint32_t fieldCount = iscene.getDataLayout(layoutHandle).getFieldCount();
                for (ramses_internal::DataFieldHandle field(0); field < fieldCount; ++field)
                {
                    const ramses_internal::ResourceField& dataResource =
                        iscene.getDataResource(dataInstanceHandle, field);

                    ramses_internal::ResourceContentHash bufferResourceHash = dataResource.hash;
                    if (ramses_internal::ResourceContentHash::Invalid() != bufferResourceHash)
                    {
                        requiredBufferResourceHashes.put(bufferResourceHash);
                    }
                }
            }
        }
        markRequiredResourcesFromHash(requiredBufferResourceHashes);
    }

    SceneDumper::CameraNodeSet SceneDumper::markRequiredCameras(const RenderPassSet& requiredRenderPasses)
    {
        CameraNodeSet requiredCameras;
        for (auto renderPass : requiredRenderPasses)
        {
            const Camera* camera = renderPass->getCamera();
            if (nullptr != camera)
            {
                if (addToRequiredObjects(camera->impl))
                {
                    requiredCameras.put(&camera->impl);
                }
            }
        }

        return requiredCameras;
    }

    void SceneDumper::markRequiredResourcesFromHash(const ResourceContentHashSet& requiredResourceHashes)
    {
        for (auto requiredResourceHash : requiredResourceHashes)
        {
            const ResourceImpl** resource = m_resourceContentHashToObjectMap.get(requiredResourceHash);
            if (nullptr != resource)
            {
                assert(nullptr != *resource);
                addToRequiredObjects(**resource);
            }
            else
            {
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                          "SceneDumper::markRequiredResourcesFromHash Could not lookup resource content hash: "
                              << requiredResourceHash << " !!!");
                assert(false);
            }
        }
    }

    void SceneDumper::markRequiredRenderTargets(const RenderPassSet& requiredRenderPasses)
    {
        for (auto renderPass : requiredRenderPasses)
        {
            const RenderTarget* renderTarget = renderPass->getRenderTarget();
            if (nullptr != renderTarget)
            {
                addToRequiredObjects(renderTarget->impl);
            }
        }
    }

    SceneDumper::RenderBufferSet SceneDumper::markRequiredRenderBuffer(const TextureSamplerSet& requiredTextureSamplers)
    {
        RenderBufferSet requiredRenderBuffers;

        for (auto textureSampler : requiredTextureSamplers)
        {
            const auto sampler = textureSampler->getIScene().getTextureSampler(textureSampler->getTextureSamplerHandle());
            if (ramses_internal::TextureSampler::ContentType::RenderBuffer == sampler.contentType && ramses_internal::RenderBufferHandle::Invalid() != sampler.contentHandle)
            {
                const ramses_internal::RenderBufferHandle renderBufferHandle(sampler.contentHandle);
                const RenderBufferImpl** renderBuffer = m_renderBufferHandleToObjectMap.get(renderBufferHandle);

                if (nullptr != renderBuffer)
                {
                    assert(nullptr != *renderBuffer);
                    addRenderBufferRecursive(**renderBuffer, requiredRenderBuffers);
                }
                else
                {
                    LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                              "SceneDumper::getRequiredRenderBuffer Could not lookup render buffer handle: "
                                  << renderBufferHandle << " !!!");
                    assert(false);
                }
            }
        }

        return requiredRenderBuffers;
    }

    void SceneDumper::markRequiredTextureBuffer(const TextureSamplerSet& requiredTextureSamplers)
    {
        ramses_internal::HashSet<ramses_internal::ResourceContentHash> requiredTextureResourceHashes;
        for (auto textureSampler : requiredTextureSamplers)
        {
            const auto sampler = textureSampler->getIScene().getTextureSampler(textureSampler->getTextureSamplerHandle());

            if (ramses_internal::TextureSampler::ContentType::TextureBuffer == sampler.contentType && ramses_internal::TextureBufferHandle::Invalid() != sampler.contentHandle)
            {
                const ramses_internal::TextureBufferHandle textureBufferHandle(sampler.contentHandle);

                const Texture2DBufferImpl** textureBuffer = m_textureBufferHandleToObjectMap.get(textureBufferHandle);

                if (nullptr != textureBuffer)
                {
                    assert(nullptr != *textureBuffer);
                    addToRequiredObjects(**textureBuffer);
                }
                else
                {
                    LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                              "SceneDumper::getRequiredTextureBuffers Could not lookup texture buffer handle: "
                                  << textureBufferHandle << " !!!");
                    assert(false);
                }
            }
        }
    }

    SceneDumper::StreamTextureSet
    SceneDumper::markRequiredStreamTextures(const TextureSamplerSet& requiredTextureSamplers)
    {
        SceneDumper::StreamTextureSet                                  requiredStreamTextures;
        ramses_internal::HashSet<ramses_internal::ResourceContentHash> requiredTextureResourceHashes;
        for (auto textureSampler : requiredTextureSamplers)
        {
            const auto sampler = textureSampler->getIScene().getTextureSampler(textureSampler->getTextureSamplerHandle());
            if (ramses_internal::TextureSampler::ContentType::StreamTexture == sampler.contentType && ramses_internal::StreamTextureHandle::Invalid() != sampler.contentHandle)
            {
                const ramses_internal::StreamTextureHandle streamTextureHandle(sampler.contentHandle);
                const StreamTextureImpl** streamTexture = m_streamTextureHandleToObjectMap.get(streamTextureHandle);

                if (nullptr != streamTexture)
                {
                    assert(*streamTexture);
                    if (addToRequiredObjects(**streamTexture))
                    {
                        requiredStreamTextures.put(*streamTexture);
                    }
                }
                else
                {
                    LOG_ERROR(ramses_internal::CONTEXT_CLIENT,
                              "SceneDumper::getRequiredStreamTextures Could not lookup stream texture handle: "
                                  << streamTextureHandle << " !!!");
                    assert(false);
                }
            }
        }

        return requiredStreamTextures;
    }

    void SceneDumper::addNodeWithAllParentNodes(const NodeImpl* node)
    {
        while (nullptr != node)
        {
            if (!addToRequiredObjects(*node))
            {
                break;
            }
            node = node->getParentImpl();
        }
    }

    void SceneDumper::addRenderGroupRecursive(const RenderGroupImpl&                            renderGroup,
                                              ramses_internal::HashSet<const RenderGroupImpl*>& requiredRenderGroups)
    {
        if (addToRequiredObjects(renderGroup))
        {
            requiredRenderGroups.put(&renderGroup);
            const RenderGroupVector& renderGroups = renderGroup.getAllRenderGroups();
            for (auto childRenderGroup : renderGroups)
            {
                addRenderGroupRecursive(*childRenderGroup, requiredRenderGroups);
            }
        }
    }

    SceneDumper::RamsesObjectSet SceneDumper::getSceneAndClientObjects()
    {
        RamsesObjectSet    objects;
        RamsesObjectVector sceneObjects;
        m_objectRegistry.getObjectsOfType(sceneObjects, ERamsesObjectType_RamsesObject);
        for (auto sceneObject : sceneObjects)
        {
            objects.put(sceneObject);
        }

        return objects;
    }

    struct Counter
    {
        uint32_t unrequired = 0;
        uint32_t total      = 0;
    };

    void SceneDumper::outputNotRequiredObjects(ramses_internal::StringOutputStream& output)
    {
        RamsesObjectSet objects = getSceneAndClientObjects();
        output << "SceneDumper::outputNotRequiredObjects Unrequired objects:\n";

        ramses_internal::HashMap<ERamsesObjectType, Counter> typeStatistic;
        for (RamsesObject* object : objects)
        {
            const RamsesObjectImpl* objectImpl = &object->impl;
            ERamsesObjectType type = object->getType();
            if (false == m_requiredObjects.contains(objectImpl))
            {
                outputNotRequiredObject(*object, output);
                typeStatistic[type].unrequired++;
            }
            typeStatistic[type].total++;
        }

        output << "\nSceneDumper::outputNotRequiredObjects Statistic of unrequired objects:\n\n";

        AddString("OBEJCT TYPE", output, 39);

        AddString("#UNREQUIRED / #TOTAL\n\n", output);
        for (uint32_t type = 0; type < ERamsesObjectType::ERamsesObjectType_NUMBER_OF_TYPES; type++)
        {
            if (RamsesObjectTypeUtils::IsConcreteType(ERamsesObjectType(type)))
            {
                Counter counter = typeStatistic[ERamsesObjectType(type)];
                if (counter.total > 0)
                {

                    ramses_internal::StringOutputStream unrequiredString;
                    unrequiredString << counter.unrequired;
                    ramses_internal::StringOutputStream totalString;
                    totalString << counter.total;

                    AddString(RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType(type)), output, 45);
                    AddString(" ", output);
                    AddString(unrequiredString.c_str(), output, 4, true);
                    AddString(" / ", output);
                    AddString(totalString.c_str(), output, 4, true);
                    AddString("\n", output);
                }
            }
        }
    }

    void SceneDumper::AddString(const ramses_internal::String&       stringToAppend,
                                ramses_internal::StringOutputStream& string,
                                uint32_t                             width,
                                bool rightAlign)
    {
        if (!rightAlign)
        {
            string << stringToAppend;
        }

        const int32_t n = static_cast<int32_t>(width) - static_cast<int32_t>(stringToAppend.size());
        for (int32_t i = 0; i < n; i++)
        {
            string << " ";
        }

        if (rightAlign)
        {
            string << stringToAppend;
        }
    }


    void SceneDumper::outputNotRequiredObject(const RamsesObject&                  object,
                                              ramses_internal::StringOutputStream& outputStream)
    {
        outputStream << "Not required " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(object.getType());
        outputStream << " name: \"" << object.getName() << "\" handle: " << object.impl.getObjectRegistryHandle()
                     << "\n";

    }
}
