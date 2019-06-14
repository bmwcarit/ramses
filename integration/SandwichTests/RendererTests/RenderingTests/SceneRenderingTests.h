//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENERENDERINGTESTS_H
#define RAMSES_SCENERENDERINGTESTS_H

#include "IRendererTest.h"

class SceneRenderingTests : public IRendererTest
{
public:
    virtual void setUpTestCases(RendererTestsFramework& testFramework) final;
    virtual bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    template <typename INTEGRATION_SCENE>
    bool runBasicTest(
        RendererTestsFramework& testFramework,
        ramses_internal::UInt32 sceneState,
        const ramses_internal::String& expectedImageName,
        float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel,
        const ramses_internal::Vector3& cameraTranslation = ramses_internal::Vector3(0.0f),
        const ramses::SceneConfig& sceneConfig = ramses::SceneConfig());

    enum
    {
        RenderStateTest_Culling = 0,
        RenderStateTest_ColorMask,
        RenderStateTest_DepthFunc,
        RenderStateTest_DrawMode,
        RenderStateTest_StencilTest1,
        RenderStateTest_StencilTest2,
        RenderStateTest_StencilTest3,
        RenderStateTest_ScissorTest,

        AppearanceTest_RedTriangles,
        AppearanceTest_GreenTriangles,
        AppearanceTest_ChangeAppearance,
        AppearanceTest_TrianglesWithSharedColor,
        AppearanceTest_TrianglesWithUnsharedColor,

        BlendingTest_BlendingDisabled,
        BlendingTest_AlphaBlending,
        BlendingTest_SubtractiveBlending,
        BlendingTest_AdditiveBlending,

        CameraTest_Perspective,
        CameraTest_Orthographic,
        CameraTest_Viewport,

        SceneModificationTest_DeleteMeshNode,
        SceneModificationTest_NoVisibility,
        SceneModificationTest_PartialVisibility,
        SceneModificationTest_RotateAndScale,
        SceneModificationTest_CameraTransformation,
        SceneModificationTest_MeshRenderOrder,

        GeometryTest_SharedAppearance,
        GeometryTest_32bitIndices,
        GeometryTest_32bitIndicesWithOffset,
        GeometryTest_16bitIndices,
        GeometryTest_16bitIndicesWithOffset,
        GeometryTest_InstancingWithUniform,
        GeometryTest_InstancingWithVertexArray,
        GeometryTest_InstancingAndNotInstancing,
        GeometryTest_TriangleListWithoutIndexArray,
        GeometryTest_TriangleStripWithoutIndexArray,

        RenderPassTest_MeshesNotInPassNotRendered,
        RenderPassTest_DifferentCameras,
        RenderPassTest_MeshesInMultiplePasses,
        RenderPassTest_RenderOrder,

        BlitPassTest_BlitsColorBuffer,
        BlitPassTest_BlitsSubregion,
        BlitPassTest_BlitsDepthBuffer,
        BlitPassTest_BlitsDepthStencilBuffer,

        RenderGroupTest_RenderOrder,
        RenderGroupTest_RenderOrderWithNestedGroups,

        TextTest_SimpleText,
        TextTest_DeletedTextsAndNode,
        TextTest_DifferentLanguages,
        TextTest_ForceAutoHinting,
        TextTest_FontCascade,
        TextTest_FontCascadeWithVerticalOffset,
        TextTest_Shaping,

        AnimationTest_AnimatedScene,

        AntiAliasingTest_MSAA4,

        RenderPassClear_None,
        RenderPassClear_Color,
        RenderPassClear_Depth,
        RenderPassClear_Stencil,
        RenderPassClear_ColorStencil,
        RenderPassClear_ColorDepth,
        RenderPassClear_StencilDepth,
        RenderPassClear_ColorStencilDepth,

        ArrayInputTest_ArrayInputVec4,
        ArrayInputTest_ArrayInputInt32,
        ArrayInputTest_ArrayInputInt32DynamicIndex,

        DataBuffer_IndexDataBufferUInt16,
        DataBuffer_IndexDataBufferUInt32,
        DataBuffer_VertexDataBufferFloat,
        DataBuffer_VertexDataBufferVector2f,
        DataBuffer_VertexDataBufferVector3f,
        DataBuffer_VertexDataBufferVector4f,
        DataBuffer_IndexDataBufferGetsUpdated,
        DataBuffer_VertexDataBufferGetsUpdated,
        DataBuffer_SwitchFromClientArrayResourceToDataBuffer,
        DataBuffer_SwitchFromDataBufferToClientArrayResource,

        Display_SetClearColor,

        FrameProfiler_Show
    };
};

#endif
