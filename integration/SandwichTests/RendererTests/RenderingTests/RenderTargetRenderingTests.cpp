//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderTargetRenderingTests.h"
#include "TestScenes/RenderTargetScene.h"
#include "TestScenes/RenderBufferScene.h"
#include "TestScenes/MsaaRenderBufferScene.h"
#include "TestScenes/MultipleRenderTargetScene.h"

using namespace ramses_internal;

void RenderTargetRenderingTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Perspective, *this, "RenderTarget_Perspective");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Orthographic, *this, "RenderTarget_Orthographic");

    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_R8, *this, "RenderTarget_Format_R8");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RG8, *this, "RenderTarget_Format_RG8");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RGB8, *this, "RenderTarget_Format_RGB8");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_R16F, *this, "RenderTarget_Format_R16F");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_R32F, *this, "RenderTarget_Format_R32F");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RG16F, *this, "RenderTarget_Format_RG16F");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RG32F, *this, "RenderTarget_Format_RG32F");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RGB16F, *this, "RenderTarget_Format_RGB16F");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RGB32F, *this, "RenderTarget_Format_RGB32F");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RGBA16F, *this, "RenderTarget_Format_RGBA16F");
    testFramework.createTestCaseWithDefaultDisplay(RenderTarget_Format_RGBA32F, *this, "RenderTarget_Format_RGBA32F");

    testFramework.createTestCaseWithDefaultDisplay(MultipleRenderTarget_TwoColorBuffersCleared, *this, "MultipleRenderTarget_TwoColorBuffersCleared");
    testFramework.createTestCaseWithDefaultDisplay(MultipleRenderTarget_TwoColorBuffersWritten, *this, "MultipleRenderTarget_TwoColorBuffersWritten");
    testFramework.createTestCaseWithDefaultDisplay(MultipleRenderTarget_ShaderWritesTwoColorBuffers_RTHasOnlyOne, *this, "MultipleRenderTarget_ShaderWritesTwoColorBuffers_RTHasOnlyOne");
    testFramework.createTestCaseWithDefaultDisplay(MultipleRenderTarget_ReuseSameColorBufferInTwoRTs, *this, "MultipleRenderTarget_ReuseSameColorBufferInTwoRTs");
    testFramework.createTestCaseWithDefaultDisplay(MultipleRenderTarget_ReuseSameDepthBufferInTwoRTs, *this, "MultipleRenderTarget_ReuseSameDepthBufferInTwoRTs");
    testFramework.createTestCaseWithDefaultDisplay(MultipleRenderTarget_ReadFromDepth, *this, "MultipleRenderTarget_ReadFromDepth");

    testFramework.createTestCaseWithDefaultDisplay(RenderBuffer_OneColorBufferNoDepthOrStencil, *this, "RenderBuffer_OneColorBufferNoDepthOrStencil");
    testFramework.createTestCaseWithDefaultDisplay(RenderBuffer_OneColorBufferWithWriteOnlyDepthBuffer, *this, "RenderBuffer_OneColorBufferWithWriteOnlyDepthBuffer");
    testFramework.createTestCaseWithDefaultDisplay(RenderBuffer_OneColorBufferWithWriteOnlyDepthStencilBuffer, *this, "RenderBuffer_OneColorBufferWithWriteOnlyDepthStencilBuffer");
    testFramework.createTestCaseWithDefaultDisplay(RenderBuffer_MsaaSampleCount2, *this, "RenderBuffer_MsaaSampleCount2");
    testFramework.createTestCaseWithDefaultDisplay(RenderBuffer_MsaaSampleCount4, *this, "RenderBuffer_MsaaSampleCount4");
}

