//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MSAARENDERBUFFERSCENE_H
#define RAMSES_MSAARENDERBUFFERSCENE_H

#include "CommonRenderBufferTestScene.h"

namespace ramses_internal
{
    class MsaaRenderBufferScene : public CommonRenderBufferTestScene
    {
    public:
        MsaaRenderBufferScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            SAMPLE_COUNT_2 = 0,
            SAMPLE_COUNT_4
        };

    private:
        ramses::RenderTarget&   createRenderTarget(UInt32 state);
        ramses::RenderTarget&   createBlittingRenderTarget(UInt32 state);
        void                    initRenderingPass(UInt32 state);
        void                    initBlittingPass(UInt32 state);
        ramses::MeshNode&       createMesh();

        ramses::RenderBuffer& m_colorBufferMsaa2;
        ramses::RenderBuffer& m_colorBufferMsaa4;
        ramses::RenderBuffer& m_blittingColorBuffer;
    };
}

#endif
