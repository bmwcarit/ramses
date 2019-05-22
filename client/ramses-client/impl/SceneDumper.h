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

    class VertexDataBufferImpl;
    class GeometryBindingImpl;
    class IndexDataBufferImpl;
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
        SceneDumper(const ramses::SceneImpl& scene);
        void dumpUnrequiredObjects(ramses_internal::StringOutputStream& output);
    private:
        typedef ramses_internal::HashSet<const GeometryBindingImpl*> GeometryBindingSet;
        typedef ramses_internal::HashSet<const TextureSamplerImpl*>  TextureSamplerSet;
        typedef ramses_internal::HashSet<const StreamTextureImpl*>   StreamTextureSet;
        typedef ramses_internal::HashSet<const RenderTargetImpl*>    RenderTargetSet;
        typedef ramses_internal::HashSet<const RenderBufferImpl*>    RenderBufferSet;
        typedef ramses_internal::HashSet<const RenderGroupImpl*>     RenderGroupSet;
        typedef ramses_internal::HashSet<const RenderPassImpl*>      RenderPassSet;
        typedef ramses_internal::HashSet<const AppearanceImpl*>      AppearanceSet;
        typedef ramses_internal::HashSet<const CameraNodeImpl*>      CameraNodeSet;
        typedef ramses_internal::HashSet<const MeshNodeImpl*>        MeshNodeSet;
        typedef ramses_internal::HashSet<const NodeImpl*>            NodeSet;

        typedef ramses_internal::HashSet<ramses_internal::ResourceContentHash> ResourceContentHashSet;
        typedef ramses_internal::HashSet<ramses_internal::RenderBufferHandle>  RenderBufferHandleSet;
        typedef ramses_internal::HashSet<RamsesObject*>                        RamsesObjectSet;


        typedef ramses_internal::HashMap<ramses_internal::TextureSamplerHandle, const TextureSamplerImpl*> TextureSamplerHandleToObjectMap;
        typedef ramses_internal::HashMap<ramses_internal::TextureBufferHandle, const Texture2DBufferImpl*> TextureBufferHandleToObjectMap;
        typedef ramses_internal::HashMap<ramses_internal::StreamTextureHandle, const StreamTextureImpl*> StreamTextureHandleToObjectMap;
        typedef ramses_internal::HashMap<ramses_internal::RenderBufferHandle, const RenderBufferImpl*> RenderBufferHandleToObjectMap;
        typedef ramses_internal::HashMap<ramses_internal::RenderBufferHandle, RenderBufferSet> RenderBufferHandleRenderBufferSetMap;
        typedef ramses_internal::HashMap<ramses_internal::ResourceContentHash, const ResourceImpl*> ResourceContentHashToObjectMap;
        typedef ramses_internal::HashMap<ramses_internal::RenderBufferHandle, RenderPassSet> RenderBufferHandleRenderPassSetMap;

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
        void               markRequiredResourcesFromHash(ResourceContentHashSet requiredBufferResourceHashes);

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
        ramses_internal::HashSet<const RamsesObjectImpl*> m_requiredObjects;

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