bool RenderTargetRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    switch (testCase.m_id)
    {
    case RenderTarget_Perspective:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::PERSPECTIVE_PROJECTION, "RenderTargetScene_PerspectiveProjection");
    case RenderTarget_Orthographic:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::ORTHOGRAPHIC_PROJECTION, "RenderTargetScene_OrthographicProjection");

    case RenderTarget_Format_R8:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_R8, "RenderTargetScene_Red");
    case RenderTarget_Format_RG8:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RG8, "RenderTargetScene_RedGreen");
    case RenderTarget_Format_RGB8:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RGB8, "RenderTargetScene_RedGreenBlue_NoAlpha");
    case RenderTarget_Format_R16F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_R16F, "RenderTargetScene_Red");
    case RenderTarget_Format_R32F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_R32F, "RenderTargetScene_Red");
    case RenderTarget_Format_RG16F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RG16F, "RenderTargetScene_RedGreen");
    case RenderTarget_Format_RG32F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RG32F, "RenderTargetScene_RedGreen");
    case RenderTarget_Format_RGB16F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RGB16F, "RenderTargetScene_RedGreenBlue_NoAlpha");
    case RenderTarget_Format_RGB32F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RGB32F, "RenderTargetScene_RedGreenBlue");
    case RenderTarget_Format_RGBA16F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RGBA16F, "RenderTargetScene_RedGreenBlue", 0.35f); // higher threshold needed due to floating formats get sampled differently on different platforms
    case RenderTarget_Format_RGBA32F:
        return runBasicTest<RenderTargetScene>(testFramework, RenderTargetScene::RENDERBUFFER_FORMAT_RGBA32F, "RenderTargetScene_RedGreenBlue", 0.35f); // higher threshold needed due to floating formats get sampled differently on different platforms
    case MultipleRenderTarget_TwoColorBuffersCleared:
        return runBasicTest<MultipleRenderTargetScene>(testFramework, MultipleRenderTargetScene::CLEAR_MRT, "MultiRenderTarget_ClearTwoColorBuffers");
    case MultipleRenderTarget_TwoColorBuffersWritten:
        return runBasicTest<MultipleRenderTargetScene>(testFramework, MultipleRenderTargetScene::TWO_COLOR_BUFFERS, "MultiRenderTarget_TwoColorBuffers");
    case MultipleRenderTarget_ShaderWritesTwoColorBuffers_RTHasOnlyOne:
        return runBasicTest<MultipleRenderTargetScene>(testFramework, MultipleRenderTargetScene::SHADER_WRITES_TWO_COLOR_BUFFERS_RT_HAS_ONE, "MultiRenderTarget_OneColorBufferWritten");
    case MultipleRenderTarget_ReuseSameColorBufferInTwoRTs:
        return runBasicTest<MultipleRenderTargetScene>(testFramework, MultipleRenderTargetScene::COLOR_WRITTEN_BY_TWO_DIFFERENT_RTS, "MultiRenderTarget_ColorBufferWrittenByTwoPasses");
    case MultipleRenderTarget_ReuseSameDepthBufferInTwoRTs:
        return runBasicTest<MultipleRenderTargetScene>(testFramework, MultipleRenderTargetScene::DEPTH_WRITTEN_AND_USED_BY_DIFFERENT_RT, "MultiRenderTarget_DepthBufferUsedByTwoPasses");
    case MultipleRenderTarget_ReadFromDepth:
        return runBasicTest<MultipleRenderTargetScene>(testFramework, MultipleRenderTargetScene::DEPTH_WRITTEN_AND_READ, "MultiRenderTarget_DepthRead");
    case RenderBuffer_OneColorBufferNoDepthOrStencil:
        return runBasicTest<RenderBufferScene>(testFramework, RenderBufferScene::ONE_COLOR_BUFFER_NO_DEPTH_OR_STENCIL, "RenderBuffer_OneColorBufferNoDepthOrStencil");
    case RenderBuffer_OneColorBufferWithWriteOnlyDepthBuffer:
        return runBasicTest<RenderBufferScene>(testFramework, RenderBufferScene::ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_BUFFER, "RenderBuffer_OneColorBufferWithWriteOnlyDepthBuffer");
    case RenderBuffer_OneColorBufferWithWriteOnlyDepthStencilBuffer:
        return runBasicTest<RenderBufferScene>(testFramework, RenderBufferScene::ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_STENCIL_BUFFER, "RenderBuffer_OneColorBufferWithWriteOnlyDepthStencilBuffer");
    case RenderBuffer_MsaaSampleCount2:
        return runBasicTest<MsaaRenderBufferScene>(testFramework, MsaaRenderBufferScene::SAMPLE_COUNT_2, "RenderBuffer_MsaaSampleCount2", 0.33f);
    case RenderBuffer_MsaaSampleCount4:
        return runBasicTest<MsaaRenderBufferScene>(testFramework, MsaaRenderBufferScene::SAMPLE_COUNT_4, "RenderBuffer_MsaaSampleCount4");

    default:
        assert(!"Invalid renderer test ID!");
        return false;
    }
}

template <typename INTEGRATION_SCENE>
bool RenderTargetRenderingTests::runBasicTest(RendererTestsFramework& testFramework, UInt32 sceneState, const String& expectedImageName, float maxAveragePercentErrorPerPixel)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.subscribeScene(sceneId);
    testFramework.mapScene(sceneId);
    testFramework.showScene(sceneId);
    return testFramework.renderAndCompareScreenshot(expectedImageName, 0u, maxAveragePercentErrorPerPixel);
}
