//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneRenderingTests.h"
#include "SceneAPI/RenderState.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "TestScenes/SingleAppearanceScene.h"
#include "TestScenes/HierarchicalRedTrianglesScene.h"
#include "TestScenes/MultipleGeometryScene.h"
#include "TestScenes/IndexArray32BitScene.h"
#include "TestScenes/RenderPassScene.h"
#include "TestScenes/BlitPassScene.h"
#include "TestScenes/RenderPassClearScene.h"
#include "TestScenes/TextScene.h"
#include "TestScenes/MultiLanguageTextScene.h"
#include "TestScenes/AntiAliasingScene.h"
#include "TestScenes/AnimatedTrianglesScene.h"
#include "TestScenes/ArrayInputScene.h"
#include "TestScenes/GeometryInstanceScene.h"
#include "TestScenes/RenderTargetScene.h"
#include "RamsesRendererImpl.h"
#include "TestScenes/DataBufferScene.h"

using namespace ramses_internal;

void SceneRenderingTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_Culling, *this, "RenderStateTest_Culling");
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_ColorMask, *this, "RenderStateTest_ColorMask");
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_DepthFunc, *this, "RenderStateTest_DepthFunc");
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_DrawMode, *this, "RenderStateTest_DrawMode");
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_StencilTest1, *this, "RenderStateTest_StencilTest1");
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_StencilTest2, *this, "RenderStateTest_StencilTest2");
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_StencilTest3, *this, "RenderStateTest_StencilTest3");
    testFramework.createTestCaseWithDefaultDisplay(RenderStateTest_ScissorTest, *this, "RenderStateTest_ScissorTest");

    testFramework.createTestCaseWithDefaultDisplay(AppearanceTest_RedTriangles, *this, "AppearanceTest_RedTriangles");
    testFramework.createTestCaseWithDefaultDisplay(AppearanceTest_GreenTriangles, *this, "AppearanceTest_GreenTriangles");
    testFramework.createTestCaseWithDefaultDisplay(AppearanceTest_ChangeAppearance, *this, "AppearanceTest_ChangeAppearance");
    testFramework.createTestCaseWithDefaultDisplay(AppearanceTest_TrianglesWithSharedColor, *this, "AppearanceTest_TrianglesWithSharedColor");
    testFramework.createTestCaseWithDefaultDisplay(AppearanceTest_TrianglesWithUnsharedColor, *this, "AppearanceTest_TrianglesWithUnsharedColor");

    testFramework.createTestCaseWithDefaultDisplay(BlendingTest_BlendingDisabled, *this, "BlendingTest_BlendingDisabled");
    testFramework.createTestCaseWithDefaultDisplay(BlendingTest_AlphaBlending, *this, "BlendingTest_AlphaBlending");
    testFramework.createTestCaseWithDefaultDisplay(BlendingTest_SubtractiveBlending, *this, "BlendingTest_SubtractiveBlending");
    testFramework.createTestCaseWithDefaultDisplay(BlendingTest_AdditiveBlending, *this, "BlendingTest_AdditiveBlending");

    testFramework.createTestCaseWithDefaultDisplay(CameraTest_Perspective, *this, "CameraTest_Perspective");
    testFramework.createTestCaseWithDefaultDisplay(CameraTest_Orthographic, *this, "CameraTest_Orthographic");
    testFramework.createTestCaseWithDefaultDisplay(CameraTest_Viewport, *this, "CameraTest_Viewport");

    testFramework.createTestCaseWithDefaultDisplay(SceneModificationTest_DeleteMeshNode, *this, "SceneModificationTest_DeleteMeshNode");
    testFramework.createTestCaseWithDefaultDisplay(SceneModificationTest_NoVisibility, *this, "SceneModificationTest_NoVisibility");
    testFramework.createTestCaseWithDefaultDisplay(SceneModificationTest_PartialVisibility, *this, "SceneModificationTest_PartialVisibility");
    testFramework.createTestCaseWithDefaultDisplay(SceneModificationTest_RotateAndScale, *this, "SceneModificationTest_RotateAndScale");
    testFramework.createTestCaseWithDefaultDisplay(SceneModificationTest_CameraTransformation, *this, "SceneModificationTest_CameraTransformation");
    testFramework.createTestCaseWithDefaultDisplay(SceneModificationTest_MeshRenderOrder, *this, "SceneModificationTest_MeshRenderOrder");

    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_SharedAppearance, *this, "GeometryTest_SharedAppearance");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_TriangleListWithoutIndexArray, *this, "GeometryTest_TriangleListWithoutIndexArray");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_TriangleStripWithoutIndexArray, *this, "GeometryTest_TriangleStripWithoutIndexArray");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_32bitIndices, *this, "GeometryTest_32bitIndices");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_32bitIndicesWithOffset, *this, "GeometryTest_32bitIndicesWithOffset");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_16bitIndices, *this, "GeometryTest_16bitIndices");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_16bitIndicesWithOffset, *this, "GeometryTest_16bitIndicesWithOffset");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_InstancingWithUniform, *this, "GeometryTest_InstancingWithUniform");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_InstancingWithVertexArray, *this, "GeometryTest_InstancingWithVertexArray");
    testFramework.createTestCaseWithDefaultDisplay(GeometryTest_InstancingAndNotInstancing, *this, "GeometryTest_InstancingAndNotInstancing");

    testFramework.createTestCaseWithDefaultDisplay(RenderPassTest_MeshesNotInPassNotRendered, *this, "RenderPassTest_MeshesNotInPassNotRendered");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassTest_DifferentCameras, *this, "RenderPassTest_DifferentCameras");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassTest_MeshesInMultiplePasses, *this, "RenderPassTest_MeshesInMultiplePasses");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassTest_RenderOrder, *this, "RenderPassTest_RenderOrder");

    testFramework.createTestCaseWithDefaultDisplay(BlitPassTest_BlitsColorBuffer, *this, "BlitPassTest_BlitsColorBuffer");
    testFramework.createTestCaseWithDefaultDisplay(BlitPassTest_BlitsSubregion, *this, "BlitPassTest_BlitsSubregion");
    testFramework.createTestCaseWithDefaultDisplay(BlitPassTest_BlitsDepthBuffer, *this, "BlitPassTest_BlitsDepthBuffer");
    testFramework.createTestCaseWithDefaultDisplay(BlitPassTest_BlitsDepthStencilBuffer, *this, "BlitPassTest_BlitsDepthStencilBuffer");

    testFramework.createTestCaseWithDefaultDisplay(RenderGroupTest_RenderOrder, *this, "RenderGroupTest_RenderOrder");
    testFramework.createTestCaseWithDefaultDisplay(RenderGroupTest_RenderOrderWithNestedGroups, *this, "RenderGroupTest_RenderOrderWithNestedGroups");

    testFramework.createTestCaseWithDefaultDisplay(TextTest_SimpleText, *this, "TextTest_SimpleText");
    testFramework.createTestCaseWithDefaultDisplay(TextTest_DeletedTextsAndNode, *this, "TextTest_DeletedTextsAndNode");
    testFramework.createTestCaseWithDefaultDisplay(TextTest_DifferentLanguages, *this, "TextTest_DifferentLanguages");
    testFramework.createTestCaseWithDefaultDisplay(TextTest_ForceAutoHinting, *this, "TextTest_ForceAutoHinting");
    testFramework.createTestCaseWithDefaultDisplay(TextTest_FontCascade, *this, "TextTest_FontCascade");
    testFramework.createTestCaseWithDefaultDisplay(TextTest_FontCascadeWithVerticalOffset, *this, "TextTest_FontCascadeWithVerticalOffset");
    testFramework.createTestCaseWithDefaultDisplay(TextTest_Shaping, *this, "TextTest_Shaping");

    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_None, *this, "RenderPassClear_None");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_Color, *this, "RenderPassClear_Color");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_Depth, *this, "RenderPassClear_Depth");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_Stencil, *this, "RenderPassClear_Stencil");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_ColorStencil, *this, "RenderPassClear_ColorStencil");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_ColorDepth, *this, "RenderPassClear_ColorDepth");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_StencilDepth, *this, "RenderPassClear_StencilDepth");
    testFramework.createTestCaseWithDefaultDisplay(RenderPassClear_ColorStencilDepth, *this, "RenderPassClear_ColorStencilDepth");

    testFramework.createTestCaseWithDefaultDisplay(AnimationTest_AnimatedScene, *this, "AnimationTest_AnimatedScene");

    testFramework.createTestCaseWithDefaultDisplay(ArrayInputTest_ArrayInputVec4, *this, "ArrayInputTest_ArrayInputVec4");
    testFramework.createTestCaseWithDefaultDisplay(ArrayInputTest_ArrayInputInt32, *this, "ArrayInputTest_ArrayInputInt32");
    testFramework.createTestCaseWithDefaultDisplay(ArrayInputTest_ArrayInputInt32DynamicIndex, *this, "ArrayInputTest_ArrayInputInt32DynamicIndex");

    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_IndexDataBufferUInt16, *this, "DataBuffer_IndexDataBufferUInt16");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_IndexDataBufferUInt32, *this, "DataBuffer_IndexDataBufferUInt32");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_VertexDataBufferFloat, *this, "DataBuffer_VertexDataBufferFloat");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_VertexDataBufferVector2f, *this, "DataBuffer_VertexDataBufferVector2f");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_VertexDataBufferVector3f, *this, "DataBuffer_VertexDataBufferVector3f");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_VertexDataBufferVector4f, *this, "DataBuffer_VertexDataBufferVector4f");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_IndexDataBufferGetsUpdated, *this, "DataBuffer_IndexDataBufferGetsUpdated");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_VertexDataBufferGetsUpdated, *this, "DataBuffer_VertexDataBufferGetsUpdated");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_SwitchFromClientArrayResourceToDataBuffer, *this, "DataBuffer_SwitchFromClientArrayResourceToDataBuffer");
    testFramework.createTestCaseWithDefaultDisplay(DataBuffer_SwitchFromDataBufferToClientArrayResource, *this, "DataBuffer_SwitchFromDataBufferToClientArrayResource");

    testFramework.createTestCaseWithDefaultDisplay(Display_SetClearColor, *this, "Display_SetClearColor").m_displayConfigs.front().setClearColor(0.5f, 0.25f, 0.75f, 1.f);

    testFramework.createTestCaseWithDefaultDisplay(FrameProfiler_Show, *this, "RendererTest_FrameProfiler");

    RenderingTestCase& testCase = testFramework.createTestCaseWithDefaultDisplay(AntiAliasingTest_MSAA4, *this, "AntiAliasingTest_MSAA4");
    testCase.m_displayConfigs.front().setMultiSampling(4u);
}

