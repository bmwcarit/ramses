//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_SCENEVIEWERGUI_H
#define RAMSES_SCENE_VIEWER_SCENEVIEWERGUI_H

#include "Utils/ImguiClientHelper.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/ResourceContentHash.h"
#include "RamsesObjectVector.h"
#include "SceneDumper.h"
#include <unordered_map>

namespace ramses
{
    class Scene;
    class RenderPass;
    class Camera;
    class RenderGroupImpl;
    class NodeImpl;
    class MeshNodeImpl;
    class CameraNodeImpl;
    class RenderTarget;
    class RamsesObjectImpl;
    class Effect;
    class PickableObjectImpl;
    class SceneReferenceImpl;
    class AnimationSystemImpl;
    class BlitPassImpl;
}

namespace ramses_internal
{
    class TextureResource;
    struct DataSlot;

    class SceneViewerGui
    {
    public:
        SceneViewerGui(ramses::Scene& scene, const std::string& filename, ImguiClientHelper& imguiHelper);
        void drawFrame();

    private:
        const ramses::RenderBuffer* findRenderBuffer(ramses_internal::RenderBufferHandle handle) const;
        const ramses::StreamTexture* findStreamTexture(ramses_internal::StreamTextureHandle handle) const;
        const ramses::Texture2DBuffer* findTextureBuffer(ramses_internal::TextureBufferHandle handle) const;
        const ramses::TextureSampler* findTextureSampler(ramses_internal::TextureSamplerHandle handle) const;
        const ramses::ArrayBuffer* findDataBuffer(ramses_internal::DataBufferHandle handle) const;
        const ramses::Node* findNode(ramses_internal::NodeHandle handle) const;
        const ramses::DataObject* findDataObject(ramses_internal::DataInstanceHandle) const;
        const ramses::Texture2D* findTexture2D(ramses_internal::ResourceContentHash hash) const;
        const ramses_internal::DataSlot* findDataSlot(ramses_internal::NodeHandle handle) const;
        const ramses_internal::DataSlot* findDataSlot(ramses_internal::DataInstanceHandle handle) const;
        const ramses_internal::DataSlot* findDataSlot(ramses_internal::TextureSamplerHandle handle) const;
        const ramses_internal::DataSlot* findDataSlot(ramses_internal::ResourceContentHash hash) const;

        template<class T, class Filter>
        void drawAll(const char* headline, Filter f);

        void draw(ramses::RamsesObjectImpl& obj);
        void drawNode(ramses::NodeImpl& obj);
        void drawPickableObject(ramses::PickableObjectImpl& obj);
        void drawNodeChildrenParent(ramses::NodeImpl& obj);
        void drawMeshNode(ramses::MeshNodeImpl& obj);
        void drawCameraNode(ramses::CameraNodeImpl& obj);
        void drawRenderPass(ramses::RenderPassImpl& obj);
        void drawResource(ramses::ResourceImpl& obj);
        void drawEffect(ramses::EffectImpl& obj);
        void drawRenderGroup(ramses::RenderGroupImpl& obj);
        void drawRenderTarget(ramses::RenderTargetImpl& obj);
        void drawRenderBuffer(ramses::RenderBufferImpl& obj);
        void drawBlitPass(ramses::BlitPassImpl& obj);
        void drawAppearance(ramses::AppearanceImpl& obj);
        void drawUniformValue(ramses::AppearanceImpl& obj, ramses::UniformInput& uniform);
        void drawGeometryBinding(ramses::GeometryBindingImpl& obj);
        void drawTexture2D(ramses::Texture2DImpl& obj);
        void drawTexture3D(ramses::Texture3DImpl& obj);
        void drawTexture2DBuffer(ramses::Texture2DBufferImpl& obj);
        void drawTextureCube(ramses::TextureCubeImpl& obj);
        void drawStreamTexture(ramses::StreamTextureImpl& obj);
        void drawTextureSampler(ramses::TextureSamplerImpl& obj);
        void drawArrayResource(ramses::ArrayResourceImpl& obj);
        void drawArrayBuffer(ramses::ArrayBufferImpl& obj);
        void drawDataObject(ramses::DataObjectImpl& obj);
        void drawSceneReference(ramses::SceneReferenceImpl& obj);
        void drawDataSlot(const ramses_internal::DataSlot& obj);

        void drawFile();
        void drawSceneObjects();
        void drawNodeHierarchy();
        void drawResources();
        void drawRenderHierarchy();
        void drawErrors();

        bool drawRamsesObject(ramses::RamsesObjectImpl& obj) const;
        template<class C>
        bool drawRamsesObject(ramses::RamsesObjectImpl& obj, const C& drawTreeNode) const;

        void saveSceneToFile();

        struct ResourceInfo
        {
            ramses::RamsesObjectVector objects;
            uint32_t compressedSize = 0U;
            uint32_t decompressedSize = 0U;
            uint32_t unavailable = 0U;
        };

        // Stores RenderPasses, RenderGroups, MeshNodes in drawing order
        struct RenderInfo
        {
            ramses::RamsesObjectVector renderPassVector;
            std::unordered_map<const ramses::RamsesObjectImpl*, std::vector<const ramses::RamsesObjectImpl*>> renderableMap;
            std::unordered_map<const ramses::RamsesObjectImpl*, std::vector<const ramses::RenderGroupImpl*>> renderGroupMap;
        };

        ImguiClientHelper&        m_imguiHelper;
        ImGuiTextFilter           m_filter;
        ramses::Scene&            m_scene;
        ramses::SceneDumper::RamsesObjectImplSet m_usedObjects;
        const std::string         m_loadedSceneFile;
        std::string               m_filename;
        std::string               m_lastErrorMessage;
        ResourceInfo              m_resourceInfo;
        RenderInfo                m_renderInfo;
        bool                      m_compressFile = false;
        bool                      m_alwaysOverwrite = false;
        bool                      m_hasSceneErrors = false;

        struct PreviewInfo
        {
            ramses::TextureSampler* sampler;
            uint32_t                width;
            uint32_t                height;
        };
        std::unordered_map<const ramses_internal::TextureResource*, PreviewInfo> m_texturePreview;
    };
}

#endif
