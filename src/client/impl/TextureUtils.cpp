//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/TextureUtils.h"
#include "internal/Core/Utils/TextureMathUtils.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/Image.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"

namespace ramses::internal
{
    void TextureUtils::FillMipDataSizes(ramses::internal::MipDataSizeVector& mipDataSizes, const std::vector<MipLevelData>& mipLevelData)
    {
        assert(mipDataSizes.empty());
        mipDataSizes.reserve(mipLevelData.size());
        for (const auto& data : mipLevelData)
        {
            mipDataSizes.push_back(static_cast<uint32_t>(data.size()));
        }
    }

    void TextureUtils::FillMipDataSizes(ramses::internal::MipDataSizeVector& mipDataSizes, const std::vector<CubeMipLevelData>& mipLevelData)
    {
        assert(mipDataSizes.empty());
        mipDataSizes.reserve(mipLevelData.size());
        for (const auto& data : mipLevelData)
        {
            mipDataSizes.push_back(static_cast<uint32_t>(data.m_dataPX.size()));
        }
    }

    void TextureUtils::FillMipData(std::byte* dest, const std::vector<MipLevelData>& mipLevelData)
    {
        for (const auto& data : mipLevelData)
        {
            const size_t mipDataSize = data.size();
            ramses::internal::PlatformMemory::Copy(dest, data.data(), mipDataSize);
            dest += mipDataSize;
        }
    }

    void TextureUtils::FillMipData(std::byte* dest, const std::vector<CubeMipLevelData>& mipLevelData)
    {
        for (const auto& data : mipLevelData)
        {
            const size_t mipDataSize = data.m_dataPX.size();
            ramses::internal::PlatformMemory::Copy(dest, data.m_dataPX.data(), mipDataSize);
            dest += mipDataSize;
        }
        for (const auto& data : mipLevelData)
        {
            const size_t mipDataSize = data.m_dataNX.size();
            ramses::internal::PlatformMemory::Copy(dest, data.m_dataNX.data(), mipDataSize);
            dest += mipDataSize;
        }
        for (const auto& data : mipLevelData)
        {
            const size_t mipDataSize = data.m_dataPY.size();
            ramses::internal::PlatformMemory::Copy(dest, data.m_dataPY.data(), mipDataSize);
            dest += mipDataSize;
        }
        for (const auto& data : mipLevelData)
        {
            const size_t mipDataSize = data.m_dataNY.size();
            ramses::internal::PlatformMemory::Copy(dest, data.m_dataNY.data(), mipDataSize);
            dest += mipDataSize;
        }
        for (const auto& data : mipLevelData)
        {
            const size_t mipDataSize = data.m_dataPZ.size();
            ramses::internal::PlatformMemory::Copy(dest, data.m_dataPZ.data(), mipDataSize);
            dest += mipDataSize;
        }
        for (const auto& data : mipLevelData)
        {
            const size_t mipDataSize = data.m_dataNZ.size();
            ramses::internal::PlatformMemory::Copy(dest, data.m_dataNZ.data(), mipDataSize);
            dest += mipDataSize;
        }
    }

    bool TextureUtils::MipDataValid(uint32_t width, uint32_t height, uint32_t depth, const std::vector<MipLevelData>& mipLevelData, ETextureFormat format)
    {
        if (mipLevelData.empty())
        {
            return false;
        }

        for (size_t i = 0u; i < mipLevelData.size(); ++i)
        {
            if (mipLevelData[i].empty())
            {
                return false;
            }

            const uint32_t mipWidth = ramses::internal::TextureMathUtils::GetMipSize(static_cast<uint32_t>(i), width);
            const uint32_t mipHeight = ramses::internal::TextureMathUtils::GetMipSize(static_cast<uint32_t>(i), height);
            const uint32_t mipDepth = ramses::internal::TextureMathUtils::GetMipSize(static_cast<uint32_t>(i), depth);
            const ramses::internal::EPixelStorageFormat internalFormat = TextureUtils::GetTextureFormatInternal(format);
            if (!ramses::internal::IsFormatCompressed(internalFormat))
            {
                const uint32_t expectedMipDataSize = mipWidth * mipHeight * mipDepth * ramses::internal::GetTexelSizeFromFormat(internalFormat);
                if (mipLevelData[i].size() < expectedMipDataSize)
                {
                    return false;
                }
                if (mipLevelData[i].size() > expectedMipDataSize)
                {
                    LOG_WARN(CONTEXT_CLIENT, "Provided texture mip data does not match expected size, texture might not be as expected");
                }
            }

            if (!TextureUtils::IsTextureSizeSupportedByFormat(mipWidth, mipHeight, format))
            {
                LOG_WARN(CONTEXT_CLIENT, "Provided texture mip {} might fail to be uploaded due to its size {}x{} not supported by used format {}",
                    i, mipWidth, mipHeight, toString(format));
            }
        }

        return true;
    }

