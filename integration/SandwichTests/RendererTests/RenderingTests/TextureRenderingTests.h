//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURERENDERINGTESTS_H
#define RAMSES_TEXTURERENDERINGTESTS_H

#include "IRendererTest.h"

class TextureRenderingTests : public IRendererTest
{
public:
    virtual void setUpTestCases(RendererTestsFramework& testFramework) final;
    virtual bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    template <typename INTEGRATION_SCENE>
    ramses::sceneId_t createAndShowScene(RendererTestsFramework& testFramework, ramses_internal::UInt32 sceneState);

    template <typename INTEGRATION_SCENE>
    bool runBasicTest(RendererTestsFramework& testFramework, ramses_internal::UInt32 sceneState, const ramses_internal::String& expectedImageName, float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel);

    enum
    {
        TextureTest_Texture2D_Format_R8 = 0,
        TextureTest_Texture2D_Format_RG8,
        TextureTest_Texture2D_Format_RGB8,
        TextureTest_Texture2D_Format_RGB565,
        TextureTest_Texture2D_Format_RGBA8,
        TextureTest_Texture2D_Format_RGBA4,
        TextureTest_Texture2D_Format_RGBA5551,
        TextureTest_Texture2D_Format_BGR8,
        TextureTest_Texture2D_Format_BGRA8,
        TextureTest_Texture2D_Format_ETC2RGB,
        TextureTest_Texture2D_Format_ETC2RGBA,
        TextureTest_Texture2D_Format_R16F,
        TextureTest_Texture2D_Format_R32F,
        TextureTest_Texture2D_Format_RG16F,
        TextureTest_Texture2D_Format_RG32F,
        TextureTest_Texture2D_Format_RGB16F,
        TextureTest_Texture2D_Format_RGB32F,
        TextureTest_Texture2D_Format_RGBA16F,
        TextureTest_Texture2D_Format_RGBA32F,
        TextureTest_Texture2D_Format_SRGB8,
        TextureTest_Texture2D_Format_SRGB8_ALPHA8,
        TextureTest_Texture2D_Format_ASTC_RGBA_4x4,
        TextureTest_Texture2D_Format_ASTC_SRGB_ALPHA_4x4,

        TextureTest_Texture2D_Sampling_Nearest,
        TextureTest_Texture2D_Sampling_NearestWithMipMaps,
        TextureTest_Texture2D_Sampling_Bilinear,
        TextureTest_Texture2D_Sampling_BilinearWithMipMaps,
        TextureTest_Texture2D_Sampling_Trilinear,
        TextureTest_Texture2D_Sampling_MinLinearMagNearest,
        TextureTest_Texture2D_Sampling_MinNearestMagLinear,

        TextureTest_Texture2D_AddressMode,
        TextureTest_Texture2D_AnisotropicFilter,
        TextureTest_Texture2D_GenerateMipMapSingle,
        TextureTest_Texture2D_GenerateMipMapMultiple,
        TextureTest_Texture2D_CompressedMipMap,

        TextureTest_Texture3D_RGBA8,
        TextureTest_CubeMap_RGBA8,
        TextureTest_CubeMap_Float,
        TextureTest_TextureCube_AnisotropicFilter,

        TextureBufferTest_RGBA8_OneMip,
        TextureBufferTest_RGBA8_ThreeMips,
        TextureBufferTest_PartialUpdate,
        TextureBufferTest_PartialUpdateMipMap,
        TextureBufferTest_PartialUpdateMipMap_RG8,
        TextureBufferTest_SwitchSceneTextureToClientTexture,
        TextureBufferTest_SwitchClientTextureToSceneTexture,
        TextureBufferTest_SwitchClientTextureToSceneTextureAndBack,
        TextureBufferTest_SwitchClientTextureToSceneTextureAndUpdate,
        TextureBufferTest_ReMapScene,

        SamplerTest_ChangeData_ClientTexture,
        SamplerTest_ChangeData_TextureBufferToClientTexture,
        SamplerTest_ChangeData_ClientTextureToTextureBuffer,
        SamplerTest_ChangeData_ClientTextureToRenderBuffer,
        SamplerTest_ChangeData_ClientTextureToStreamTexture
    };
};

#endif
