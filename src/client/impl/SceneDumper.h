//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "ramses/client/Scene.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include "impl/TextureSamplerImpl.h"

#include <string_view>

namespace ramses
{
    class RamsesObject;
}

namespace ramses::internal
{
    class SceneObjectRegistry;
    class ArrayBufferImpl;
    class GeometryImpl;
    class Texture2DBufferImpl;
    class TextureSamplerImpl;
    class TextureBufferImpl;
    class RamsesObjectImpl;
    class RamsesClientImpl;
    class RenderTargetImpl;
    class RenderBufferImpl;
    class RenderGroupImpl;
    class EffectInputImpl;
    class RenderPassImpl;
    class AppearanceImpl;
    class CameraNodeImpl;
    class MeshNodeImpl;
    class ResourceImpl;
    class EffectImpl;
    class NodeImpl;

    class SceneDumper
    {
    public:
        using RamsesObjectImplSet = ramses::internal::HashSet<const RamsesObjectImpl*>;
        explicit SceneDumper(const SceneImpl& scene);
        void dumpUnrequiredObjects(ramses::internal::StringOutputStream& output);
        [[nodiscard]] const RamsesObjectImplSet& getRequiredObjects() const;
    private:
        using GeometrySet = ramses::internal::HashSet<const GeometryImpl *>;
        using TextureSamplerSet = ramses::internal::HashSet<const TextureSamplerImpl *>;
        using RenderTargetSet = ramses::internal::HashSet<const RenderTargetImpl *>;
        using RenderBufferSet = ramses::internal::HashSet<const RenderBufferImpl *>;
        using RenderGroupSet = ramses::internal::HashSet<const RenderGroupImpl *>;
        using RenderPassSet = ramses::internal::HashSet<const RenderPassImpl *>;
        using AppearanceSet = ramses::internal::HashSet<const AppearanceImpl *>;
        using CameraNodeSet = ramses::internal::HashSet<const CameraNodeImpl *>;
        using MeshNodeSet = ramses::internal::HashSet<const MeshNodeImpl *>;
        using NodeSet = ramses::internal::HashSet<const NodeImpl *>;

        using ResourceContentHashSet = ramses::internal::HashSet<ramses::internal::ResourceContentHash>;
        using RenderBufferHandleSet = ramses::internal::HashSet<ramses::internal::RenderBufferHandle>;
        using RamsesObjectSet = ramses::internal::HashSet<RamsesObject *>;


        using TextureSamplerHandleToObjectMap = ramses::internal::HashMap<ramses::internal::TextureSamplerHandle, const TextureSamplerImpl *>;
        using TextureBufferHandleToObjectMap = ramses::internal::HashMap<ramses::internal::TextureBufferHandle, const Texture2DBufferImpl *>;
        using RenderBufferHandleToObjectMap = ramses::internal::HashMap<ramses::internal::RenderBufferHandle, const RenderBufferImpl *>;
        using RenderBufferHandleRenderBufferSetMap = ramses::internal::HashMap<ramses::internal::RenderBufferHandle, RenderBufferSet>;
        using ResourceContentHashToObjectMap = ramses::internal::HashMap<ramses::internal::ResourceContentHash, const ResourceImpl *>;
        using RenderBufferHandleRenderPassSetMap = ramses::internal::HashMap<ramses::internal::RenderBufferHandle, RenderPassSet>;

        void setupMaps();

        template <class ObjectImplType, class HandleType>
        static HandleType GetHandle(const ObjectImplType& impl);

        template <class ObjectType, class ObjectImplType, class HandleType>
        void setupMap(ramses::internal::HashMap<HandleType, const ObjectImplType*>& map);

        template <class ObjectType, class ObjectImplType, class HandleType>
        void addToMap(ramses::internal::HashMap<HandleType, const ObjectImplType*>& map);

        void setupRenderBufferSetMap();
        void setupRenderPassMap();
        void setupResourceMap();

        bool addToRequiredObjects(const RamsesObjectImpl& object);

        void               markRequiredLogic();
        RenderPassSet      markRequiredScreenRenderPasses();
        RenderPassSet      getRequiredRenderPasses(const RenderBufferSet& requiredRenderBuffers);
        RenderPassSet      getRequiredRenderPasses();
        RenderGroupSet     markRequiredRenderGroups(const RenderPassSet& requiredRenderPasses);
        MeshNodeSet        markRequiredMeshNodes(const RenderGroupSet& requiredRenderGroups);
        AppearanceSet      markRequiredAppearance(const MeshNodeSet& requiredMeshNodes);
        TextureSamplerSet  markRequiredTextureSampler(const AppearanceSet& requiredAppearances);
        void               markRequiredEffects(const AppearanceSet& requiredAppearances);
        void               markRequiredTextures(const TextureSamplerSet& requiredTextureSamplers);
        GeometrySet        markRequiredGeometries(const MeshNodeSet& requiredMeshNodes);
        void               markRequiredVertexAndIndexBuffers(const GeometrySet& requiredGeometries);
        CameraNodeSet      markRequiredCameras(const RenderPassSet& requiredRenderPasses);
        void               markRequiredRenderTargets(const RenderPassSet& requiredRenderPasses);
        RenderBufferSet    markRequiredRenderBuffer(const TextureSamplerSet& requiredTextureSamplers);
        void               markRequiredTextureBuffer(const TextureSamplerSet& requiredTextureSamplers);
        RenderBufferSet    getRequiredRenderBuffers(RenderBufferSet& requiredRenderBuffers);
        void               markRequiredResourcesFromHash(const ResourceContentHashSet& requiredResourceHashes);

        template <class T> void markAllParentNodesAsRequired(const T& requiredNodes);

        RamsesObjectSet getSceneAndClientObjects();
        void            outputNotRequiredObjects(ramses::internal::StringOutputStream& output);
        static void OutputNotRequiredObject(const RamsesObject& object, ramses::internal::StringOutputStream& outputStream);

        void addRenderGroupRecursive(const RenderGroupImpl& renderGroup, RenderGroupSet& requiredRenderGroups);
        void addNodeWithAllParentNodes(const NodeImpl* node);
        void addRenderBufferRecursive(const RenderBufferImpl& renderBuffer, RenderBufferSet& requiredRenderBuffers);

        static void AddString(std::string_view                     stringToAppend,
                              ramses::internal::StringOutputStream& string,
                              uint32_t                             width      = 0,
                              bool                                 rightAlign = false);

        const RamsesClientImpl&    m_client;
        const SceneObjectRegistry& m_objectRegistry;
        RamsesObjectImplSet        m_requiredObjects;

        RenderBufferHandleRenderBufferSetMap              m_blitPassDestinationRenderBufferToSourceRenderBuffersMap;
        RenderBufferHandleRenderPassSetMap                m_destinationRenderBufferHandleToRenderPassSetMap;
        TextureSamplerHandleToObjectMap                   m_textureSamplerHandleToObjectMap;
        ResourceContentHashToObjectMap                    m_resourceContentHashToObjectMap;
        TextureBufferHandleToObjectMap                    m_textureBufferHandleToObjectMap;
        RenderBufferHandleToObjectMap                     m_renderBufferHandleToObjectMap;
    };

    template <class T>
    void SceneDumper::markAllParentNodesAsRequired(const T& requiredNodes)
    {
        for (auto node : requiredNodes)
        {
            addNodeWithAllParentNodes(node->getParentImpl());
        }
    }
}
