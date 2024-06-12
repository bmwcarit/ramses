//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SemanticUniformBufferScene.h"
#include "internal/RendererLib/IRendererResourceManager.h"
#include "internal/Core/Math3d/CameraMatrixHelper.h"
#include "ramses/framework/EFeatureLevel.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    SemanticUniformBufferScene::SemanticUniformBufferScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : BaseT(sceneLinksManager, sceneInfo)
        , m_semanticUBOsModel{ sceneInfo.sceneID }
        , m_semanticUBOsCamera{ sceneInfo.sceneID }
        , m_semanticUBOsModelCamera{ sceneInfo.sceneID }
    {
    }

    void SemanticUniformBufferScene::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        BaseT::preallocateSceneSize(sizeInfo);
        m_cameraProjectionParamsCache.preallocateSize(sizeInfo.cameraCount);
    }

    void SemanticUniformBufferScene::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        BaseT::setRenderableDataInstance(renderableHandle, slot, newDataInstance);
        if (slot == ERenderableDataSlotType::ERenderableDataSlotType_Uniforms)
        {
            ActiveSemantics activeSemantics{ false, false };
            if (newDataInstance.isValid())
            {
                const auto dataLayout = getDataLayout(getLayoutOfDataInstance(newDataInstance));
                for (DataFieldHandle field{ 0u }; field < dataLayout.getFieldCount(); field++)
                {
                    activeSemantics.model |= (dataLayout.getField(field).semantics == EFixedSemantics::ModelBlock);
                    activeSemantics.modelCamera |= (dataLayout.getField(field).semantics == EFixedSemantics::ModelCameraBlock);
                }
            }

            m_renderableSemantics[renderableHandle] = activeSemantics;
        }
    }

    CameraHandle SemanticUniformBufferScene::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle)
    {
        const auto newHandle = BaseT::allocateCamera(type, nodeHandle, dataInstance, handle);
        m_cameraProjectionParamsCache.allocate(newHandle);
        return newHandle;
    }

    void SemanticUniformBufferScene::releaseCamera(CameraHandle cameraHandle)
    {
        m_cameraProjectionParamsCache.release(cameraHandle);
        BaseT::releaseCamera(cameraHandle);
    }

    const SemanticUniformBufferScene::CameraProjectionParams& SemanticUniformBufferScene::updateCameraProjectionParams(CameraHandle cameraHandle) const
    {
        // store in cache for next dirtiness check
        const auto& camera = getCamera(cameraHandle);
        CameraProjectionParams& cachedParams = *m_cameraProjectionParamsCache.getMemory(cameraHandle);
        cachedParams = getCameraProjection(camera);
        return cachedParams;
    }

    DeviceResourceHandle SemanticUniformBufferScene::getDeviceHandle(RenderableHandle renderable) const
    {
        return m_semanticUBOsModel.getDeviceHandle(SemanticUniformBufferHandle{ renderable });
    }

    DeviceResourceHandle SemanticUniformBufferScene::getDeviceHandle(CameraHandle camera) const
    {
        return m_semanticUBOsCamera.getDeviceHandle(SemanticUniformBufferHandle{ camera });
    }

    DeviceResourceHandle SemanticUniformBufferScene::getDeviceHandle(RenderableHandle renderable, CameraHandle camera) const
    {
        return m_semanticUBOsModelCamera.getDeviceHandle(SemanticUniformBufferHandle{ renderable, camera });
    }

    SemanticUniformBufferScene::CameraProjectionParams SemanticUniformBufferScene::getCameraProjection(const Camera& camera) const
    {
        const auto planesDataInstance = getDataReference(camera.dataInstance, Camera::FrustumPlanesField);
        const auto nearFarDataInstance = getDataReference(camera.dataInstance, Camera::FrustumNearFarPlanesField);
        return CameraProjectionParams{ getDataSingleVector4f(planesDataInstance, DataFieldHandle{0u}), getDataSingleVector2f(nearFarDataInstance, DataFieldHandle{0u}) };
    }

    bool SemanticUniformBufferScene::checkRenderableDirtiness(const Renderable& renderable) const
    {
        return renderable.visibilityMode == EVisibilityMode::Visible
            && getMatrixCacheEntry(renderable.node).m_matrixDirty[ETransformationMatrixType_World];
    }

    bool SemanticUniformBufferScene::checkCameraDirtiness(CameraHandle cameraHandle) const
    {
        const auto& camera = getCamera(cameraHandle);
        return getMatrixCacheEntry(camera.node).m_matrixDirty[ETransformationMatrixType_Object]
            || getCameraProjection(camera) != *m_cameraProjectionParamsCache.getMemory(cameraHandle);
    }

    void SemanticUniformBufferScene::collectDirtySemanticUniformBuffers(const RenderableVector& renderableHandles, CameraHandle camera)
    {
        // here we get all renderables with camera (per render pass) that will be rendered in this frame
        if (renderableHandles.empty())
            return;

        const SemanticUniformBufferHandle cameraUboHandle{ camera };
        m_semanticUBOsCamera.markAsUsed(cameraUboHandle);
        const bool camDirty = checkCameraDirtiness(camera);
        if (camDirty)
            m_semanticUBOsCamera.markDirty(cameraUboHandle);

        for (auto renderableHandle : renderableHandles)
        {
            const auto renderableSemantics = m_renderableSemantics[renderableHandle];
            if (!renderableSemantics.model && !renderableSemantics.modelCamera)
                continue;

            const auto& renderable = getRenderable(renderableHandle);
            const bool renderableDirty = checkRenderableDirtiness(renderable);

            if (renderableSemantics.model)
            {
                const SemanticUniformBufferHandle uboHandle{ renderableHandle };
                m_semanticUBOsModel.markAsUsed(uboHandle);
                if (renderableDirty)
                    m_semanticUBOsModel.markDirty(uboHandle);
            }

            if (renderableSemantics.modelCamera)
            {
                const SemanticUniformBufferHandle uboHandle{ renderableHandle, camera };
                m_semanticUBOsModelCamera.markAsUsed(uboHandle);
                if (camDirty || renderableDirty)
                    m_semanticUBOsModelCamera.markDirty(uboHandle);
            }
        }
    }

    void SemanticUniformBufferScene::updateSemanticUniformBuffers()
    {
        m_semanticUBOsModel.update(*this);
        m_semanticUBOsCamera.update(*this);
        m_semanticUBOsModelCamera.update(*this);
    }

    void SemanticUniformBufferScene::uploadSemanticUniformBuffers(IRendererResourceManager& resourceManager)
    {
        m_semanticUBOsModel.upload(resourceManager);
        m_semanticUBOsCamera.upload(resourceManager);
        m_semanticUBOsModelCamera.upload(resourceManager);
    }
}
