//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STEREODISPLAYCONTROLLER_H
#define RAMSES_STEREODISPLAYCONTROLLER_H

#include "RendererLib/DisplayController.h"
#include "FrameBufferInfo.h"

namespace ramses_internal
{
    class StereoDisplayController : public DisplayController
    {
    public:
        explicit StereoDisplayController(IRenderBackend& rendererBackend);

        virtual void enableContext() override;

        virtual SceneRenderExecutionIterator renderScene(const RendererCachedScene& scene, DeviceResourceHandle buffer, const Viewport& viewport, const SceneRenderExecutionIterator& renderFrom = {}, const FrameTimer* frameTimer = nullptr) override;
        virtual void executePostProcessing() override;

        virtual void setProjectionParams(const ProjectionParams& params) override;

    protected:
        struct ViewInfo
        {
            FrameBufferInfo         m_fbInfo{ DeviceResourceHandle::Invalid(), ProjectionParams::Perspective(1.f, 1.f, 1.f, 2.f), {} };
            Vector3                 m_eyePosition{ 0.f };
            Matrix44f               m_viewMatrix;
        };
        ViewInfo m_viewInfo[2];
    };
}

#endif
