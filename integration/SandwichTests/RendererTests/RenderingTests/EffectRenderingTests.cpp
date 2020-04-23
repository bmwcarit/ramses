//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectRenderingTests.h"
#include "TestScenes/ShaderTestScene.h"
#include "TestScenes/SingleAppearanceScene.h"
#include "TestScenes/CustomShaderTestScene.h"

using namespace ramses_internal;

void EffectRenderingTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_Shaders, *this, "EffectTest_Shaders");
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_Discard_Shaders, *this, "EffectTest_Discard_Shaders");
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_UniformWithSameNameInBothStages, *this, "EffectTest_UniformWithSameNameInBothStages");
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_ExplicitAttributeLocation, *this, "EffectTest_ExplicitAttributeLocation");
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_ExplicitAttributeLocationSwapped, *this, "EffectTest_ExplicitAttributeLocationSwapped");
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_StructUniform, *this, "EffectTest_StructUniform");
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_TextureSize, *this, "EffectTest_TextureSize");
    testFramework.createTestCaseWithDefaultDisplay(EffectTest_OptimizedInput, *this, "EffectTest_OptimizedInput");
}

bool EffectRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    switch (testCase.m_id)
    {
    case EffectTest_Shaders:
        return runBasicTest<SingleAppearanceScene>(testFramework, SingleAppearanceScene::RED_TRIANGLES, "SingleAppearanceScene_RedTriangles");
    case EffectTest_Discard_Shaders:
        return runBasicTest<ShaderTestScene>(testFramework, ShaderTestScene::DISCARD, "ShaderTestScene_Discard");
    case EffectTest_OptimizedInput:
        return runBasicTest<ShaderTestScene>(testFramework, ShaderTestScene::OPTIMIZED_INPUT, "ShaderTestScene_OptimizedInput");
    case EffectTest_UniformWithSameNameInBothStages:
        return runBasicTest<ShaderTestScene>(testFramework, ShaderTestScene::UNIFORM_WITH_SAME_NAME_IN_BOTH_STAGES, "ShaderTestScene_MultiStageUniform");
    case EffectTest_ExplicitAttributeLocation:
        return runBasicTest<CustomShaderTestScene>(testFramework, CustomShaderTestScene::EXPLICIT_ATTRIBUTE_LOCATION, "ShaderTestScene_TexturedQuad");
    case EffectTest_ExplicitAttributeLocationSwapped:
        return runBasicTest<CustomShaderTestScene>(testFramework, CustomShaderTestScene::EXPLICIT_ATTRIBUTE_LOCATION_SWAPPED, "ShaderTestScene_TexturedQuad");
    case EffectTest_StructUniform:
        return runBasicTest<ShaderTestScene>(testFramework, ShaderTestScene::STRUCT_UNIFORM, "ShaderTestScene_StructUniform");
    case EffectTest_TextureSize:
        return runBasicTest<ShaderTestScene>(testFramework, ShaderTestScene::TEXTURE_SIZE, "ShaderTestScene_TextureSize");
    default:
        assert(!"Invalid renderer test ID!");
        return false;
    }
}

template <typename INTEGRATION_SCENE>
bool EffectRenderingTests::runBasicTest(RendererTestsFramework& testFramework, UInt32 sceneState, const String& expectedImageName)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId);
    return testFramework.renderAndCompareScreenshot(expectedImageName, 0u);
}
