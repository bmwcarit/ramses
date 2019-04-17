//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextureRenderingTests.h"

#include "TestScenes/Texture2DAnisotropicTextureFilteringScene.h"
#include "TestScenes/Texture2DFormatScene.h"
#include "TestScenes/Texture2DSamplingScene.h"
#include "TestScenes/Texture2DGenerateMipMapScene.h"
#include "TestScenes/Texture2DCompressedMipMapScene.h"
#include "TestScenes/TextureAddressScene.h"
#include "TestScenes/Texture3DScene.h"
#include "TestScenes/CubeTextureScene.h"
#include "TestScenes/TextureCubeAnisotropicTextureFilteringScene.h"
#include "TestScenes/TextureBufferScene.h"
#include "TestScenes/TextureSamplerScene.h"

using namespace ramses_internal;

void TextureRenderingTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_R8, *this, "TextureTest_Texture2D_Format_R8");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RG8, *this, "TextureTest_Texture2D_Format_RG8");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGB8, *this, "TextureTest_Texture2D_Format_RGB8");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGB565, *this, "TextureTest_Texture2D_Format_RGB565");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGBA8, *this, "TextureTest_Texture2D_Format_RGBA8");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGBA4, *this, "TextureTest_Texture2D_Format_RGBA4");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGBA5551, *this, "TextureTest_Texture2D_Format_RGBA5551");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_BGR8, *this, "TextureTest_Texture2D_Format_BGR8");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_BGRA8, *this, "TextureTest_Texture2D_Format_BGRA8");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_R16F, *this, "TextureTest_Texture2D_Format_R16F");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_R32F, *this, "TextureTest_Texture2D_Format_R32F");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RG16F, *this, "TextureTest_Texture2D_Format_RG16F");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RG32F, *this, "TextureTest_Texture2D_Format_RG32F");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGB16F, *this, "TextureTest_Texture2D_Format_RGB16F");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGB32F, *this, "TextureTest_Texture2D_Format_RGB32F");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGBA16F, *this, "TextureTest_Texture2D_Format_RGBA16F");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_RGBA32F, *this, "TextureTest_Texture2D_Format_RGBA32F");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_SRGB8, *this, "TextureTest_Texture2D_Format_SRGB8");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_SRGB8_ALPHA8, *this, "TextureTest_Texture2D_Format_SRGB8_ALPHA8");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_ETC2RGB, *this, "TextureTest_Texture2D_Format_ETC2RGB");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_ETC2RGBA, *this, "TextureTest_Texture2D_Format_ETC2RGBA");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_ASTC_RGBA_4x4, *this, "TextureTest_Texture2D_Format_ASTC_RGBA_4x4");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4, *this, "TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Sampling_Nearest, *this, "TextureTest_Texture2D_Sampling_Nearest");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Sampling_NearestWithMipMaps, *this, "TextureTest_Texture2D_Sampling_NearestWithMipMaps");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Sampling_Bilinear, *this, "TextureTest_Texture2D_Sampling_Bilinear");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Sampling_BilinearWithMipMaps, *this, "TextureTest_Texture2D_Sampling_BilinearWithMipMaps");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Sampling_Trilinear, *this, "TextureTest_Texture2D_Sampling_Trilinear");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Sampling_MinLinearMagNearest, *this, "TextureTest_Texture2D_Sampling_MinLinearMagNearest");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_Sampling_MinNearestMagLinear, *this, "TextureTest_Texture2D_Sampling_MinNearestMagLinear");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_AddressMode, *this, "TextureTest_Texture2D_AddressMode");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_AnisotropicFilter, *this, "TextureTest_Texture2D_AnisotropicFilter");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_GenerateMipMapSingle, *this, "TextureTest_Texture2D_GenerateSingleMipMap");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_GenerateMipMapMultiple, *this, "TextureTest_Texture2D_GenerateMultipleMipMaps");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture2D_CompressedMipMap, *this, "TextureTest_Texture2D_CompressedMipMap");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_Texture3D_RGBA8, *this, "TextureTest_Texture3D_RGBA8");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_CubeMap_RGBA8, *this, "TextureTest_CubeMap_RGBA8");
    testFramework.createTestCaseWithDefaultDisplay(TextureTest_CubeMap_Float, *this, "TextureTest_CubeMap_Float");

    testFramework.createTestCaseWithDefaultDisplay(TextureTest_TextureCube_AnisotropicFilter, *this, "TextureTest_TextureCube_AnisotropicFilter");

    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_RGBA8_OneMip, *this, "TextureBufferTest_RGBA8_OneMip");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_RGBA8_ThreeMips, *this, "TextureBufferTest_RGBA8_ThreeMips");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_PartialUpdate, *this, "TextureBufferTest_PartialUpdate");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_PartialUpdateMipMap, *this, "TextureBufferTest_PartialUpdateMipMap");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_PartialUpdateMipMap_RG8, *this, "TextureBufferTest_PartialUpdateMipMap_RG8");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_SwitchSceneTextureToClientTexture, *this, "TextureBufferTest_SwitchSceneTextureToClientTexture");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_SwitchClientTextureToSceneTexture, *this, "TextureBufferTest_SwitchClientTextureToSceneTexture");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_SwitchClientTextureToSceneTextureAndBack, *this, "TextureBufferTest_SwitchClientTextureToSceneTextureAndBack");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_SwitchClientTextureToSceneTextureAndUpdate, *this, "TextureBufferTest_SwitchClientTextureToSceneTextureAndUpdate");
    testFramework.createTestCaseWithDefaultDisplay(TextureBufferTest_ReMapScene, *this, "TextureBufferTest_ReMapScene");

    testFramework.createTestCaseWithDefaultDisplay(SamplerTest_ChangeData_ClientTexture, *this, "SamplerTest_ChangeData_ClientTexture");
    testFramework.createTestCaseWithDefaultDisplay(SamplerTest_ChangeData_TextureBufferToClientTexture, *this, "SamplerTest_ChangeData_TextureBufferToClientTexture");
    testFramework.createTestCaseWithDefaultDisplay(SamplerTest_ChangeData_ClientTextureToTextureBuffer, *this, "SamplerTest_ChangeData_ClientTextureToTextureBuffer");
    testFramework.createTestCaseWithDefaultDisplay(SamplerTest_ChangeData_ClientTextureToRenderBuffer, *this, "SamplerTest_ChangeData_ClientTextureToRenderBuffer");
    testFramework.createTestCaseWithDefaultDisplay(SamplerTest_ChangeData_ClientTextureToStreamTexture, *this, "SamplerTest_ChangeData_ClientTextureToStreamTexture");
}

