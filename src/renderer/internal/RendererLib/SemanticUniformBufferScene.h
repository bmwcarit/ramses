//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/TextureLinkCachedScene.h"
#include "internal/RendererLib/SemanticUniformBuffers.h"

namespace ramses::internal
{
    class IRendererResourceManager;

    class SemanticUniformBufferScene : public TextureLinkCachedScene
    {
        using BaseT = TextureLinkCachedScene;

    public:
        explicit SemanticUniformBufferScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        void                        preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;
        void                        setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        CameraHandle                allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle) override;
        void                        releaseCamera(CameraHandle cameraHandle) override;

        DeviceResourceHandle        getDeviceHandle(RenderableHandle renderable) const;
        DeviceResourceHandle        getDeviceHandle(CameraHandle camera) const;
        DeviceResourceHandle        getDeviceHandle(RenderableHandle renderable, CameraHandle camera) const;

        void                        collectDirtySemanticUniformBuffers(const RenderableVector& renderableHandles, CameraHandle camera);
        void                        updateSemanticUniformBuffers();
        void                        uploadSemanticUniformBuffers(IRendererResourceManager& resourceManager);

        using CameraProjectionParams = std::pair<vec4f, vec2f>;// frustum planes (vec4), frustum near and far (vec2)
        const CameraProjectionParams& updateCameraProjectionParams(CameraHandle camera) const;

    private:
        CameraProjectionParams      getCameraProjection(const Camera& camera) const;
        inline bool                 checkRenderableDirtiness(const Renderable& renderable) const;
        inline bool                 checkCameraDirtiness(CameraHandle cameraHandle) const;

        struct ActiveSemantics
        {
            bool model = false;
            bool modelCamera = false;
        };
        std::unordered_map<RenderableHandle, ActiveSemantics> m_renderableSemantics;
        mutable MemoryPoolExplicit<CameraProjectionParams, CameraHandle> m_cameraProjectionParamsCache;

        SemanticUniformBuffers_Model m_semanticUBOsModel;
        SemanticUniformBuffers_Camera m_semanticUBOsCamera;
        SemanticUniformBuffers_ModelCamera m_semanticUBOsModelCamera;
    };
}
