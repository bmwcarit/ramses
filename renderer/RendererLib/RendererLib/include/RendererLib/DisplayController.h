//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYCONTROLLER_H
#define RAMSES_DISPLAYCONTROLLER_H

#include "RendererAPI/IDisplayController.h"
#include "Math3d/Vector3.h"
#include "Math3d/CameraMatrixHelper.h"
#include "RendererLib/Postprocessing.h"
#include "EmbeddedCompositingManager.h"
#include <memory>


namespace ramses_internal
{
    class RendererConfig;
    class IRenderBackend;
    class UpdateScene;
    class RendererCachedScene;
    class VirtualUpdateScene;
    class IDevice;

    class DisplayController : public IDisplayController
    {
        friend class RendererLogger;

    public:
        DisplayController(IRenderBackend& renderer, UInt32 samples = 1, UInt32 postProcessingEffectIds = EPostProcessingEffect_None);

        virtual void                    handleWindowEvents() override;
        virtual Bool                    canRenderNewFrame() const override;
        virtual void                    enableContext() override;
        virtual void                    swapBuffers() override;
        virtual SceneRenderExecutionIterator renderScene(const RendererCachedScene& scene, DeviceResourceHandle buffer, const Viewport& viewport, const SceneRenderExecutionIterator& renderFrom = {}, const FrameTimer* frameTimer = nullptr) override;
        virtual void                    executePostProcessing() override;
        virtual void                    clearBuffer(DeviceResourceHandle buffer, const Vector4& clearColor) override;

        virtual DeviceResourceHandle    getDisplayBuffer() const override final;
        virtual IRenderBackend&         getRenderBackend() const override;
        virtual IEmbeddedCompositingManager& getEmbeddedCompositingManager() override;
        virtual UInt32                  getDisplayWidth() const override;
        virtual UInt32                  getDisplayHeight() const override;
        virtual void                    setProjectionParams(const ProjectionParams& params) override;
        virtual const ProjectionParams& getProjectionParams() const override;

        virtual void                    setViewPosition(const Vector3& position) override;
        virtual void                    setViewRotation(const Vector3& rotation) override;
        virtual const Vector3&          getViewPosition() const override;
        virtual const Vector3&          getViewRotation() const override;
        virtual const Matrix44f&        getViewMatrix() const override;
        virtual void                    resetView() const override;

        virtual Bool                    readPixels(UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut) override;
        virtual Bool                    isWarpingEnabled() const override;
        virtual void                    setWarpingMeshData(const WarpingMeshData& warpingMeshData) override;

        virtual void validateRenderingStatusHealthy() const override;

    private:
        void updateViewMatrix();

        IRenderBackend&         m_renderBackend;
        IDevice&                m_device;
        EmbeddedCompositingManager m_embeddedCompositingManager;

        Vector3                 m_viewPosition;
        Vector3                 m_viewRotation;
        Matrix44f               m_viewMatrix;
        ProjectionParams        m_projectionParams;

        const UInt32            m_displayWidth;
        const UInt32            m_displayHeight;

        std::unique_ptr<Postprocessing> m_postProcessing;
    };
}

#endif
