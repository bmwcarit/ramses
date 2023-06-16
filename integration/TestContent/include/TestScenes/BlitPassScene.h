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
        BlitPassScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            BLITS_COLOR_BUFFER = 0,
            BLITS_SUBREGION,
            BLITS_DEPTH_BUFFER,
            BLITS_DEPTH_STENCIL_BUFFER
        };

    private:
        ramses::RenderTarget& createRenderTarget(uint32_t state);
        ramses::RenderTarget& createBlittingRenderTarget(uint32_t state);

        void initClearPass(uint32_t state);
        void initRenderingPass(uint32_t state);
        void initClearPassForBlittingBuffers(uint32_t state);
        void initBlittingPass(uint32_t state);
        void initRenderPassFromBlittingResult(uint32_t state);

        ramses::RenderBuffer& m_colorBuffer;
        ramses::RenderBuffer& m_depthBuffer;
        ramses::RenderBuffer& m_depthStencilBuffer;

        ramses::RenderBuffer& m_blittingColorBuffer;
        ramses::RenderBuffer& m_blittingDepthBuffer;
        ramses::RenderBuffer& m_blittingDepthStencilBuffer;
    };
}

#endif
