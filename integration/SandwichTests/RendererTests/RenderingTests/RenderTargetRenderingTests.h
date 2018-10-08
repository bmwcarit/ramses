//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERTARGETRENDERINGTESTS_H
#define RAMSES_RENDERTARGETRENDERINGTESTS_H

#include "IRendererTest.h"

class RenderTargetRenderingTests : public IRendererTest
{
public:
    virtual void setUpTestCases(RendererTestsFramework& testFramework) final;
    virtual bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    template <typename INTEGRATION_SCENE>
    bool runBasicTest(RendererTestsFramework& testFramework, ramses_internal::UInt32 sceneState, const ramses_internal::String& expectedImageName, float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel);

    enum
    {
        RenderTarget_Perspective,
        RenderTarget_Orthographic,

        RenderTarget_Format_R8,
        RenderTarget_Format_RG8,
        RenderTarget_Format_RGB8,
        RenderTarget_Format_R16F,
        RenderTarget_Format_R32F,
        RenderTarget_Format_RG16F,
        RenderTarget_Format_RG32F,
        RenderTarget_Format_RGB16F,
        RenderTarget_Format_RGB32F,
        RenderTarget_Format_RGBA16F,
        RenderTarget_Format_RGBA32F,

        MultipleRenderTarget_TwoColorBuffersCleared,
        MultipleRenderTarget_TwoColorBuffersWritten,
        MultipleRenderTarget_ShaderWritesTwoColorBuffers_RTHasOnlyOne,
        MultipleRenderTarget_ReuseSameColorBufferInTwoRTs,
        MultipleRenderTarget_ReuseSameDepthBufferInTwoRTs,
        MultipleRenderTarget_ReadFromDepth,

        RenderBuffer_OneColorBufferNoDepthOrStencil,
        RenderBuffer_OneColorBufferWithWriteOnlyDepthBuffer,
        RenderBuffer_OneColorBufferWithWriteOnlyDepthStencilBuffer,
        RenderBuffer_MsaaSampleCount2,
        RenderBuffer_MsaaSampleCount4
    };
};

#endif
