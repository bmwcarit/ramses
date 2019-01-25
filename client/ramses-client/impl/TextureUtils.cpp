//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextureUtils.h"
#include "Utils/TextureMathUtils.h"
#include "Utils/LogMacros.h"
#include "Utils/Image.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses
{
    void TextureUtils::FillMipDataSizes(ramses_internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const MipLevelData mipLevelData[])
    {
        assert(mipDataSizes.empty());
        mipDataSizes.reserve(mipMapCount);
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_size;
            mipDataSizes.push_back(mipDataSize);
        }
    }

    void TextureUtils::FillMipDataSizes(ramses_internal::MipDataSizeVector& mipDataSizes, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[])
    {
        assert(mipDataSizes.empty());
        mipDataSizes.reserve(mipMapCount);
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_faceDataSize;
            mipDataSizes.push_back(mipDataSize);
        }
    }

    void TextureUtils::FillMipData(uint8_t* dest, uint32_t mipMapCount, const MipLevelData mipLevelData[])
    {
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_size;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_data, mipDataSize);
            dest += mipDataSize;
        }
    }

    void TextureUtils::FillMipData(uint8_t* dest, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[])
    {
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_faceDataSize;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_dataPX, mipDataSize);
            dest += mipDataSize;
        }
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_faceDataSize;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_dataNX, mipDataSize);
            dest += mipDataSize;
        }
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_faceDataSize;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_dataPY, mipDataSize);
            dest += mipDataSize;
        }
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_faceDataSize;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_dataNY, mipDataSize);
            dest += mipDataSize;
        }
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_faceDataSize;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_dataPZ, mipDataSize);
            dest += mipDataSize;
        }
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_faceDataSize;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_dataNZ, mipDataSize);
            dest += mipDataSize;
        }
    }

    bool TextureUtils::MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const MipLevelData mipLevelData[], ETextureFormat format)
    {
        if (mipMapCount == 0u || mipLevelData == nullptr)
        {
            return false;
        }

        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            if (mipLevelData[i].m_data == nullptr || mipLevelData[i].m_size == 0u)
            {
                return false;
            }

            const ramses_internal::ETextureFormat internalFormat = TextureUtils::GetTextureFormatInternal(format);
            if (!ramses_internal::IsFormatCompressed(internalFormat))
            {
                const uint32_t mipWidth = ramses_internal::TextureMathUtils::GetMipSize(i, width);
                const uint32_t mipHeight = ramses_internal::TextureMathUtils::GetMipSize(i, height);
                const uint32_t mipDepth = ramses_internal::TextureMathUtils::GetMipSize(i, depth);
                const uint32_t expectedMipDataSize = mipWidth * mipHeight * mipDepth * ramses_internal::GetTexelSizeFromFormat(internalFormat);
                if (mipLevelData[i].m_size < expectedMipDataSize)
                {
                    return false;
                }
                else if (mipLevelData[i].m_size > expectedMipDataSize)
                {
                    LOG_WARN(ramses_internal::CONTEXT_CLIENT, "Provided texture mip data does not match expected size, texture might not be as expected");
                }
            }
        }

        return true;
    }

    bool TextureUtils::MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format)
    {
        // wrapper so that this function can be called from template following 2D/3D texture function signature
        UNUSED(height);
        UNUSED(depth);
        return MipDataValid(width, mipMapCount, mipLevelData, format);
    }

    bool TextureUtils::MipDataValid(uint32_t size, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format)
    {
        if (mipMapCount == 0u || mipLevelData == nullptr)
        {
            return false;
        }

        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            if (mipLevelData[i].m_dataPX == nullptr ||
                mipLevelData[i].m_dataNX == nullptr ||
                mipLevelData[i].m_dataPY == nullptr ||
                mipLevelData[i].m_dataNY == nullptr ||
                mipLevelData[i].m_dataPZ == nullptr ||
                mipLevelData[i].m_dataNZ == nullptr ||
                mipLevelData[i].m_faceDataSize == 0u)
            {
                return false;
            }

            const ramses_internal::ETextureFormat internalFormat = TextureUtils::GetTextureFormatInternal(format);
            if (!ramses_internal::IsFormatCompressed(internalFormat))
            {
                const uint32_t mipSize = ramses_internal::TextureMathUtils::GetMipSize(i, size);
                const uint32_t expectedMipDataSize = mipSize * mipSize * ramses_internal::GetTexelSizeFromFormat(internalFormat);
                if (mipLevelData[i].m_faceDataSize < expectedMipDataSize)
                {
                    return false;
                }
                else if (mipLevelData[i].m_faceDataSize > expectedMipDataSize)
                {
                    LOG_WARN(ramses_internal::CONTEXT_CLIENT, "Provided texture mip data does not match expected size, texture might not be as expected");
                }
            }
        }

        return true;
    }

    bool TextureUtils::TextureParametersValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount)
    {
        if (width == 0u || height == 0u || depth == 0u)
        {
            return false;
        }

        const uint32_t fullChainMipMapCount = ramses_internal::TextureMathUtils::GetMipLevelCount(width, height, depth);
        if (mipMapCount > fullChainMipMapCount)
        {
            return false;
        }

        return true;
    }

    Texture2D* TextureUtils::CreateTextureResourceFromPng(const char* filePath, RamsesClient& client, const char* name)
    {
        ramses_internal::Image image;
        image.loadFromFilePNG(filePath);

        if (image.getNumberOfPixels() == 0u)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "CreateTextureResourceFromPNG:  Could not load PNG. File not found or format not supported: " << filePath);
            return nullptr;
        }

        const MipLevelData mipLevelData(static_cast<uint32_t>(image.getData().size()), image.getData().data());
        return client.createTexture2D(image.getWidth(), image.getHeight(), ETextureFormat_RGBA8, 1, &mipLevelData, false, ResourceCacheFlag_DoNotCache, name);
    }
}
