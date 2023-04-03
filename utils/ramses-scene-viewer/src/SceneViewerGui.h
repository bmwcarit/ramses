//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_SCENEVIEWERGUI_H
#define RAMSES_SCENE_VIEWER_SCENEVIEWERGUI_H

#include "ImguiClientHelper.h"
#include "ImguiImageCache.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/ResourceContentHash.h"
#include "RamsesObjectVector.h"
#include "SceneDumper.h"
#include "ProgressMonitor.h"
#include "ResourceList.h"
#include <unordered_map>
#include <map>
#include <array>

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
        void draw();
        void setSceneTexture(ramses::TextureSampler* sampler, uint32_t width, uint32_t height);

    private:
        [[nodiscard]] const ramses::RenderBuffer* findRenderBuffer(ramses_internal::RenderBufferHandle handle) const;
        [[nodiscard]] const ramses::Texture2DBuffer* findTextureBuffer(ramses_internal::TextureBufferHandle handle) const;
        [[nodiscard]] const ramses::TextureSampler* findTextureSampler(ramses_internal::TextureSamplerHandle handle) const;
        [[nodiscard]] const ramses::ArrayBuffer* findArrayBuffer(ramses_internal::DataBufferHandle handle) const;
        [[nodiscard]] const ramses::Node* findNode(ramses_internal::NodeHandle handle) const;
        [[nodiscard]] const ramses::DataObject* findDataObject(ramses_internal::DataInstanceHandle) const;
        [[nodiscard]] const ramses::Texture2D* findTexture2D(ramses_internal::ResourceContentHash hash) const;
        [[nodiscard]] const ramses_internal::DataSlot* findDataSlot(ramses_internal::NodeHandle handle) const;
        [[nodiscard]] const ramses_internal::DataSlot* findDataSlot(ramses_internal::DataInstanceHandle handle) const;
        [[nodiscard]] const ramses_internal::DataSlot* findDataSlot(ramses_internal::TextureSamplerHandle handle) const;
        [[nodiscard]] const ramses_internal::DataSlot* findDataSlot(ramses_internal::ResourceContentHash hash) const;

        void zoomIn();
        void zoomOut();

        void drawInspectionWindow();
        void drawSceneTexture();
        void drawMenuBar();
        void drawMenuItemShowWindow();
        void drawMenuItemShowPreview();
        void drawMenuItemCopyTexture2D();
        void drawMenuItemStorePng();
        void drawMenuItemExportShaderSources();

        template <class F>
        void processObjectsAsync(F&& func, ramses::ERamsesObjectType objType, const char* message);

        /**
         * Draws all objects of type T that own a link to the target object
         */
        template<class T, class Filter>
        void drawRefs(const char* headline, const ramses::RamsesObjectImpl& target, Filter f);

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
        void drawAppearance(ramses::Appearance& appearance);
        void drawUniformValue(ramses::Appearance& appearance, ramses::UniformInput& uniform);
        void drawGeometryBinding(ramses::GeometryBindingImpl& obj);
        void drawTexture2D(ramses::Texture2DImpl& obj);
        void drawTexture3D(ramses::Texture3DImpl& obj);
        void drawTexture2DBuffer(ramses::Texture2DBufferImpl& obj);
        void drawTextureCube(ramses::TextureCubeImpl& obj);
        void drawTextureSampler(ramses::TextureSamplerImpl& obj);
        void drawArrayResource(ramses::ArrayResourceImpl& obj);
        void drawArrayBuffer(ramses::ArrayBufferImpl& obj);
        void drawDataObject(ramses::DataObjectImpl& obj);
        void drawSceneReference(ramses::SceneReferenceImpl& obj);
        void drawDataSlot(const ramses_internal::DataSlot& obj);

        void drawFile();
        void drawSceneObjects();
        void drawSceneObjectsFilter();
        void drawNodeHierarchy();
        void drawResources();
        void drawRenderHierarchy();
        void drawErrors();

        [[nodiscard]] bool passFilter(const ramses::RamsesObjectImpl& obj) const;

        bool drawRamsesObject(ramses::RamsesObjectImpl& obj);

        template<class C>
        bool drawRamsesObject(ramses::RamsesObjectImpl& obj, const C& drawTreeNode);

        void drawUnusedObject(ramses::RamsesObjectImpl& obj);

        void logRamsesObject(ramses::RamsesObjectImpl& obj);
        void logResource(ramses::ResourceImpl& obj);
        void logTexture2D(ramses::Texture2DImpl& obj);

        void saveSceneToFile();
        [[nodiscard]] std::string saveTexture2D(const ramses::Texture2DImpl& obj) const;
        [[nodiscard]] std::string saveShaderSources(const ramses::EffectImpl& obj) const;

        void setVisibility(ramses::NodeImpl& node, ramses::EVisibilityMode visibility);

        // Stores RenderPasses, RenderGroups, MeshNodes in drawing order
        struct RenderInfo
        {
            ramses::RamsesObjectVector renderPassVector;
            std::unordered_map<const ramses::RamsesObjectImpl*, std::vector<const ramses::RamsesObjectImpl*>> renderableMap;
            std::unordered_map<const ramses::RamsesObjectImpl*, std::vector<const ramses::RenderGroupImpl*>> renderGroupMap;
        };

        struct NodeFilter
        {
            bool showOff = true;
            bool showVisible = true;
            bool showInvisible = true;
        };

        using SceneObjects = std::map<ramses::ERamsesObjectType, std::vector<const ramses::RamsesObject*>>;

        ImGuiTextFilter           m_filter;
        NodeFilter                m_nodeFilter;
        ramses::Scene&            m_scene;
        ramses::SceneDumper::RamsesObjectImplSet m_usedObjects;
        const std::string         m_loadedSceneFile;
        std::string               m_filename;
        std::string               m_lastErrorMessage;
        SceneObjects              m_sceneObjects;
        std::unique_ptr<ResourceList> m_resourceInfo;
        RenderInfo                m_renderInfo;
        bool                      m_compressFile = false;
        bool                      m_alwaysOverwrite = false;
        bool                      m_hasSceneErrors = false;
        bool                      m_nodeVisibilityChanged = false;

        ImguiImageCache m_imageCache;

        struct RefKey
        {
            const ramses::RamsesObjectImpl* target;
            std::string               headline;

            bool operator==(const RefKey& rhs) const
            {
                return target == rhs.target && headline == rhs.headline;
            }
        };

        struct RefKeyHash
        {
            size_t operator()(const RefKey& key) const
            {
                // there are only few (usually 1-3) back reference categories (headlines) per target
                // so it's probably good enough to just hash the target
                return std::hash<const void*>()(key.target);
            }
        };

        std::unordered_map<RefKey, ramses::RamsesObjectVector, RefKeyHash> m_refs;

        ramses::RamsesObjectVector m_objectsWithErrors;

        struct Settings
        {
            bool showWindow = true;
            bool showPreview = false;
            const std::array<float, 6> zoomLevels = {(1.f / 4.f), (1.f / 3.f), (1.f / 2.f), (2.f / 3.f), (3.f / 4.f), 1.f};
            int zoomIx = 4;
        };
        Settings m_settings;

        ProgressMonitor m_progress;

        ramses::TextureSampler* m_sceneTexture = nullptr;
        ImVec2 m_sceneTextureSize;
    };
}

#endif
