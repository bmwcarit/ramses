//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ImguiImageCache.h"
#include "impl/TextureUtils.h"
#include "ramses/client/Scene.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/SceneGraph/SceneAPI/TextureBuffer.h"

namespace ramses::internal
{
    ImguiImageCache::ImguiImageCache(ramses::Scene* scene)
        : m_scene(scene)
    {
    }

    imgui::Image ImguiImageCache::get(const MipMap& mm, EPixelStorageFormat format)
    {
        const auto it = m_imageBuffers.find(mm.data.data());
        imgui::Image image = {nullptr, 0, 0};
        if (it == m_imageBuffers.end())
        {
            const ramses::TextureSwizzle textureSwizzle;
            std::vector<ramses::MipLevelData> mipLevelData{ mm.data };
            auto* texture = m_scene->createTexture2D(TextureUtils::GetTextureFormatFromInternal(format),
                                                     mm.width,
                                                     mm.height,
                                                     mipLevelData,
                                                     false,
                                                     textureSwizzle);
            image.width  = mm.width;
            image.height = mm.height;
            image.sampler = m_scene->createTextureSampler(ramses::ETextureAddressMode::Clamp,
                                                          ramses::ETextureAddressMode::Clamp,
                                                          ramses::ETextureSamplingMethod::Linear_MipMapLinear,
                                                          ramses::ETextureSamplingMethod::Linear,
                                                          *texture);
            m_imageBuffers[mm.data.data()] = image;
        }
        else
        {
            image = it->second;
        }
        return image;
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

            std::vector<ramses::MipLevelData> mipLevelData;
            mipLevelData.reserve(res->getMipDataSizes().size());
            const auto* data = blob.data();
            for (const auto& mipSize : res->getMipDataSizes())
            {
                assert(data + mipSize <= blob.data() + blob.size());
                mipLevelData.emplace_back(data, data + mipSize);
                data += mipSize;
            }

            ramses::Texture2D*   texture     = m_scene->createTexture2D(TextureUtils::GetTextureFormatFromInternal(res->getTextureFormat()),
                                                                  res->getWidth(),
                                                                  res->getHeight(),
                                                                  mipLevelData,
                                                                  false,
                                                                  textureSwizzle);

            image.width  = res->getWidth();
            image.height = res->getHeight();

            image.sampler = m_scene->createTextureSampler(ramses::ETextureAddressMode::Clamp,
                                                          ramses::ETextureAddressMode::Clamp,
                                                          ramses::ETextureSamplingMethod::Linear_MipMapLinear,
                                                          ramses::ETextureSamplingMethod::Linear,
                                                          *texture);
            m_images[res] = image;
        }
        else
        {
            image = it->second;
        }

        return image;
    }
} // namespace ramses::internal
