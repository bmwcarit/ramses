//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ImguiClientHelper.h"
#include "ImguiImageCache.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "impl/RamsesObjectVector.h"
#include "impl/SceneDumper.h"
#include "ProgressMonitor.h"
#include "ResourceList.h"
#include <unordered_map>
#include <map>

namespace ramses
{
    class Scene;
    class RenderPass;
    class Camera;
    class RenderTarget;
    class Effect;
}

namespace ramses::internal
{
    class RenderGroupImpl;
    class NodeImpl;
    class MeshNodeImpl;
    class CameraNodeImpl;
    class SceneObjectImpl;
    class PickableObjectImpl;
    class SceneReferenceImpl;
    class AnimationSystemImpl;
    class BlitPassImpl;

    class TextureResource;
    struct DataSlot;
    struct MipMap;

    class ViewerGuiApp;
    struct ViewerSettings;

    class SceneViewerGui
    {
    public:
        SceneViewerGui(ViewerGuiApp& app, ramses::Scene& scene);
        void draw();
        void setSceneTexture(ramses::TextureSampler* sampler, uint32_t width, uint32_t height);

    private:
        [[nodiscard]] const ramses::RenderBuffer* findRenderBuffer(ramses::internal::RenderBufferHandle handle) const;
        [[nodiscard]] const ramses::Texture2DBuffer* findTextureBuffer(ramses::internal::TextureBufferHandle handle) const;
        [[nodiscard]] const ramses::TextureSampler* findTextureSampler(ramses::internal::TextureSamplerHandle handle) const;
        [[nodiscard]] const ramses::ArrayBuffer* findArrayBuffer(ramses::internal::DataBufferHandle handle) const;
        [[nodiscard]] const ramses::Node* findNode(ramses::internal::NodeHandle handle) const;
        [[nodiscard]] const ramses::DataObject* findDataObject(ramses::internal::DataInstanceHandle handle) const;
        [[nodiscard]] const ramses::Texture2D* findTexture2D(ramses::internal::ResourceContentHash hash) const;
        [[nodiscard]] const ramses::internal::DataSlot* findDataSlot(ramses::internal::NodeHandle handle) const;
        [[nodiscard]] const ramses::internal::DataSlot* findDataSlot(ramses::internal::DataInstanceHandle handle) const;
        [[nodiscard]] const ramses::internal::DataSlot* findDataSlot(ramses::internal::TextureSamplerHandle handle) const;
        [[nodiscard]] const ramses::internal::DataSlot* findDataSlot(ramses::internal::ResourceContentHash hash) const;

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
        void drawRefs(const char* headline, const SceneObjectImpl& target, Filter f);

        void draw(SceneObjectImpl& obj);
        void drawNode(NodeImpl& obj);
        void drawPickableObject(PickableObjectImpl& obj);
        void drawNodeChildrenParent(NodeImpl& obj);
        void drawMeshNode(MeshNodeImpl& obj);
        void drawCameraNode(CameraNodeImpl& obj);
        void drawRenderPass(RenderPassImpl& obj);
        void drawResource(ResourceImpl& obj);
        void drawEffect(EffectImpl& obj);
        void drawRenderGroup(RenderGroupImpl& obj);
        void drawRenderTarget(RenderTargetImpl& obj);
        void drawRenderBuffer(RenderBufferImpl& obj);
        void drawBlitPass(BlitPassImpl& obj);
        void drawAppearance(ramses::Appearance& appearance);
        void drawUniformValue(ramses::Appearance& appearance, const ramses::UniformInput& uniform);
        void drawGeometry(GeometryImpl& obj);
        void drawTexture2D(Texture2DImpl& obj);
        void drawTexture3D(Texture3DImpl& obj);
        void drawTexture2DBuffer(Texture2DBufferImpl& obj);
        void drawTextureCube(TextureCubeImpl& obj);
        void drawTextureSampler(TextureSamplerImpl& obj);
        void drawArrayResource(ArrayResourceImpl& obj);
        void drawArrayBuffer(ArrayBufferImpl& obj);
        void drawDataObject(ramses::DataObject& obj);
        static void DrawSceneReference(SceneReferenceImpl& obj);
        void drawDataSlot(const ramses::internal::DataSlot& obj);

        void drawFile();
        void drawSceneObjects();
        void drawSceneObjectsFilter();
        void drawNodeHierarchy();
        void drawResources();
        void drawRenderHierarchy();
        void drawErrors();

        [[nodiscard]] bool passFilter(const SceneObjectImpl& obj) const;

        bool drawRamsesObject(SceneObjectImpl& obj);
        static void DrawIssues(const SceneObjectImpl& obj, const ramses::ValidationReport& report);

        template<class C>
        bool drawRamsesObject(SceneObjectImpl& obj, const C& drawTreeNode);

        void drawUnusedObject(SceneObjectImpl& obj);

        static void LogRamsesObject(SceneObjectImpl& obj);
        void logResource(ResourceImpl& obj);
        void logTexture2D(Texture2DImpl& obj);

        void saveSceneToFile();
        [[nodiscard]] std::string saveTexture2D(const Texture2DImpl& obj) const;
        [[nodiscard]] std::string saveShaderSources(const EffectImpl& obj) const;

        void setVisibility(NodeImpl& node, ramses::EVisibilityMode visibility);

        // Stores RenderPasses, RenderGroups, MeshNodes in drawing order
        struct RenderInfo
        {
            SceneObjectVector renderPassVector;
            std::unordered_map<const SceneObjectImpl*, std::vector<const SceneObjectImpl*>> renderableMap;
            std::unordered_map<const SceneObjectImpl*, std::vector<const RenderGroupImpl*>> renderGroupMap;
        };

        struct NodeFilter
        {
            bool showOff = true;
            bool showVisible = true;
            bool showInvisible = true;
        };

        using SceneObjects = std::map<ramses::ERamsesObjectType, std::vector<ramses::SceneObject*>>;

        ImGuiTextFilter           m_filter;
        NodeFilter                m_nodeFilter;
        ramses::Scene&            m_scene;
        const ramses::ValidationReport& m_validationReport;
        SceneDumper::RamsesObjectImplSet m_usedObjects;
        const std::string         m_loadedSceneFile;
        std::string               m_filename;
        std::string               m_lastErrorMessage;
        SceneObjects              m_sceneObjects;
        std::unique_ptr<ResourceList> m_resourceInfo;
        RenderInfo                m_renderInfo;
        bool                      m_compressFile = false;
        bool                      m_alwaysOverwrite = false;
        bool                      m_nodeVisibilityChanged = false;

        ImguiImageCache m_imageCache;

        struct RefKey
        {
            const SceneObjectImpl* target;
            std::string headline;

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

        std::unordered_map<RefKey, SceneObjectVector, RefKeyHash> m_refs;

        ViewerSettings& m_settings;

        ProgressMonitor m_progress;

        ramses::TextureSampler* m_sceneTexture = nullptr;
        ImVec2 m_sceneTextureSize;
    };
}
