//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE2DFORMATSCENE_H
#define RAMSES_TEXTURE2DFORMATSCENE_H

#include "IntegrationScene.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/MipLevelData.h"

namespace ramses
{
    class TextureSampler;
    struct TextureSwizzle;
}

namespace ramses_internal
{
    class Texture2DFormatScene : public IntegrationScene
    {
    public:
        enum EState
        {
            EState_R8 = 0,
            EState_Swizzled_Luminance_Alpha,
            EState_RG8,
            EState_RGB8,
            EState_RGB565,
            EState_RGBA8,
            EState_RGBA4,
            EState_RGBA5551,
            EState_Swizzled_BGR8,
            EState_Swizzled_BGRA8,
            EState_ETC2RGB,
            EState_ETC2RGBA,
            EState_ASTC_RGBA_4x4,
            EState_ASTC_SRGB_ALPHA_4x4,

            EState_R16F,
            EState_R32F,
            EState_RG16F,
            EState_RG32F,
            EState_RGB16F,
            EState_RGB32F,
            EState_RGBA16F,
            EState_RGBA32F,
            EState_SRGB8,
            EState_SRGB8_ALPHA8
        };

        Texture2DFormatScene(ramses::Scene& scene, uint32_t state, const Vector3& cameraPosition);

    protected:
        void createOrthoCamera();
        void createQuad(const ramses::TextureSampler& sampler);

        const ramses::MipLevelData& getTextureFormatAndData(EState state, ramses::ETextureFormat& format, uint32_t& width, uint32_t& height, ramses::TextureSwizzle& swizzle) const;
    };
}

#endif
