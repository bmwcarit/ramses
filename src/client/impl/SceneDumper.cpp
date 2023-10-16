//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SceneDumper.h"

#include "ramses/framework/RamsesObjectTypes.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/SceneObject.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Resource.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Camera.h"

#include "impl/Texture2DBufferImpl.h"
#include "impl/GeometryImpl.h"
#include "impl/Texture2DBufferImpl.h"
#include "ObjectIteratorImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/RenderTargetImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/AppearanceImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/CameraNodeImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/ResourceImpl.h"
#include "impl/BlitPassImpl.h"
#include "impl/EffectImpl.h"
#include "impl/SceneImpl.h"

#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/BlitPass.h"

namespace ramses::internal
{
    template <>
    ramses::internal::TextureSamplerHandle SceneDumper::GetHandle(const TextureSamplerImpl& impl)
    {
        return impl.getTextureSamplerHandle();
    }

    template <> ramses::internal::RenderBufferHandle SceneDumper::GetHandle(const RenderBufferImpl& impl)
    {
        return impl.getRenderBufferHandle();
    }

    template <> ramses::internal::TextureBufferHandle SceneDumper::GetHandle(const Texture2DBufferImpl& impl)
    {
        return impl.getTextureBufferHandle();
    }

    template <class ObjectType, class ObjectImplType, class HandleType>
    void SceneDumper::setupMap(ramses::internal::HashMap<HandleType, const ObjectImplType*>& map)
    {
        map.clear();
        addToMap<ObjectType, ObjectImplType, HandleType>(map);
    }

    template <class ObjectType, class ObjectImplType, class HandleType>
    void SceneDumper::addToMap(ramses::internal::HashMap<HandleType, const ObjectImplType*>& map)
    {
        SceneObjectRegistryIterator iterator(m_objectRegistry, TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID);
        while (const auto* object = iterator.getNext<ObjectType>())
        {
            map.put(GetHandle<ObjectImplType, HandleType>(object->impl()), &object->impl());
        }
    }

    SceneDumper::SceneDumper(const SceneImpl& scene)
        : m_client(scene.getClientImpl())
        , m_objectRegistry(scene.getObjectRegistry())
    {
    }

    void SceneDumper::dumpUnrequiredObjects(ramses::internal::StringOutputStream& output)
    {
        setupMaps();

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
            GeometrySet requiredGeometries = markRequiredGeometries(requiredMeshNodes);
            markRequiredVertexAndIndexBuffers(requiredGeometries);
            markRequiredRenderTargets(requiredRenderPasses);
            RenderBufferSet requiredRenderBuffers = markRequiredRenderBuffer(requiredTextureSamplers);
            markRequiredTextureBuffer(requiredTextureSamplers);

            requiredRenderPasses = getRequiredRenderPasses(requiredRenderBuffers);
        }

