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
#include "SceneAPI/Viewport.h"
#include "RendererAPI/SceneRenderExecutionIterator.h"
#include "RendererAPI/Types.h"
#include "RendererLib/FrameTimer.h"
#include "RenderExecutorInternalRenderStates.h"

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

    struct TargetBufferInfo
    {
        DeviceResourceHandle deviceHandle;
        uint32_t width;
        uint32_t height;
    };

    // RenderExecutor is _stateless_, this only keeps an internal state during execution
    // to avoid copying it around
    class RenderExecutorInternalState
    {
    public:
        RenderExecutorInternalState(IDevice& device, const TargetBufferInfo& bufferInfo, const SceneRenderExecutionIterator& renderFrom = {}, const FrameTimer* frameTimer = nullptr);

        IDevice&                   getDevice() const;

        void                       setScene(const RendererCachedScene& scene);
        const RendererCachedScene& getScene() const;
        const TargetBufferInfo&    getTargetBufferInfo() const;

        const Matrix44f&           getProjectionMatrix() const;
        const Vector3&             getCameraWorldPosition() const;
        const Matrix44f&           getViewMatrix() const;
        const Matrix44f&           getModelMatrix() const;
        const Matrix44f&           getModelViewMatrix() const;
        const Matrix44f&           getModelViewProjectionMatrix() const;

        void                       setCamera(CameraHandle camera);

        void                       setRenderable(RenderableHandle renderable);
        RenderableHandle           getRenderable() const;

        Bool hasExceededTimeBudgetForRendering() const;

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
        const TargetBufferInfo      m_targetBufferInfo;

        RenderableHandle            m_renderable;

        Matrix44f                   m_projectionMatrix;
        Matrix44f                   m_viewMatrix;
        Matrix44f                   m_modelMatrix;
        Matrix44f                   m_modelViewMatrix;
        Matrix44f                   m_modelViewProjectionMatrix;
        Vector3                     m_cameraWorldPosition;

        CachedState < CameraHandle >       m_camera;

        const FrameTimer* const            m_frameTimer;
    };

    inline RenderableHandle RenderExecutorInternalState::getRenderable() const
    {
        return m_renderable;
    }

    inline IDevice& RenderExecutorInternalState::getDevice() const
    {
        return m_device;
    }

    inline void RenderExecutorInternalState::setScene(const RendererCachedScene& scene)
    {
        m_scene = &scene;
    }

    inline const RendererCachedScene& RenderExecutorInternalState::getScene() const
    {
        assert(nullptr != m_scene);
        return *m_scene;
    }

    inline const TargetBufferInfo& RenderExecutorInternalState::getTargetBufferInfo() const
    {
        return m_targetBufferInfo;
    }

    inline const Matrix44f& RenderExecutorInternalState::getProjectionMatrix() const
    {
        return m_projectionMatrix;
    }

    inline const Vector3& RenderExecutorInternalState::getCameraWorldPosition() const
    {
        return m_cameraWorldPosition;
    }

    inline const Matrix44f& RenderExecutorInternalState::getViewMatrix() const
    {
        return m_viewMatrix;
    }

    inline const Matrix44f& RenderExecutorInternalState::getModelMatrix() const
    {
        return m_modelMatrix;
    }

    inline const Matrix44f& RenderExecutorInternalState::getModelViewMatrix() const
    {
        return m_modelViewMatrix;
    }

    inline const Matrix44f& RenderExecutorInternalState::getModelViewProjectionMatrix() const
    {
        return m_modelViewProjectionMatrix;
    }

    inline bool RenderExecutorInternalState::hasExceededTimeBudgetForRendering() const
    {
        return m_frameTimer != nullptr ? m_frameTimer->isTimeBudgetExceededForSection(EFrameTimerSectionBudget::OffscreenBufferRender) : false;
    }
}

#endif
