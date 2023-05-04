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
    void setUpTestCases(RendererTestsFramework& testFramework) final;
    bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    template <typename INTEGRATION_SCENE>
    bool runBasicTest(
        RendererTestsFramework& testFramework,
        ramses_internal::UInt32 sceneState,
        const ramses_internal::String& expectedImageName,
        float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel,
        const glm::vec3& cameraTranslation = glm::vec3(0.0f),
        bool saveDiffOnError = true);

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
        BlendingTest_BlendingConstant,
        BlendingTest_BlendingDstColorAndAlpha,

        CameraTest_Perspective,
        CameraTest_Orthographic,
        CameraTest_Viewport,

        SceneModificationTest_DeleteMeshNode,
        SceneModificationTest_Invisible,
        SceneModificationTest_VisibilityOff,
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
        GeometryTest_VertexArraysWithOffset,

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

#if defined(RAMSES_TEXT_ENABLED)
        TextTest_SimpleText,
        TextTest_DeletedTextsAndNode,
        TextTest_DifferentLanguages,
        TextTest_ForceAutoHinting,
        TextTest_FontCascade,
        TextTest_FontCascadeWithVerticalOffset,
        TextTest_Shaping,
#endif

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
        DataBuffer_InterleavedVertexAttribute,
        DataBuffer_InterleavedVertexAttribute_GetsUpdated,
        DataBuffer_InterleavedVertexAttribute_TwoStrides,
        DataBuffer_InterleavedVertexAttribute_SingleAttrib,
        DataBuffer_InterleavedVertexAttribute_StartVertexOffset,

        ArrayResource_InterleavedVertexAttribute,
        ArrayResource_InterleavedVertexAttribute_TwoStrides,
        ArrayResource_InterleavedVertexAttribute_SingleAttrib,
        ArrayResource_InterleavedVertexAttribute_StartVertexOffset,

        Display_SetClearColor,

        GeometryShaderGlslV320_PointsInTriangleStripOut,
        GeometryShaderGlslV320_PointsInLineStripOut,
        GeometryShaderGlslV320_PointsInPointsOut,
        GeometryShaderGlslV320_TrianglesInTriangleStripOut,
        GeometryShaderGlslV320_TrianglesInPointsOut,
        GeometryShaderGlslV310Extension_PointsInTriangleStripOut,
        GeometryShaderGlslV310Extension_PointsInLineStripOut,
        GeometryShaderGlslV310Extension_PointsInPointsOut,
        GeometryShaderGlslV310Extension_TrianglesInTriangleStripOut,
        GeometryShaderGlslV310Extension_TrianglesInPointsOut,

        EulerRotationConventions
    };
};

#endif