bool TextureRenderingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    switch (testCase.m_id)
    {
    case TextureTest_Texture2D_Format_R8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_R8, "Texture2DFormatScene_R8");
    case TextureTest_Texture2D_Format_RG8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RG8, "Texture2DFormatScene_RG8");
    case TextureTest_Texture2D_Format_RGB8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGB8, "Texture2DFormatScene_RGB8");
    case TextureTest_Texture2D_Format_RGB565:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGB565, "Texture2DFormatScene_RGB565");
    case TextureTest_Texture2D_Format_RGBA8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGBA8, "Texture2DFormatScene_RGBA8");
    case TextureTest_Texture2D_Format_RGBA4:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGBA4, "Texture2DFormatScene_RGBA4");
    case TextureTest_Texture2D_Format_RGBA5551:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGBA5551, "Texture2DFormatScene_RGBA5551");

    case TextureTest_Texture2D_Format_R16F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_R16F, "Texture2DFormatScene_FloatRed");
    case TextureTest_Texture2D_Format_R32F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_R32F, "Texture2DFormatScene_FloatRed");
    case TextureTest_Texture2D_Format_RG16F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RG16F, "Texture2DFormatScene_FloatRG");
    case TextureTest_Texture2D_Format_RG32F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RG32F, "Texture2DFormatScene_FloatRG");
    case TextureTest_Texture2D_Format_RGB16F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGB16F, "Texture2DFormatScene_FloatRGB");
    case TextureTest_Texture2D_Format_RGB32F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGB32F, "Texture2DFormatScene_FloatRGB");
    case TextureTest_Texture2D_Format_RGBA16F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGBA16F, "Texture2DFormatScene_FloatRGB");
    case TextureTest_Texture2D_Format_RGBA32F:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_RGBA32F, "Texture2DFormatScene_FloatRGB");
    case TextureTest_Texture2D_Format_SRGB8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_SRGB8, "Texture2DFormatScene_SRGB8");
    case TextureTest_Texture2D_Format_SRGB8_ALPHA8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_SRGB8_ALPHA8, "Texture2DFormatScene_SRGB8_ALPHA8");
    case TextureTest_Texture2D_Format_ASTC_RGBA_4x4:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_ASTC_RGBA_4x4, "Texture2DFormatScene_ASTC_RGBA_4x4");
    case TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_ASTC_SRGB_ALPHA_4x4, "Texture2DFormatScene_ASTC_SRGB_ALPHA_4x4");

    case TextureTest_Texture2D_Format_BGR8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_BGR8, "Texture2DFormatScene_BGR8");
    case TextureTest_Texture2D_Format_BGRA8:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_BGRA8, "Texture2DFormatScene_BGRA8");
    case TextureTest_Texture2D_Format_ETC2RGB:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_ETC2RGB, "Texture2DFormatScene_ETC2RGB");
    case TextureTest_Texture2D_Format_ETC2RGBA:
        return runBasicTest<Texture2DFormatScene>(testFramework, Texture2DFormatScene::EState_ETC2RGBA, "Texture2DFormatScene_ETC2RGBA");
    case TextureTest_Texture2D_Sampling_Nearest:
        return runBasicTest<Texture2DSamplingScene>(testFramework, Texture2DSamplingScene::EState_Nearest, "Texture2DSamplingScene_Nearest");
    case TextureTest_Texture2D_Sampling_NearestWithMipMaps:
        return runBasicTest<Texture2DSamplingScene>(testFramework, Texture2DSamplingScene::EState_NearestWithMipMaps, "Texture2DSamplingScene_NearestWithMipMaps");
    case TextureTest_Texture2D_Sampling_Bilinear:
        return runBasicTest<Texture2DSamplingScene>(testFramework, Texture2DSamplingScene::EState_Bilinear, "Texture2DSamplingScene_Bilinear", 1.0f);
    case TextureTest_Texture2D_Sampling_BilinearWithMipMaps:
        return runBasicTest<Texture2DSamplingScene>(testFramework, Texture2DSamplingScene::EState_BilinearWithMipMaps, "Texture2DSamplingScene_BilinearWithMipMaps");
    case TextureTest_Texture2D_Sampling_Trilinear:
        return runBasicTest<Texture2DSamplingScene>(testFramework, Texture2DSamplingScene::EState_Trilinear, "Texture2DSamplingScene_Trilinear", 6.0f);
    case TextureTest_Texture2D_Sampling_MinLinearMagNearest:
        return runBasicTest<Texture2DSamplingScene>(testFramework, Texture2DSamplingScene::EState_MinLinearMagNearest, "Texture2DSamplingScene_MinLinearMagNearest", 1.0f);
    case TextureTest_Texture2D_Sampling_MinNearestMagLinear:
        return runBasicTest<Texture2DSamplingScene>(testFramework, Texture2DSamplingScene::EState_MinNearestMagLinear, "Texture2DSamplingScene_MinNearestMagLinear");

    case TextureTest_Texture2D_AddressMode:
        return runBasicTest<TextureAddressScene>(testFramework, TextureAddressScene::ADDRESS_MODE_STATE, "TextureAddressScene_AllAddressModes", 0.4f);
    case TextureTest_Texture2D_AnisotropicFilter:
        return runBasicTest<Texture2DAnisotropicTextureFilteringScene>(testFramework, Texture2DAnisotropicTextureFilteringScene::EState_Anisotropic, "Texture2DAnisotropicTextureFilteringScene_Anisotropic");
    case TextureTest_Texture2D_GenerateMipMapSingle:
        return runBasicTest<Texture2DGenerateMipMapScene>(testFramework, Texture2DGenerateMipMapScene::EState_GenerateMipMapSingle, "Texture2DGenerateMipMapScene_GenerateSingleMipMap", 1.0f);
    case TextureTest_Texture2D_GenerateMipMapMultiple:
        return runBasicTest<Texture2DGenerateMipMapScene>(testFramework, Texture2DGenerateMipMapScene::EState_GenerateMipMapMultiple, "Texture2DGenerateMipMapScene_GenerateMultipleMipMaps", 1.0f);
    case TextureTest_Texture2D_CompressedMipMap:
        return runBasicTest<Texture2DCompressedMipMapScene>(testFramework, Texture2DCompressedMipMapScene::EState_CompressedMipMap, "Texture2DCompressedMipMapScene_CompressedMipMap");
    case TextureTest_Texture3D_RGBA8:
        return runBasicTest<Texture3DScene>(testFramework, Texture3DScene::SLICES_4, "Texture3DScene_4Slices");
    case TextureTest_CubeMap_RGBA8:
        return runBasicTest<CubeTextureScene>(testFramework, CubeTextureScene::EState_RGBA8, "CubeTextureScene_CubeMap");
    case TextureTest_CubeMap_Float:
        return runBasicTest<CubeTextureScene>(testFramework, CubeTextureScene::EState_Float, "CubeTextureScene_CubeMapFloat");
    case TextureTest_TextureCube_AnisotropicFilter:
        return runBasicTest<TextureCubeAnisotropicTextureFilteringScene>(testFramework, TextureCubeAnisotropicTextureFilteringScene::EState_Anisotropic, "TextureCubeAnisotropicTextureFilteringScene_Anisotropic");

    case TextureBufferTest_RGBA8_OneMip:
        return runBasicTest<TextureBufferScene>(testFramework, TextureBufferScene::EState_RGBA8_OneMip, "TextureBuffer_RGBA8_OneMip");
    case TextureBufferTest_RGBA8_ThreeMips:
        return runBasicTest<TextureBufferScene>(testFramework, TextureBufferScene::EState_RGBA8_ThreeMips, "TextureBuffer_RGBA8_ThreeMips");
    case TextureBufferTest_PartialUpdate:
        return runBasicTest<TextureBufferScene>(testFramework, TextureBufferScene::EState_PartialUpdate, "TextureBuffer_PartialUpdate");
    case TextureBufferTest_PartialUpdateMipMap:
        return runBasicTest<TextureBufferScene>(testFramework, TextureBufferScene::EState_PartialUpdateMipMap, "TextureBuffer_PartialUpdateMipMap");
    case TextureBufferTest_PartialUpdateMipMap_RG8:
        return runBasicTest<TextureBufferScene>(testFramework, TextureBufferScene::EState_PartialUpdateMipMap_RG8, "TextureBuffer_PartialUpdateMipMap_RG8");
    case TextureBufferTest_SwitchSceneTextureToClientTexture:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureBufferScene>(testFramework, TextureBufferScene::EState_RGBA8_OneMip);
        const Bool sceneTextureCorrect = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMip", 0u);
        testFramework.getScenesRegistry().setSceneState<TextureBufferScene>(sceneId, TextureBufferScene::EState_ClientTextureResource_RGBA8);
        const Bool switchToClientTexture = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMipGreen", 0u);
        return sceneTextureCorrect && switchToClientTexture;
    }
    case TextureBufferTest_SwitchClientTextureToSceneTexture:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureBufferScene>(testFramework, TextureBufferScene::EState_ClientTextureResource_RGBA8);
        const Bool clientTextureCorrect = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMipGreen", 0u);
        testFramework.getScenesRegistry().setSceneState<TextureBufferScene>(sceneId, TextureBufferScene::EState_RGBA8_OneMip);
        const Bool switchToSceneTextureCorrect = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMip", 0u);
        return clientTextureCorrect && switchToSceneTextureCorrect;
    }
    case TextureBufferTest_SwitchClientTextureToSceneTextureAndBack:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureBufferScene>(testFramework, TextureBufferScene::EState_ClientTextureResource_RGBA8);
        const Bool clientTextureCorrect = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMipGreen", 0u);
        testFramework.getScenesRegistry().setSceneState<TextureBufferScene>(sceneId, TextureBufferScene::EState_RGBA8_OneMip);
        const Bool switchToSceneTexture = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMip", 0u);
        testFramework.getScenesRegistry().setSceneState<TextureBufferScene>(sceneId, TextureBufferScene::EState_SwitchBackToClientTexture);
        const Bool switchBackToClientTexture = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMipGreen", 0u);
        return clientTextureCorrect && switchToSceneTexture && switchBackToClientTexture;
    }
    case TextureBufferTest_SwitchClientTextureToSceneTextureAndUpdate:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureBufferScene>(testFramework, TextureBufferScene::EState_RGBA8_OneMip);
        const Bool textureBufferCorrect = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMip", 0u);
        testFramework.getScenesRegistry().setSceneState<TextureBufferScene>(sceneId, TextureBufferScene::EState_ClientTextureResource_RGBA8);
        const Bool switchToClientTexture = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMipGreen", 0u);
        testFramework.getScenesRegistry().setSceneState<TextureBufferScene>(sceneId, TextureBufferScene::EState_SwitchBackToExistingTextureBufferAndUpdate);
        const Bool switchBackToTextureBuffer = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMipRed", 0u);
        return textureBufferCorrect && switchToClientTexture && switchBackToTextureBuffer;
    }
    case TextureBufferTest_ReMapScene:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureBufferScene>(testFramework, TextureBufferScene::EState_RGBA8_OneMip);
        const Bool beforeRemapping = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMip", 0u);

        testFramework.hideAndUnmap(sceneId);
        // just for confidence (reuse existing black image)
        const Bool blackAfterUnmap = testFramework.renderAndCompareScreenshot("DistributedScene_UnpublishedScene", 0u);

        testFramework.mapScene(sceneId);
        testFramework.showScene(sceneId);
        const Bool afterRemapping = testFramework.renderAndCompareScreenshot("TextureBuffer_RGBA8_OneMip", 0u);

        return beforeRemapping && blackAfterUnmap && afterRemapping;
    }
    case SamplerTest_ChangeData_ClientTexture:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureSamplerScene>(testFramework, TextureSamplerScene::EState_ClientTexture);
        if (!testFramework.renderAndCompareScreenshot("TextureSamplerScene_Initial"))
            return false;
        testFramework.getScenesRegistry().setSceneState<TextureSamplerScene>(sceneId, TextureSamplerScene::EState_SetClientTexture);
        return testFramework.renderAndCompareScreenshot("TextureSamplerScene_Changed");
    }
    case SamplerTest_ChangeData_TextureBufferToClientTexture:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureSamplerScene>(testFramework, TextureSamplerScene::EState_TextureBuffer);
        if (!testFramework.renderAndCompareScreenshot("TextureSamplerScene_Initial"))
            return false;
        testFramework.getScenesRegistry().setSceneState<TextureSamplerScene>(sceneId, TextureSamplerScene::EState_SetClientTexture);
        return testFramework.renderAndCompareScreenshot("TextureSamplerScene_Changed");
    }
    case SamplerTest_ChangeData_ClientTextureToTextureBuffer:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureSamplerScene>(testFramework, TextureSamplerScene::EState_ClientTexture);
        if (!testFramework.renderAndCompareScreenshot("TextureSamplerScene_Initial"))
            return false;
        testFramework.getScenesRegistry().setSceneState<TextureSamplerScene>(sceneId, TextureSamplerScene::EState_SetTextureBuffer);
        return testFramework.renderAndCompareScreenshot("TextureSamplerScene_Changed");
    }
    case SamplerTest_ChangeData_ClientTextureToRenderBuffer:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureSamplerScene>(testFramework, TextureSamplerScene::EState_ClientTexture);
        if (!testFramework.renderAndCompareScreenshot("TextureSamplerScene_Initial"))
            return false;
        testFramework.getScenesRegistry().setSceneState<TextureSamplerScene>(sceneId, TextureSamplerScene::EState_SetRenderBuffer);
        return testFramework.renderAndCompareScreenshot("TextureSamplerScene_ChangedBlue");
    }
    case SamplerTest_ChangeData_ClientTextureToStreamTexture:
    {
        const ramses::sceneId_t sceneId = createAndShowScene<TextureSamplerScene>(testFramework, TextureSamplerScene::EState_ClientTexture);
        if (!testFramework.renderAndCompareScreenshot("TextureSamplerScene_Initial"))
            return false;
        testFramework.getScenesRegistry().setSceneState<TextureSamplerScene>(sceneId, TextureSamplerScene::EState_SetStreamTexture);
        return testFramework.renderAndCompareScreenshot("TextureSamplerScene_Changed");
    }
    default:
        assert(!"Invalid texture rendering test ID!");
        return false;
    }
}

template <typename INTEGRATION_SCENE>
bool TextureRenderingTests::runBasicTest(RendererTestsFramework& testFramework, UInt32 sceneState, const String& expectedImageName, float maxAveragePercentErrorPerPixel)
{
    createAndShowScene<INTEGRATION_SCENE>(testFramework, sceneState);
    return testFramework.renderAndCompareScreenshot(expectedImageName, 0u, maxAveragePercentErrorPerPixel);
}

template <typename INTEGRATION_SCENE>
ramses::sceneId_t TextureRenderingTests::createAndShowScene(RendererTestsFramework& testFramework, UInt32 sceneState)
{
    const ramses::sceneId_t sceneId = testFramework.getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.subscribeScene(sceneId);
    testFramework.mapScene(sceneId);
    testFramework.showScene(sceneId);

    return sceneId;
}