    bool TextureUtils::MipDataValid(uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth, const std::vector<CubeMipLevelData>& mipLevelData, ETextureFormat format)
    {
        // wrapper so that this function can be called from template following 2D/3D texture function signature
        return MipDataValid(width, mipLevelData, format);
    }

    bool TextureUtils::MipDataValid(uint32_t size, const std::vector<CubeMipLevelData>& mipLevelData, ETextureFormat format)
    {
        if (mipLevelData.empty())
        {
            return false;
        }

        for (size_t i = 0u; i < mipLevelData.size(); ++i)
        {
            if (mipLevelData[i].m_dataPX.empty() ||
                mipLevelData[i].m_dataNX.empty() ||
                mipLevelData[i].m_dataPY.empty() ||
                mipLevelData[i].m_dataNY.empty() ||
                mipLevelData[i].m_dataPZ.empty() ||
                mipLevelData[i].m_dataNZ.empty() ||
                (mipLevelData[i].m_dataPX.size() != mipLevelData[i].m_dataNX.size() ||
                mipLevelData[i].m_dataNX.size() != mipLevelData[i].m_dataPY.size() ||
                mipLevelData[i].m_dataPY.size() != mipLevelData[i].m_dataNY.size() ||
                mipLevelData[i].m_dataNY.size() != mipLevelData[i].m_dataPZ.size() ||
                mipLevelData[i].m_dataPZ.size() != mipLevelData[i].m_dataNZ.size() ||
                mipLevelData[i].m_dataNZ.size() != mipLevelData[i].m_dataPX.size()))
            {
                return false;
            }

            const uint32_t mipSize = ramses::internal::TextureMathUtils::GetMipSize(static_cast<uint32_t>(i), size);
            const ramses::internal::EPixelStorageFormat internalFormat = TextureUtils::GetTextureFormatInternal(format);
            if (!ramses::internal::IsFormatCompressed(internalFormat))
            {
                const uint32_t expectedMipDataSize = mipSize * mipSize * ramses::internal::GetTexelSizeFromFormat(internalFormat);
                if (mipLevelData[i].m_dataPX.size() < expectedMipDataSize)
                {
                    return false;
                }
                if (mipLevelData[i].m_dataPX.size() > expectedMipDataSize)
                {
                    LOG_WARN(CONTEXT_CLIENT, "Provided texture mip data does not match expected size, texture might not be as expected");
                }
            }

            if (!TextureUtils::IsTextureSizeSupportedByFormat(mipSize, mipSize, format))
            {
                LOG_WARN(CONTEXT_CLIENT, "Provided texture mip {} might fail to be uploaded due to its size {}x{} not supported by used format {}",
                    i, mipSize, mipSize, toString(format));
            }
        }

        return true;
    }

    bool TextureUtils::TextureParametersValid(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount)
    {
        if (width == 0u || height == 0u || depth == 0u)
        {
            LOG_ERROR(CONTEXT_CLIENT, "TextureParametersValid: texture size cannot be 0.");
            return false;
        }

        const uint32_t fullChainMipMapCount = ramses::internal::TextureMathUtils::GetMipLevelCount(width, height, depth);
        if (mipMapCount > fullChainMipMapCount)
        {
            LOG_ERROR(CONTEXT_CLIENT, "TextureParametersValid: too many mip levels provided.");
            return false;
        }

        return true;
    }
}
