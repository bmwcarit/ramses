//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREXECUTORINTERNALSTATE_H
#define RAMSES_RENDEREXECUTORINTERNALSTATE_H

#include "Math3d/Vector3.h"
#include "Math3d/CameraMatrixHelper.h"
#include "SceneAPI/Handles.h"
#include "RendererAPI/SceneRenderExecutionIterator.h"
#include "RendererLib/FrameTimer.h"
#include "RenderExecutorInternalRenderStates.h"
#include "FrameBufferInfo.h"

namespace ramses_internal
{
    class IDevice;
    class RendererCachedScene;

    template <typename STATETYPE>
    struct CachedState
    {
    public:
        explicit CachedState(const STATETYPE& initialState = STATETYPE())
            : m_state(initialState)
            , m_changed(true)
        {
        }

        void setState(const STATETYPE& state)
        {
            m_changed = false;
            if (state != m_state)
            {
                m_state = state;
                m_changed = true;
            }
        }

        const STATETYPE& getState() const
        {
            return m_state;
        }

        Bool hasChanged() const
        {
            return m_changed;
        }

        void reset()
        {
            m_changed = true;
            m_state = STATETYPE();
        }

    private:
        STATETYPE m_state;
        Bool      m_changed;
    };

    // RenderExecutor is _stateless_, this only keeps an internal state during execution
    // to avoid copying it around
    class RenderExecutorInternalState
    {
    public:
        RenderExecutorInternalState(IDevice& device, const FrameBufferInfo& frameBuffer, const SceneRenderExecutionIterator& renderFrom = {}, const FrameTimer* frameTimer = nullptr);

        IDevice&                   getDevice() const;

        void                       setScene(const RendererCachedScene& scene);
        const RendererCachedScene& getScene() const;
        const FrameBufferInfo&     getFrameBufferInfo() const;

        const Matrix44f&           getProjectionMatrix() const;
        void                       setRendererViewMatrix(const Matrix44f& matrix);
        const Matrix44f&           getRendererViewMatrix() const;
        const Matrix44f&           getCameraViewMatrix() const;
        const Vector3&             getCameraWorldPosition() const;
        const Matrix44f&           getViewMatrix() const;
        const Matrix44f&           getModelMatrix() const;
        const Matrix44f&           getModelViewMatrix() const;
        const Matrix44f&           getModelViewProjectionMatrix() const;

        void                       setCamera(CameraHandle camera);

        void                       setRenderable(RenderableHandle renderable);
        RenderableHandle           getRenderable() const;

        Bool hasExceededTimeBudgetForRendering() const
        {
            return m_frameTimer != nullptr ? m_frameTimer->isTimeBudgetExceededForSection(EFrameTimerSectionBudget::OffscreenBufferRender) : false;
        }

        CachedState < DeviceResourceHandle >    shaderDeviceHandle;
        CachedState < DeviceResourceHandle >    indexBufferDeviceHandle;
        ScissorState                            scissorState;
        CachedState < DepthStencilState >       depthStencilState;
        CachedState < BlendState >              blendState;
        CachedState < RasterizerState >         rasterizerState;
        CachedState < RenderTargetHandle >      renderTargetState;
        CachedState < RenderPassHandle >        renderPassState;
        CachedState < Viewport >                viewportState;

        SceneRenderExecutionIterator            m_currentRenderIterator;

    private:
        IDevice&                    m_device;
        const RendererCachedScene*  m_scene;
        const FrameBufferInfo       m_frameBuffer;

        RenderableHandle            m_renderable;

        Matrix44f                   m_rendererVirtualViewMatrix;
        Matrix44f                   m_projectionMatrix;
        Matrix44f                   m_cameraViewMatrix;
        Matrix44f                   m_viewMatrix;
        Matrix44f                   m_modelMatrix;
        Matrix44f                   m_modelViewMatrix;
        Matrix44f                   m_modelViewProjectionMatrix;
        Vector3                     m_cameraWorldPosition;

        CachedState < CameraHandle >       m_camera;

        const FrameTimer* const            m_frameTimer;
    };
}

#endif
