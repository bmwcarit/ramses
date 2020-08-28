//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTIPLERENDERTARGETSCENE_H
#define RAMSES_MULTIPLERENDERTARGETSCENE_H

#include "CommonRenderBufferTestScene.h"
#include "Math3d/Vector4.h"

namespace ramses
{
    class RenderBuffer;
    class RenderTarget;
    class PerspectiveCamera;
    class Effect;
    class MeshNode;
}

namespace ramses_internal
{
    class MultipleRenderTargetScene : public CommonRenderBufferTestScene
    {
    public:
        MultipleRenderTargetScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            CLEAR_MRT = 0,
            TWO_COLOR_BUFFERS,
            TWO_COLOR_BUFFERS_RGBA8_AND_RGBA4,
            SHADER_WRITES_TWO_COLOR_BUFFERS_RT_HAS_ONE,
            SHADER_WRITES_ONE_COLOR_BUFFER_RT_HAS_TWO,
            COLOR_WRITTEN_BY_TWO_DIFFERENT_RTS,
            DEPTH_WRITTEN_AND_USED_BY_DIFFERENT_RT,
            DEPTH_WRITTEN_AND_READ
        };

    private:
        const ramses::Effect& getMRTEffect(UInt32 state);
        ramses::RenderTarget& createMRTRenderTarget(UInt32 state);
        ramses::RenderBuffer& initRenderBuffer(ramses::Scene& scene, UInt32 state);
        const ramses::MeshNode& createQuadWithTexture(const ramses::RenderBuffer& renderBuffer, const Vector3& translation, const Vector4& modulateColor = Vector4(1.f, 1.f, 1.f, 1.f));

        void initClearPass();
        void initMRTPass(UInt32 state);
        void initFinalRenderPass(UInt32 state);

        ramses::RenderBuffer& m_renderBuffer1;
        ramses::RenderBuffer& m_renderBuffer2;
        ramses::RenderBuffer& m_depthBuffer;
    };
}

#endif
