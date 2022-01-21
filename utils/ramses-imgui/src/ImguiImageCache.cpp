//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ImguiImageCache.h"
#include "TextureUtils.h"
#include "ramses-client-api/Scene.h"
#include "Resource/TextureResource.h"

namespace ramses_internal
{
    ImguiImageCache::ImguiImageCache(ramses::Scene* scene)
        : m_scene(scene)
    {
    }

    imgui::Image ImguiImageCache::get(const TextureResource* res)
    {
        const auto it = m_images.find(res);

        imgui::Image image = {nullptr, 0, 0};
        if (it == m_images.end())
        {
            const auto& swizzle = res->getTextureSwizzle();
            if (res->isCompressedAvailable() && !res->isDeCompressedAvailable())
            {
                res->decompress();
            }
            const auto&                  blob           = res->getResourceData();
            const ramses::TextureSwizzle textureSwizzle = {
                static_cast<ramses::ETextureChannelColor>(swizzle[0]),
                static_cast<ramses::ETextureChannelColor>(swizzle[1]),
                static_cast<ramses::ETextureChannelColor>(swizzle[2]),
                static_cast<ramses::ETextureChannelColor>(swizzle[3]),
            };
            ramses::MipLevelData mipLevelData(static_cast<uint32_t>(blob.size()), blob.data());
            const auto           mipMapCount = static_cast<uint32_t>(res->getMipDataSizes().size());
            ramses::Texture2D*   texture     = m_scene->createTexture2D(ramses::TextureUtils::GetTextureFormatFromInternal(res->getTextureFormat()),
                                                                  res->getWidth(),
                                                                  res->getHeight(),
                                                                  mipMapCount,
                                                                  &mipLevelData,
                                                                  false,
                                                                  textureSwizzle);

            image.width  = res->getWidth();
            image.height = res->getHeight();

            image.sampler = m_scene->createTextureSampler(ramses::ETextureAddressMode_Clamp,
                                                          ramses::ETextureAddressMode_Clamp,
                                                          ramses::ETextureSamplingMethod_Linear_MipMapLinear,
                                                          ramses::ETextureSamplingMethod_Linear,
                                                          *texture);
            m_images[res] = image;
        }
        else
        {
            image = it->second;
        }

        return image;
    }
} // namespace ramses_internal