bool SceneRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    switch (testCase.m_id)
    {
    case RenderStateTest_Culling:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::FACE_CULLING, "MultipleTrianglesScene_FaceCulling");
    case RenderStateTest_ColorMask:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::COLOR_MASK, "MultipleTrianglesScene_ColorMask");
    case RenderStateTest_DepthFunc:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::DEPTH_FUNC, "MultipleTrianglesScene_DepthFunc");
    case RenderStateTest_DrawMode:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::DRAW_MODE, "MultipleTrianglesScene_DrawMode", 1.1f);
    case RenderStateTest_StencilTest1:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::STENCIL_TEST_1, "MultipleTrianglesScene_StencilTest_1");
    case RenderStateTest_StencilTest2:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::STENCIL_TEST_2, "MultipleTrianglesScene_StencilTest_2");
    case RenderStateTest_StencilTest3:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::STENCIL_TEST_3, "MultipleTrianglesScene_StencilTest_3");
    case RenderStateTest_ScissorTest:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::SCISSOR_TEST, "MultipleTrianglesScene_ScissorTest");
    case AppearanceTest_RedTriangles:
        return runBasicTest<SingleAppearanceScene>(testFramework, SingleAppearanceScene::RED_TRIANGLES, "SingleAppearanceScene_RedTriangles");
    case AppearanceTest_GreenTriangles:
        return runBasicTest<SingleAppearanceScene>(testFramework, SingleAppearanceScene::GREEN_TRIANGLES, "SingleAppearanceScene_GreenTriangles");
    case AppearanceTest_ChangeAppearance:
        return runBasicTest<SingleAppearanceScene>(testFramework, SingleAppearanceScene::CHANGE_APPEARANCE, "SingleAppearanceScene_ChangeAppearance");
    case AppearanceTest_TrianglesWithSharedColor:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::THREE_TRIANGLES_WITH_SHARED_COLOR, "MultipleTrianglesScene_ThreeTrianglesRed");
    case AppearanceTest_TrianglesWithUnsharedColor:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::THREE_TRIANGLES_WITH_UNSHARED_COLOR, "MultipleTrianglesScene_ThreeTriangles");

    case BlendingTest_BlendingDisabled:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::THREE_TRIANGLES, "MultipleTrianglesScene_ThreeTriangles");
    case BlendingTest_AlphaBlending:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::ALPHA_BLENDING, "MultipleTrianglesScene_AlphaBlending");
    case BlendingTest_SubtractiveBlending:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::SUBTRACTIVE_BLENDING, "MultipleTrianglesScene_SubtractiveBlending");
    case BlendingTest_AdditiveBlending:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::ADDITIVE_BLENDING, "MultipleTrianglesScene_AdditiveBlending");

    case CameraTest_Perspective:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::PERSPECTIVE_CAMERA, "MultipleTrianglesScene_PerspectiveCamera");
    case CameraTest_Orthographic:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::ORTHOGRAPHIC_CAMERA, "MultipleTrianglesScene_OrthographicCamera");
    case CameraTest_Viewport:
        return runBasicTest<RenderPassScene>(testFramework, RenderPassScene::PASSES_WITH_LEFT_AND_RIGHT_VIEWPORT, "RenderPassScene_LeftRightViewport");

    case SceneModificationTest_DeleteMeshNode:
        return runBasicTest<HierarchicalRedTrianglesScene>(testFramework, HierarchicalRedTrianglesScene::DELETE_MESHNODE, "HierarchicalRedTrianglesScene_DeleteMeshNode");
    case SceneModificationTest_NoVisibility:
        return runBasicTest<HierarchicalRedTrianglesScene>(testFramework, HierarchicalRedTrianglesScene::NO_VISIBILITY, "HierarchicalRedTrianglesScene_AllTrianglesInvisible");
    case SceneModificationTest_PartialVisibility:
        return runBasicTest<HierarchicalRedTrianglesScene>(testFramework, HierarchicalRedTrianglesScene::PARTIAL_VISIBILITY, "HierarchicalRedTrianglesScene_SomeTrianglesInvisible");
    case SceneModificationTest_RotateAndScale:
        return runBasicTest<HierarchicalRedTrianglesScene>(testFramework, HierarchicalRedTrianglesScene::ROTATE_AND_SCALE, "HierarchicalRedTrianglesScene_RotateAndScale");
    case SceneModificationTest_CameraTransformation:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::CAMERA_TRANSFORMATION, "MultipleTrianglesScene_CameraTransformation");
    case SceneModificationTest_MeshRenderOrder:
        return runBasicTest<MultipleTrianglesScene>(testFramework, MultipleTrianglesScene::TRIANGLES_REORDERED, "MultipleTrianglesScene_RenderingOrderChanged");

    case GeometryTest_SharedAppearance:
        return runBasicTest<MultipleGeometryScene>(testFramework, MultipleGeometryScene::MULTI_TRIANGLE_LIST_GEOMETRY_WITH_INDEX_ARRAY, "MultipleGeometryScene_MultipleGeometry");
    case GeometryTest_TriangleListWithoutIndexArray:
        return runBasicTest<MultipleGeometryScene>(testFramework, MultipleGeometryScene::MULTI_TRIANGLE_LIST_GEOMETRY_WITHOUT_INDEX_ARRAY, "MultipleGeometryScene_MultipleGeometry");
    case GeometryTest_TriangleStripWithoutIndexArray:
        return runBasicTest<MultipleGeometryScene>(testFramework, MultipleGeometryScene::MULTI_TRIANGLE_STRIP_GEOMETRY_WITHOUT_INDEX_ARRAY, "MultipleGeometryScene_MultipleGeometry");
    case GeometryTest_32bitIndices:
        return runBasicTest<IndexArray32BitScene>(testFramework, IndexArray32BitScene::NO_OFFSET_32BIT_INDICES, "IndexArray32BitScene_NoOffset32BitIndices");
    case GeometryTest_32bitIndicesWithOffset:
        return runBasicTest<IndexArray32BitScene>(testFramework, IndexArray32BitScene::OFFSET_32BIT_INDICES, "IndexArray32BitScene_Offset32BitIndices");
    case GeometryTest_16bitIndices:
        return runBasicTest<IndexArray32BitScene>(testFramework, IndexArray32BitScene::NO_OFFSET_16BIT_INDICES, "IndexArray32BitScene_NoOffset16BitIndices");
    case GeometryTest_16bitIndicesWithOffset:
        return runBasicTest<IndexArray32BitScene>(testFramework, IndexArray32BitScene::OFFSET_16BIT_INDICES, "IndexArray32BitScene_Offset16BitIndices");
    case GeometryTest_InstancingWithUniform:
        return runBasicTest<GeometryInstanceScene>(testFramework, GeometryInstanceScene::GEOMETRY_INSTANCE_UNIFORM, "GeometryInstanceScene_Instancing");
    case GeometryTest_InstancingWithVertexArray:
        return runBasicTest<GeometryInstanceScene>(testFramework, GeometryInstanceScene::GEOMETRY_INSTANCE_VERTEX, "GeometryInstanceScene_Instancing");
    case GeometryTest_InstancingAndNotInstancing:
        return runBasicTest<GeometryInstanceScene>(testFramework, GeometryInstanceScene::GEOMETRY_INSTANCE_AND_NOT_INSTANCE, "GeometryInstanceScene_InstancingAndNotInstancing");

    case RenderPassTest_MeshesNotInPassNotRendered:
        return runBasicTest<RenderPassScene>(testFramework, RenderPassScene::MESHES_NOT_IN_PASS, "RenderPassScene_MeshNotInPass");
    case RenderPassTest_DifferentCameras:
        return runBasicTest<RenderPassScene>(testFramework, RenderPassScene::ONE_MESH_PER_PASS, "RenderPassScene_OneMeshPerPass");
    case RenderPassTest_MeshesInMultiplePasses:
        return runBasicTest<RenderPassScene>(testFramework, RenderPassScene::MESH_IN_MULTIPLE_PASSES, "RenderPassScene_MeshInMultiplePasses");
    case RenderPassTest_RenderOrder:
        return runBasicTest<RenderPassScene>(testFramework, RenderPassScene::PASSES_WITH_DIFFERENT_RENDER_ORDER, "RenderPassScene_PassesWithDifferentRenderOrder");

    case BlitPassTest_BlitsColorBuffer:
        return runBasicTest<BlitPassScene>(testFramework, BlitPassScene::BLITS_COLOR_BUFFER, "BlitPassTest_BlitsColorBuffer");
    case BlitPassTest_BlitsSubregion:
        return runBasicTest<BlitPassScene>(testFramework, BlitPassScene::BLITS_SUBREGION, "BlitPassTest_BlitsSubregion", 0.25f);
    case BlitPassTest_BlitsDepthBuffer:
        return runBasicTest<BlitPassScene>(testFramework, BlitPassScene::BLITS_DEPTH_BUFFER, "BlitPassTest_BlitsDepthBuffer", 0.25f);
    case BlitPassTest_BlitsDepthStencilBuffer:
        return runBasicTest<BlitPassScene>(testFramework, BlitPassScene::BLITS_DEPTH_STENCIL_BUFFER, "BlitPassTest_BlitsDepthStencilBuffer");

    case RenderGroupTest_RenderOrder:
        return runBasicTest<RenderPassScene>(testFramework, RenderPassScene::GROUPS_WITH_DIFFERENT_RENDER_ORDER, "RenderPassScene_PassesWithDifferentRenderOrder");
    case RenderGroupTest_RenderOrderWithNestedGroups:
        return runBasicTest<RenderPassScene>(testFramework, RenderPassScene::NESTED_GROUPS, "RenderPassScene_PassesWithDifferentRenderOrder");

    case TextTest_SimpleText:
        return runBasicTest<TextScene>(testFramework, TextScene::EState_INITIAL, "TextScene_SimpleText");
    case TextTest_DeletedTextsAndNode:
        return runBasicTest<TextScene>(testFramework, TextScene::EState_DELETED_TEXTS, "TextScene_DeletedTextsAndNode");
    case TextTest_ForceAutoHinting:
        return runBasicTest<TextScene>(testFramework, TextScene::EState_FORCE_AUTO_HINTING, "TextScene_ForceAutoHinting");
    case TextTest_FontCascade:
        return runBasicTest<TextScene>(testFramework, TextScene::EState_FONT_CASCADE, "TextScene_FontCascade");
    case TextTest_FontCascadeWithVerticalOffset:
        return runBasicTest<TextScene>(testFramework, TextScene::EState_FONT_CASCADE_WITH_VERTICAL_OFFSET, "TextScene_VerticalOffset");
    case TextTest_Shaping:
        return runBasicTest<TextScene>(testFramework, TextScene::EState_SHAPING, "TextScene_Shaping");
    case TextTest_DifferentLanguages:
        return runBasicTest<MultiLanguageTextScene>(testFramework, MultiLanguageTextScene::EState_INITIAL, "MultiLanguageScene_MultiLanguageText");


    case AnimationTest_AnimatedScene:
        return runBasicTest<AnimatedTrianglesScene>(testFramework, AnimatedTrianglesScene::ANIMATION_POINT4, "AnimatedTriangleScene_AnimatedScene");

    case AntiAliasingTest_MSAA4:
        return runBasicTest<AntiAliasingScene>(testFramework, AntiAliasingScene::MSAA_4_STATE, "AntiAliasingScene_MSAAx4", 2.5f);

    case RenderPassClear_None:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_None, "RenderPassClear_None");
    case RenderPassClear_Color:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_Color, "RenderPassClear_Color");
    case RenderPassClear_Depth:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_Depth, "RenderPassClear_Depth");
    case RenderPassClear_Stencil:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_Stencil, "RenderPassClear_Stencil");
    case RenderPassClear_ColorStencil:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_Color | EClearFlags_Stencil, "RenderPassClear_ColorStencil");
    case RenderPassClear_ColorDepth:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_Color | EClearFlags_Depth, "RenderPassClear_ColorDepth");
    case RenderPassClear_StencilDepth:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_Stencil | EClearFlags_Depth, "RenderPassClear_StencilDepth");
    case RenderPassClear_ColorStencilDepth:
        return runBasicTest<RenderPassClearScene>(testFramework, EClearFlags_All, "RenderPassClear_ColorStencilDepth");

    case ArrayInputTest_ArrayInputVec4:
        return runBasicTest<ArrayInputScene>(testFramework, ArrayInputScene::ARRAY_INPUT_VEC4, "ArrayInputScene_ArrayInputVec4");
    case ArrayInputTest_ArrayInputInt32:
        return runBasicTest<ArrayInputScene>(testFramework, ArrayInputScene::ARRAY_INPUT_INT32, "ArrayInputScene_ArrayInputInt32");
    case ArrayInputTest_ArrayInputInt32DynamicIndex:
        return runBasicTest<ArrayInputScene>(testFramework, ArrayInputScene::ARRAY_INPUT_INT32_DYNAMIC_INDEX, "ArrayInputScene_ArrayInputInt32DynamicIndex");

    case DataBuffer_IndexDataBufferUInt16:
        return runBasicTest<DataBufferScene>(testFramework, DataBufferScene::INDEX_DATA_BUFFER_UINT16, "DataBufferScene_RedTriangle", 0.0f, Vector3(2, -1, 18));
    case DataBuffer_IndexDataBufferUInt32:
        return runBasicTest<DataBufferScene>(testFramework, DataBufferScene::INDEX_DATA_BUFFER_UINT32, "DataBufferScene_RedTriangle", 0.0f, Vector3(2, -1, 18));
    case DataBuffer_VertexDataBufferFloat:
        return runBasicTest<DataBufferScene>(testFramework, DataBufferScene::VERTEX_DATA_BUFFER_FLOAT, "DataBufferScene_RedTriangle", 0.0f, Vector3(2, -1, 18));
    case DataBuffer_VertexDataBufferVector2f:
        return runBasicTest<DataBufferScene>(testFramework, DataBufferScene::VERTEX_DATA_BUFFER_VECTOR2F, "DataBufferScene_RedTriangle", 0.0f, Vector3(2, -1, 18));
    case DataBuffer_VertexDataBufferVector3f:
        return runBasicTest<DataBufferScene>(testFramework, DataBufferScene::VERTEX_DATA_BUFFER_VECTOR3F, "DataBufferScene_RedTriangle", 0.0f, Vector3(2, -1, 18));
    case DataBuffer_VertexDataBufferVector4f:
        return runBasicTest<DataBufferScene>(testFramework, DataBufferScene::VERTEX_DATA_BUFFER_VECTOR4F, "DataBufferScene_RedTriangle", 0.0f, Vector3(2, -1, 18));
    case DataBuffer_IndexDataBufferGetsUpdated:
    {
        const ramses::sceneId_t sceneId = testFramework.createAndShowScene<DataBufferScene>(DataBufferScene::INDEX_DATA_BUFFER_UINT32, Vector3(2, -1, 18));
        testFramework.getScenesRegistry().setSceneState<DataBufferScene>(sceneId, DataBufferScene::UPDATE_INDEX_DATA_BUFFER);
        return testFramework.renderAndCompareScreenshot("DataBufferScene_RedTriangleInverted", 0u);
    }
    case DataBuffer_VertexDataBufferGetsUpdated:
    {
        const ramses::sceneId_t sceneId = testFramework.createAndShowScene<DataBufferScene>(DataBufferScene::VERTEX_DATA_BUFFER_VECTOR4F, Vector3(2, -1, 18));
        testFramework.getScenesRegistry().setSceneState<DataBufferScene>(sceneId, DataBufferScene::UPDATE_VERTEX_DATA_BUFFER);
        return testFramework.renderAndCompareScreenshot("DataBufferScene_RedTriangleInverted", 0u);
    }
    case DataBuffer_SwitchFromClientArrayResourceToDataBuffer:
    {
        const ramses::sceneId_t sceneId = testFramework.createAndShowScene<DataBufferScene>(DataBufferScene::VERTEX_ARRAY_BUFFER_VECTOR4F, Vector3(2, -1, 18));
        testFramework.getScenesRegistry().setSceneState<DataBufferScene>(sceneId, DataBufferScene::VERTEX_DATA_BUFFER_VECTOR4F);
        return testFramework.renderAndCompareScreenshot("DataBufferScene_RedTriangle", 0u);
    }
    case DataBuffer_SwitchFromDataBufferToClientArrayResource:
    {
        const ramses::sceneId_t sceneId = testFramework.createAndShowScene<DataBufferScene>(DataBufferScene::VERTEX_DATA_BUFFER_VECTOR4F, Vector3(2, -1, 18));
        testFramework.getScenesRegistry().setSceneState<DataBufferScene>(sceneId, DataBufferScene::VERTEX_ARRAY_BUFFER_VECTOR4F);
        return testFramework.renderAndCompareScreenshot("DataBufferScene_EquilateralTriangle", 0u);
    }
    case Display_SetClearColor:
    {
        return testFramework.renderAndCompareScreenshot("Display_SetClearColor", 0u, 0.4f);
    }
    case FrameProfiler_Show:
    {
        testFramework.getTestRenderer().toggleRendererFrameProfiler();
        bool profilerIsVisible = !runBasicTest<SingleAppearanceScene>(testFramework, SingleAppearanceScene::RED_TRIANGLES, "SingleAppearanceScene_RedTriangles");
        testFramework.getTestRenderer().toggleRendererFrameProfiler();
        bool profilerIsInvisible = runBasicTest<SingleAppearanceScene>(testFramework, SingleAppearanceScene::RED_TRIANGLES, "SingleAppearanceScene_RedTriangles");
        return profilerIsInvisible && profilerIsVisible;
    }

    default:
        assert(!"Invalid renderer test ID!");
        return false;
    }
}

template <typename INTEGRATION_SCENE>
bool SceneRenderingTests::runBasicTest(
    RendererTestsFramework& testFramework,
    UInt32 sceneState,
    const String& expectedImageName,
    float maxAveragePercentErrorPerPixel,
    const Vector3& cameraTranslation,
    const ramses::SceneConfig& sceneConfig)
{
    testFramework.createAndShowScene<INTEGRATION_SCENE>(sceneState, cameraTranslation, 0, sceneConfig);
    return testFramework.renderAndCompareScreenshot(expectedImageName, 0u, maxAveragePercentErrorPerPixel);
}
