//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderExecutorInternalState.h"
#include "RendererLib/RendererCachedScene.h"

namespace ramses_internal
{
    RenderExecutorInternalState::RenderExecutorInternalState(IDevice& device, const FrameBufferInfo& frameBuffer, const SceneRenderExecutionIterator& renderFrom, const FrameTimer* frameTimer)
        : viewportState(Viewport(std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max()))
        , m_currentRenderIterator(renderFrom)
        , m_device(device)
        , m_scene(0)
        , m_frameBuffer(frameBuffer)
        , m_rendererVirtualViewMatrix(Matrix44f::Identity)
        , m_projectionMatrix(Matrix44f::Identity)
        , m_cameraWorldPosition(0.0f)
        , m_frameTimer(frameTimer)
    {
        // Currently framebuffer is default render target and invalid RT handle is used to refer to it.
        // For that reason the cached state here needs to be set to another 'invalid' so it can properly
        // track state change. (In case framebuffer is used as first RT after initialization)
        renderTargetState.setState(RenderTargetHandle::Invalid() - 1u);
    }

    IDevice& RenderExecutorInternalState::getDevice() const
    {
        return m_device;
    }

    void RenderExecutorInternalState::setScene(const RendererCachedScene& scene)
    {
        m_scene = &scene;
    }

    const RendererCachedScene& RenderExecutorInternalState::getScene() const
    {
        assert(0 != m_scene);
        return *m_scene;
    }

    const FrameBufferInfo& RenderExecutorInternalState::getFrameBufferInfo() const
    {
        return m_frameBuffer;
    }

    const Matrix44f& RenderExecutorInternalState::getProjectionMatrix() const
    {
        return m_projectionMatrix;
    }

    void RenderExecutorInternalState::setRendererViewMatrix(const Matrix44f& matrix)
    {
        m_rendererVirtualViewMatrix = matrix;
    }

    const Matrix44f& RenderExecutorInternalState::getRendererViewMatrix() const
    {
        return m_rendererVirtualViewMatrix;
    }

    const Matrix44f& RenderExecutorInternalState::getCameraViewMatrix() const
    {
        return m_cameraViewMatrix;
    }

    const Vector3& RenderExecutorInternalState::getCameraWorldPosition() const
    {
        return m_cameraWorldPosition;
    }

    const Matrix44f& RenderExecutorInternalState::getViewMatrix() const
    {
        return m_viewMatrix;
    }

    const Matrix44f& RenderExecutorInternalState::getModelMatrix() const
    {
        return m_modelMatrix;
    }

    const Matrix44f& RenderExecutorInternalState::getModelViewMatrix() const
    {
        return m_modelViewMatrix;
    }

    const Matrix44f& RenderExecutorInternalState::getModelViewProjectionMatrix() const
    {
        return m_modelViewProjectionMatrix;
    }

    void RenderExecutorInternalState::setCamera(CameraHandle camera)
    {
        assert(camera.isValid());
        m_camera.setState(camera);

        if (m_camera.hasChanged())
        {
            const Camera& cameraData = m_scene->getCamera(camera);

            m_cameraViewMatrix = m_scene->updateMatrixCacheWithLinks(ETransformationMatrixType_Object, cameraData.node);
            m_viewMatrix = m_rendererVirtualViewMatrix * m_cameraViewMatrix;

            const Vector4 position = m_viewMatrix.inverse() * Vector4(0.0f, 0.0f, 0.0f, 1.0f);
            m_cameraWorldPosition = Vector3(position.x, position.y, position.z);

            Viewport newViewport;
            if (cameraData.projectionType == ECameraProjectionType_Renderer)
            {
                newViewport = m_frameBuffer.viewport;
                assert(!renderTargetState.getState().isValid());
                m_projectionMatrix = CameraMatrixHelper::ProjectionMatrix(m_frameBuffer.projectionParams);
            }
            else
            {
                const auto vpOffsetRef = m_scene->getDataReference(cameraData.viewportDataInstance, Camera::ViewportOffsetField);
                const auto vpSizeRef = m_scene->getDataReference(cameraData.viewportDataInstance, Camera::ViewportSizeField);
                const auto vpOffset = m_scene->getDataSingleVector2i(vpOffsetRef, DataFieldHandle{ 0 });
                const auto vpSize = m_scene->getDataSingleVector2i(vpSizeRef, DataFieldHandle{ 0 });
                newViewport = Viewport{ vpOffset.x, vpOffset.y, UInt32(vpSize.x), UInt32(vpSize.y) };
                m_projectionMatrix = CameraMatrixHelper::ProjectionMatrix(
                    ProjectionParams::Frustum(
                        cameraData.projectionType,
                        cameraData.frustum.leftPlane,
                        cameraData.frustum.rightPlane,
                        cameraData.frustum.bottomPlane,
                        cameraData.frustum.topPlane,
                        cameraData.frustum.nearPlane,
                        cameraData.frustum.farPlane));
            }

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

    RenderableHandle RenderExecutorInternalState::getRenderable() const
    {
        return m_renderable;
    }
}
