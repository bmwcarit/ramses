//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RenderExecutorInternalState.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/RenderingContext.h"

namespace ramses::internal
{
    RenderExecutorInternalState::RenderExecutorInternalState(IDevice& device, RenderingContext& renderContext, const FrameTimer* frameTimer)
        : m_currentRenderIterator(renderContext.renderFrom)
        , m_device(device)
        , m_renderContext(renderContext)
        , m_frameTimer(frameTimer)
    {
        // Currently framebuffer is default render target and invalid RT handle is used to refer to it.
        // For that reason the cached state here needs to be set to another 'invalid' so it can properly
        // track state change. (In case framebuffer is used as first RT after initialization)
        renderTargetState.setState(RenderTargetHandle::Invalid() - 1u);

        assert(!(m_renderContext.displayBufferDepthDiscard && m_renderContext.renderFrom != SceneRenderExecutionIterator{})
            && "Discard depth not supported for incremental rendering (interruptible buffers)!");
    }

    void RenderExecutorInternalState::setCamera(CameraHandle camera)
    {
        assert(camera.isValid());
        m_camera.setState(camera);

        if (m_camera.hasChanged())
        {
            const Camera& cameraData = m_scene->getCamera(camera);

            m_viewMatrix = m_scene->updateMatrixCacheWithLinks(ETransformationMatrixType_Object, cameraData.node);

            const auto position = glm::inverse(m_viewMatrix) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            m_cameraWorldPosition = position;

            Viewport newViewport;
            const auto vpOffsetRef = m_scene->getDataReference(cameraData.dataInstance, Camera::ViewportOffsetField);
            const auto vpSizeRef = m_scene->getDataReference(cameraData.dataInstance, Camera::ViewportSizeField);
            const auto& vpOffset = m_scene->getDataSingleVector2i(vpOffsetRef, DataFieldHandle{ 0 });
            const auto& vpSize = m_scene->getDataSingleVector2i(vpSizeRef, DataFieldHandle{ 0 });
            newViewport = Viewport{ vpOffset.x, vpOffset.y, uint32_t(vpSize.x), uint32_t(vpSize.y) };

            const auto frustumPlanesRef = m_scene->getDataReference(cameraData.dataInstance, Camera::FrustumPlanesField);
            const auto frustumNearFarRef = m_scene->getDataReference(cameraData.dataInstance, Camera::FrustumNearFarPlanesField);
            const auto& frustumPlanes = m_scene->getDataSingleVector4f(frustumPlanesRef, DataFieldHandle{ 0 });
            const auto& frustumNearFar = m_scene->getDataSingleVector2f(frustumNearFarRef, DataFieldHandle{ 0 });
            m_projectionMatrix = CameraMatrixHelper::ProjectionMatrix(
                ProjectionParams::Frustum(cameraData.projectionType, frustumPlanes.x, frustumPlanes.y, frustumPlanes.z, frustumPlanes.w, frustumNearFar.x, frustumNearFar.y));

            viewportState.setState(newViewport);
        }
    }

    void RenderExecutorInternalState::setRenderable(RenderableHandle renderable)
    {
        m_renderable = renderable;
        m_modelMatrix = m_scene->getRenderableWorldMatrix(renderable);
        m_modelViewMatrix = m_viewMatrix * m_modelMatrix;
        m_modelViewProjectionMatrix = m_projectionMatrix * m_modelViewMatrix;
    }
}