        outputNotRequiredObjects(output);
    }

    const SceneDumper::RamsesObjectImplSet& SceneDumper::getRequiredObjects() const
    {
        return m_requiredObjects;
    }

    void SceneDumper::setupMaps()
    {
        setupMap<ramses::TextureSampler, TextureSamplerImpl, ramses::internal::TextureSamplerHandle>(m_textureSamplerHandleToObjectMap);
        addToMap<TextureSamplerMS, TextureSamplerImpl, ramses::internal::TextureSamplerHandle>(m_textureSamplerHandleToObjectMap);
        addToMap<TextureSamplerExternal, TextureSamplerImpl, ramses::internal::TextureSamplerHandle>(m_textureSamplerHandleToObjectMap);
        setupMap<ramses::RenderBuffer, RenderBufferImpl, ramses::internal::RenderBufferHandle>(m_renderBufferHandleToObjectMap);
        setupMap<Texture2DBuffer, Texture2DBufferImpl, ramses::internal::TextureBufferHandle>(m_textureBufferHandleToObjectMap);

        setupResourceMap();
        setupRenderPassMap();
        setupRenderBufferSetMap();
    }

    void SceneDumper::setupResourceMap()
    {
        m_resourceContentHashToObjectMap.clear();

        ObjectIteratorImpl iter(m_objectRegistry, ERamsesObjectType::Resource);
        auto resourceAsObject = iter.getNext();
        while (resourceAsObject)
        {
            const ResourceImpl* resource = &RamsesObjectTypeUtils::ConvertTo<Resource>(*resourceAsObject).impl();
            m_resourceContentHashToObjectMap.put(resource->getLowlevelResourceHash(), resource);
            resourceAsObject = iter.getNext();
        }
    }

    void SceneDumper::setupRenderPassMap()
    {
        m_destinationRenderBufferHandleToRenderPassSetMap.clear();
        SceneObjectRegistryIterator renderPassIterator(m_objectRegistry, ERamsesObjectType::RenderPass);
        while (const auto* renderPass = renderPassIterator.getNext<ramses::RenderPass>())
        {
            const ramses::RenderTarget* renderTarget = renderPass->getRenderTarget();
            if (nullptr != renderTarget)
            {
                const ramses::internal::ClientScene& clientScene = renderTarget->impl().getIScene();

                const ramses::internal::RenderTargetHandle renderTargetHandle =
                    renderTarget->impl().getRenderTargetHandle();
                uint32_t renderBufferCount = clientScene.getRenderTargetRenderBufferCount(renderTargetHandle);

                for (uint32_t i = 0; i < renderBufferCount; i++)
                {
                    const ramses::internal::RenderBufferHandle renderBufferHandle =
                        clientScene.getRenderTargetRenderBuffer(renderTargetHandle, i);

                    m_destinationRenderBufferHandleToRenderPassSetMap[renderBufferHandle].put(&renderPass->impl());
                }
            }
        }
    }

    void SceneDumper::setupRenderBufferSetMap()
    {
        m_blitPassDestinationRenderBufferToSourceRenderBuffersMap.clear();
        SceneObjectRegistryIterator blitPassIterator(m_objectRegistry, ERamsesObjectType::BlitPass);
        while (const auto* blitPass = blitPassIterator.getNext<ramses::BlitPass>())
        {
            const ramses::internal::ClientScene& clientScene = blitPass->impl().getIScene();
            const ramses::internal::BlitPassHandle blitPassHandle = blitPass->impl().getBlitPassHandle();
            const ramses::internal::BlitPass& blitPassData = clientScene.getBlitPass(blitPassHandle);

            const RenderBufferImpl** sourceRenderBuffer = m_renderBufferHandleToObjectMap.get(blitPassData.sourceRenderBuffer);
            if (nullptr != sourceRenderBuffer)
            {
                m_blitPassDestinationRenderBufferToSourceRenderBuffersMap[blitPassData.destinationRenderBuffer].put(
                    *sourceRenderBuffer);
            }
            else
            {
                LOG_ERROR(ramses::internal::CONTEXT_CLIENT,
                          "SceneDumper::setupRenderBufferSetMap Could not lookup render buffer handle: "
                              << blitPassData.sourceRenderBuffer << " !!!");
                assert(false);
            }
        }
    }

    bool SceneDumper::addToRequiredObjects(const RamsesObjectImpl& object)
    {
        if (!m_requiredObjects.contains(&object))
        {
            m_requiredObjects.put(&object);
            return true;
        }
        return false;
    }

    SceneDumper::RenderPassSet SceneDumper::markRequiredScreenRenderPasses()
    {
        RenderPassSet requiredRenderPasses;

        SceneObjectRegistryIterator objectIterator(m_objectRegistry, ERamsesObjectType::RenderPass);
        while (const auto* renderPass = objectIterator.getNext<ramses::RenderPass>())
        {
            if (renderPass->isEnabled() && nullptr == renderPass->getRenderTarget())
            {
                if (addToRequiredObjects(renderPass->impl()))
                {
                    requiredRenderPasses.put(&renderPass->impl());
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
            ramses::internal::DataInstanceHandle uniformHandle = appearance->getUniformDataInstance();
            const ramses::internal::ClientScene& scene         = appearance->getIScene();
            ramses::internal::DataLayoutHandle   layoutHandle  = scene.getLayoutOfDataInstance(uniformHandle);
            uint32_t                            fieldCount    = scene.getDataLayout(layoutHandle).getFieldCount();
            for (uint32_t field = 0; field < fieldCount; field++)
            {
                ramses::internal::DataFieldHandle fieldHandle(field);
                const auto fieldDataType = scene.getDataLayout(layoutHandle).getField(fieldHandle).dataType;
                if (IsTextureSamplerType(fieldDataType))
                {
                    ramses::internal::TextureSamplerHandle textureSamplerHandle =
                        scene.getDataTextureSamplerHandle(uniformHandle, fieldHandle);

                    if (ramses::internal::TextureSamplerHandle::Invalid() != textureSamplerHandle)
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
                                ramses::internal::CONTEXT_CLIENT,
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
        ramses::internal::HashSet<ramses::internal::ResourceContentHash> requiredResourceHashes;
        for (auto appearance : requiredAppearances)
        {
            const EffectImpl* effect = appearance->getEffectImpl();
            if (effect)
            {
                requiredResourceHashes.put(effect->getLowlevelResourceHash());
            }
        }
        markRequiredResourcesFromHash(requiredResourceHashes);
    }

    void SceneDumper::markRequiredTextures(const TextureSamplerSet& requiredTextureSamplers)
    {
        ramses::internal::HashSet<ramses::internal::ResourceContentHash> requiredTextureResourceHashes;
        for (auto textureSampler : requiredTextureSamplers)
        {
            ramses::internal::TextureSamplerHandle textureSamplerHandle = textureSampler->getTextureSamplerHandle();
            if (ramses::internal::TextureSamplerHandle::Invalid() != textureSamplerHandle)
            {
                ramses::internal::ResourceContentHash textureResourceHash =
                    textureSampler->getIScene().getTextureSampler(textureSamplerHandle).textureResource;
                if (ramses::internal::ResourceContentHash::Invalid() != textureResourceHash)
                {
                    requiredTextureResourceHashes.put(textureResourceHash);
                }
            }
        }
        markRequiredResourcesFromHash(requiredTextureResourceHashes);
    }

    SceneDumper::GeometrySet SceneDumper::markRequiredGeometries(const MeshNodeSet& requiredMeshNodes)
    {
        SceneDumper::GeometrySet requiredGeometries;
        for (auto meshNode : requiredMeshNodes)
        {
            const GeometryImpl* geometryBinding = meshNode->getGeometryImpl();

            if (nullptr != geometryBinding)
            {
                if (addToRequiredObjects(*geometryBinding))
                {
                    requiredGeometries.put(geometryBinding);
                }
            }
        }
        return requiredGeometries;
    }

    void SceneDumper::markRequiredVertexAndIndexBuffers(const GeometrySet& requiredGeometries)
    {
        ResourceContentHashSet requiredBufferResourceHashes;
        for (auto geometryBinding : requiredGeometries)
        {
            const ramses::internal::ClientScene& iscene             = geometryBinding->getIScene();
            ramses::internal::DataInstanceHandle dataInstanceHandle = geometryBinding->getAttributeDataInstance();

            ramses::internal::DataLayoutHandle layoutHandle = geometryBinding->getAttributeDataLayout();

            if (ramses::internal::DataLayoutHandle::Invalid() != layoutHandle)
            {
                const uint32_t fieldCount = iscene.getDataLayout(layoutHandle).getFieldCount();
                for (ramses::internal::DataFieldHandle field(0); field < fieldCount; ++field)
                {
                    const ramses::internal::ResourceField& dataResource =
                        iscene.getDataResource(dataInstanceHandle, field);

                    ramses::internal::ResourceContentHash bufferResourceHash = dataResource.hash;
                    if (ramses::internal::ResourceContentHash::Invalid() != bufferResourceHash)
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
            const ramses::Camera* camera = renderPass->getCamera();
            if (nullptr != camera)
            {
                if (addToRequiredObjects(camera->impl()))
                {
                    requiredCameras.put(&camera->impl());
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
                LOG_ERROR(ramses::internal::CONTEXT_CLIENT,
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
            const ramses::RenderTarget* renderTarget = renderPass->getRenderTarget();
            if (nullptr != renderTarget)
            {
                addToRequiredObjects(renderTarget->impl());
            }
        }
    }

    SceneDumper::RenderBufferSet SceneDumper::markRequiredRenderBuffer(const TextureSamplerSet& requiredTextureSamplers)
    {
        RenderBufferSet requiredRenderBuffers;

        for (auto textureSampler : requiredTextureSamplers)
        {
            const auto sampler = textureSampler->getIScene().getTextureSampler(textureSampler->getTextureSamplerHandle());
            if (sampler.isRenderBuffer() && ramses::internal::RenderBufferHandle::Invalid() != sampler.contentHandle)
            {
                const ramses::internal::RenderBufferHandle renderBufferHandle(sampler.contentHandle);
                const RenderBufferImpl** renderBuffer = m_renderBufferHandleToObjectMap.get(renderBufferHandle);

                if (nullptr != renderBuffer)
                {
                    assert(nullptr != *renderBuffer);
                    addRenderBufferRecursive(**renderBuffer, requiredRenderBuffers);
                }
                else
                {
                    LOG_ERROR(ramses::internal::CONTEXT_CLIENT,
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
        ramses::internal::HashSet<ramses::internal::ResourceContentHash> requiredTextureResourceHashes;
        for (auto textureSampler : requiredTextureSamplers)
        {
            const auto sampler = textureSampler->getIScene().getTextureSampler(textureSampler->getTextureSamplerHandle());

            if (ramses::internal::TextureSampler::ContentType::TextureBuffer == sampler.contentType && ramses::internal::TextureBufferHandle::Invalid() != sampler.contentHandle)
            {
                const ramses::internal::TextureBufferHandle textureBufferHandle(sampler.contentHandle);

                const Texture2DBufferImpl** textureBuffer = m_textureBufferHandleToObjectMap.get(textureBufferHandle);

                if (nullptr != textureBuffer)
                {
                    assert(nullptr != *textureBuffer);
                    addToRequiredObjects(**textureBuffer);
                }
                else
                {
                    LOG_ERROR(ramses::internal::CONTEXT_CLIENT,
                              "SceneDumper::getRequiredTextureBuffers Could not lookup texture buffer handle: "
                                  << textureBufferHandle << " !!!");
                    assert(false);
                }
            }
        }
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
                                              ramses::internal::HashSet<const RenderGroupImpl*>& requiredRenderGroups)
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
        RamsesObjectSet objects;
        SceneObjectVector sceneObjects;
        m_objectRegistry.getObjectsOfType(sceneObjects, ERamsesObjectType::RamsesObject);
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

    void SceneDumper::outputNotRequiredObjects(ramses::internal::StringOutputStream& output)
    {
        RamsesObjectSet objects = getSceneAndClientObjects();
        output << "SceneDumper::outputNotRequiredObjects Unrequired objects:\n";

        ramses::internal::HashMap<ERamsesObjectType, Counter> typeStatistic;
        for (RamsesObject* object : objects)
        {
            const RamsesObjectImpl* objectImpl = &object->impl();
            ERamsesObjectType type = object->getType();
            if (false == m_requiredObjects.contains(objectImpl))
            {
                OutputNotRequiredObject(*object, output);
                typeStatistic[type].unrequired++;
            }
            typeStatistic[type].total++;
        }

        output << "\nSceneDumper::outputNotRequiredObjects Statistic of unrequired objects:\n\n";

        AddString("OBEJCT TYPE", output, 39);

        AddString("#UNREQUIRED / #TOTAL\n\n", output);
        for (uint32_t type = 0; type < static_cast<uint32_t>(RamsesObjectTypeCount); type++)
        {
            if (RamsesObjectTypeUtils::IsConcreteType(ERamsesObjectType(type)))
            {
                Counter counter = typeStatistic[ERamsesObjectType(type)];
                if (counter.total > 0)
                {

                    ramses::internal::StringOutputStream unrequiredString;
                    unrequiredString << counter.unrequired;
                    ramses::internal::StringOutputStream totalString;
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

    void SceneDumper::AddString(std::string_view                     stringToAppend,
                                ramses::internal::StringOutputStream& string,
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

    void SceneDumper::OutputNotRequiredObject(const RamsesObject& object, ramses::internal::StringOutputStream& outputStream)
    {
        outputStream << "Not required " << object.impl().getIdentificationString();
    }
}
