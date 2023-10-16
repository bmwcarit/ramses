//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "CommonRenderBufferTestScene.h"

namespace ramses::internal
{
    class MsaaRenderBufferScene : public CommonRenderBufferTestScene
    {
    public:
        MsaaRenderBufferScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            SAMPLE_COUNT_1_BLIT = 0,
            SAMPLE_COUNT_2_BLIT,
            SAMPLE_COUNT_4_BLIT,
            SAMPLE_COUNT_4_TEXEL_FETCH
        };

    private:
        ramses::RenderTarget&   createRenderTarget(uint32_t state);
        void                    initRenderPass(uint32_t state);
        void                    initBlittingPass(uint32_t state);
        ramses::MeshNode&       createMesh();
        const ramses::MeshNode& createQuadWithTextureMS(const ramses::RenderBuffer& renderBuffer);

        ramses::RenderBuffer& m_colorBufferMsaa1;
        ramses::RenderBuffer& m_colorBufferMsaa2;
        ramses::RenderBuffer& m_colorBufferMsaa4;
        ramses::RenderBuffer* m_colorBufferMsaa1ReadWrite = nullptr;
        ramses::RenderBuffer& m_colorBufferMsaa4ReadWrite;
        ramses::RenderBuffer& m_blittingColorBuffer;
    };
}
