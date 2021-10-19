//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEDUMPER_H
#define RAMSES_SCENEDUMPER_H

#include "Collections/StringOutputStream.h"
#include "RamsesObjectRegistryIterator.h"
#include "ramses-client-api/Scene.h"
#include "Collections/HashSet.h"
#include "Collections/HashMap.h"
#include "SceneAPI/Handles.h"

#include "TextureSamplerImpl.h"

namespace ramses
{
    class RamsesObjectRegistry;
    class RamsesObject;

    class ArrayBufferImpl;
    class GeometryBindingImpl;
    class Texture2DBufferImpl;
    class TextureSamplerImpl;
    class StreamTextureImpl;
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
        using RamsesObjectImplSet = ramses_internal::HashSet<const RamsesObjectImpl*>;
        explicit SceneDumper(const ramses::SceneImpl& scene);
        void dumpUnrequiredObjects(ramses_internal::StringOutputStream& output);
        const RamsesObjectImplSet& getRequiredObjects() const;
    private:
        using GeometryBindingSet = ramses_internal::HashSet<const GeometryBindingImpl *>;
        using TextureSamplerSet = ramses_internal::HashSet<const TextureSamplerImpl *>;
        using StreamTextureSet = ramses_internal::HashSet<const StreamTextureImpl *>;
        using RenderTargetSet = ramses_internal::HashSet<const RenderTargetImpl *>;
        using RenderBufferSet = ramses_internal::HashSet<const RenderBufferImpl *>;
        using RenderGroupSet = ramses_internal::HashSet<const RenderGroupImpl *>;
        using RenderPassSet = ramses_internal::HashSet<const RenderPassImpl *>;
        using AppearanceSet = ramses_internal::HashSet<const AppearanceImpl *>;
        using CameraNodeSet = ramses_internal::HashSet<const CameraNodeImpl *>;
        using MeshNodeSet = ramses_internal::HashSet<const MeshNodeImpl *>;
        using NodeSet = ramses_internal::HashSet<const NodeImpl *>;

        using ResourceContentHashSet = ramses_internal::HashSet<ramses_internal::ResourceContentHash>;
        using RenderBufferHandleSet = ramses_internal::HashSet<ramses_internal::RenderBufferHandle>;
        using RamsesObjectSet = ramses_internal::HashSet<RamsesObject *>;


        using TextureSamplerHandleToObjectMap = ramses_internal::HashMap<ramses_internal::TextureSamplerHandle, const TextureSamplerImpl *>;
        using TextureBufferHandleToObjectMap = ramses_internal::HashMap<ramses_internal::TextureBufferHandle, const Texture2DBufferImpl *>;
        using StreamTextureHandleToObjectMap = ramses_internal::HashMap<ramses_internal::StreamTextureHandle, const StreamTextureImpl *>;
        using RenderBufferHandleToObjectMap = ramses_internal::HashMap<ramses_internal::RenderBufferHandle, const RenderBufferImpl *>;
        using RenderBufferHandleRenderBufferSetMap = ramses_internal::HashMap<ramses_internal::RenderBufferHandle, RenderBufferSet>;
        using ResourceContentHashToObjectMap = ramses_internal::HashMap<ramses_internal::ResourceContentHash, const ResourceImpl *>;
        using RenderBufferHandleRenderPassSetMap = ramses_internal::HashMap<ramses_internal::RenderBufferHandle, RenderPassSet>;

        void setupMaps();

        template <class ObjectImplType, class HandleType>
        static HandleType GetHandle(const ObjectImplType& impl);

        template <class ObjectType, class ObjectImplType, class HandleType>
        void setupMap(ramses_internal::HashMap<HandleType, const ObjectImplType*>& map);

        void setupRenderBufferSetMap();
        void setupRenderPassMap();
        void setupResourceMap();

        bool addToRequiredObjects(const RamsesObjectImpl& object);
        void markAllObjectsOfTypeAsRequired(ERamsesObjectType objectType);

        RenderPassSet      markRequiredScreenRenderPasses();
        RenderPassSet      getRequiredRenderPasses(const RenderBufferSet& requiredRenderBuffers);
        RenderPassSet      getRequiredRenderPasses();
        RenderGroupSet     markRequiredRenderGroups(const RenderPassSet& requiredRenderPasses);
        MeshNodeSet        markRequiredMeshNodes(const RenderGroupSet& requiredRenderGroups);
        AppearanceSet      markRequiredAppearance(const MeshNodeSet& requiredMeshNodes);
        TextureSamplerSet  markRequiredTextureSampler(const AppearanceSet& requiredAppearances);
        void               markRequiredEffects(const AppearanceSet& requiredAppearances);
        void               markRequiredTextures(const TextureSamplerSet& requiredTextureSamplers);
        GeometryBindingSet markRequiredGeometryBindings(const MeshNodeSet& requiredMeshNodes);
        void               markRequiredVertexAndIndexBuffers(const GeometryBindingSet& requiredGeometryBindings);
        CameraNodeSet      markRequiredCameras(const RenderPassSet& requiredRenderPasses);
        void               markRequiredRenderTargets(const RenderPassSet& requiredRenderPasses);
        RenderBufferSet    markRequiredRenderBuffer(const TextureSamplerSet& requiredTextureSamplers);
        void               markRequiredTextureBuffer(const TextureSamplerSet& requiredTextureSamplers);
        StreamTextureSet   markRequiredStreamTextures(const TextureSamplerSet& requiredTextureSamplers);
        RenderBufferSet    getRequiredRenderBuffers(RenderBufferSet& requiredRenderBuffers);
        void               markRequiredResourcesFromHash(const ResourceContentHashSet& requiredResourceHashes);

        template <class T> void markAllParentNodesAsRequired(const T& requiredNodes);

        RamsesObjectSet getSceneAndClientObjects();
        void            outputNotRequiredObjects(ramses_internal::StringOutputStream& output);
        void outputNotRequiredObject(const RamsesObject& object, ramses_internal::StringOutputStream& outputStream);

        void addRenderGroupRecursive(const RenderGroupImpl& renderGroup, RenderGroupSet& requiredRenderGroups);
        void addNodeWithAllParentNodes(const NodeImpl* node);
        void addRenderBufferRecursive(const RenderBufferImpl& renderBuffer, RenderBufferSet& requiredRenderBuffers);

        static void AddString(const ramses_internal::String&       stringToAppend,
                              ramses_internal::StringOutputStream& string,
                              uint32_t                             width      = 0,
                              bool                                 rightAlign = false);

        const RamsesClientImpl&     m_client;
        const RamsesObjectRegistry& m_objectRegistry;
        RamsesObjectImplSet         m_requiredObjects;

        RenderBufferHandleRenderBufferSetMap              m_blitPassDestinationRenderBufferToSourceRenderBuffersMap;
        RenderBufferHandleRenderPassSetMap                m_destinationRenderBufferHandleToRenderPassSetMap;
        TextureSamplerHandleToObjectMap                   m_textureSamplerHandleToObjectMap;
        ResourceContentHashToObjectMap                    m_resourceContentHashToObjectMap;
        TextureBufferHandleToObjectMap                    m_textureBufferHandleToObjectMap;
        StreamTextureHandleToObjectMap                    m_streamTextureHandleToObjectMap;
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

#endif
