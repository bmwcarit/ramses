//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BLITPASSSCENE_H
#define RAMSES_BLITPASSSCENE_H

#include "CommonRenderBufferTestScene.h"

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
    class BlitPassScene : public CommonRenderBufferTestScene
    {
    public:
        BlitPassScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            BLITS_COLOR_BUFFER = 0,
            BLITS_SUBREGION,
            BLITS_DEPTH_BUFFER,
            BLITS_DEPTH_STENCIL_BUFFER
        };

    private:
        ramses::RenderTarget& createRenderTarget(UInt32 state);
        ramses::RenderTarget& createBlittingRenderTarget(UInt32 state);

        void initClearPass(UInt32 state);
        void initRenderingPass(UInt32 state);
        void initClearPassForBlittingBuffers(UInt32 state);
        void initBlittingPass(UInt32 state);
        void initRenderPassFromBlittingResult(UInt32 state);

        ramses::RenderBuffer& m_colorBuffer;
        ramses::RenderBuffer& m_depthBuffer;
        ramses::RenderBuffer& m_depthStencilBuffer;

        ramses::RenderBuffer& m_blittingColorBuffer;
        ramses::RenderBuffer& m_blittingDepthBuffer;
        ramses::RenderBuffer& m_blittingDepthStencilBuffer;
    };
}

#endif
