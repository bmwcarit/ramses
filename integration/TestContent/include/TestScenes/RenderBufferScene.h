//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERBUFFERSCENE_H
#define RAMSES_RENDERBUFFERSCENE_H

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
    class RenderBufferScene : public CommonRenderBufferTestScene
    {
    public:
        RenderBufferScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            ONE_COLOR_BUFFER_NO_DEPTH_OR_STENCIL = 0,
            ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_BUFFER,
            ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_STENCIL_BUFFER
        };

    private:
        ramses::RenderTarget& createRenderTarget(uint32_t state);

        void initClearPass(uint32_t state);
        void initRenderingPass(uint32_t state);

        ramses::RenderBuffer& m_readWriteColorRenderBuffer;
        ramses::RenderBuffer& m_writeOnlyDepthBuffer;
        ramses::RenderBuffer& m_writeOnlyDepthStencilBuffer;
    };
}

#endif
