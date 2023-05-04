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
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
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

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
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

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    void TextureUtils::FillMipData(uint8_t* dest, uint32_t mipMapCount, const MipLevelData mipLevelData[])
    {
        for (uint32_t i = 0u; i < mipMapCount; ++i)
        {
            const uint32_t mipDataSize = mipLevelData[i].m_size;
            ramses_internal::PlatformMemory::Copy(dest, mipLevelData[i].m_data, mipDataSize);
            dest += mipDataSize;
        }
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
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

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
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

            const uint32_t mipWidth = ramses_internal::TextureMathUtils::GetMipSize(i, width);
            const uint32_t mipHeight = ramses_internal::TextureMathUtils::GetMipSize(i, height);
            const uint32_t mipDepth = ramses_internal::TextureMathUtils::GetMipSize(i, depth);
            const ramses_internal::ETextureFormat internalFormat = TextureUtils::GetTextureFormatInternal(format);
            if (!ramses_internal::IsFormatCompressed(internalFormat))
            {
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

            if (!TextureUtils::IsTextureSizeSupportedByFormat(mipWidth, mipHeight, format))
            {
                LOG_WARN(ramses_internal::CONTEXT_CLIENT, "Provided texture mip " << i << " might fail to be uploaded due to its size "
                    << mipWidth << "x" << mipHeight << " not supported by used format " << getTextureFormatString(format));
            }
        }

        return true;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    bool TextureUtils::MipDataValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount, const CubeMipLevelData mipLevelData[], ETextureFormat format)
    {
        // wrapper so that this function can be called from template following 2D/3D texture function signature
        UNUSED(height);
        UNUSED(depth);
        return MipDataValid(width, mipMapCount, mipLevelData, format);
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
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

            const uint32_t mipSize = ramses_internal::TextureMathUtils::GetMipSize(i, size);
            const ramses_internal::ETextureFormat internalFormat = TextureUtils::GetTextureFormatInternal(format);
            if (!ramses_internal::IsFormatCompressed(internalFormat))
            {
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

            if (!TextureUtils::IsTextureSizeSupportedByFormat(mipSize, mipSize, format))
            {
                LOG_WARN(ramses_internal::CONTEXT_CLIENT, "Provided texture mip " << i << " might fail to be uploaded due to its size "
                    << mipSize << "x" << mipSize << " not supported by used format " << getTextureFormatString(format));
            }
        }

        return true;
    }

    bool TextureUtils::TextureParametersValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount)
    {
        if (width == 0u || height == 0u || depth == 0u)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "TextureParametersValid: texture size cannot be 0.");
            return false;
        }

        const uint32_t fullChainMipMapCount = ramses_internal::TextureMathUtils::GetMipLevelCount(width, height, depth);
        if (mipMapCount > fullChainMipMapCount)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "TextureParametersValid: too many mip levels provided.");
            return false;
        }

        return true;
    }
}
