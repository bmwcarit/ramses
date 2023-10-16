//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/TextureMetaInfo.h"
#include "internal/SceneGraph/Resource/BufferResource.h"

#include <numeric>
#include <string_view>

namespace ramses::internal
{
    class TextureResource : public BufferResource
    {
    public:
        TextureResource(EResourceType typeID, const TextureMetaInfo& texDesc, std::string_view name)
            : BufferResource(typeID, GetTotalDataSizeFromMipSizes(texDesc.m_dataSizes, typeID), nullptr, name)
            , m_width(texDesc.m_width)
            , m_height(texDesc.m_height)
            , m_depth(texDesc.m_depth)
            , m_mipDataSizes(texDesc.m_dataSizes)
            , m_format(texDesc.m_format)
            , m_swizzle(texDesc.m_swizzle)
            , m_generateMipChain(texDesc.m_generateMipChain)
        {
            assert(m_width != 0u);
            assert(m_height != 0u);
            assert(m_depth != 0u);
            assert((texDesc.m_dataSizes.size() == 1) || !texDesc.m_generateMipChain);
        };

        uint32_t getWidth() const
        {
            return m_width;
        }

        uint32_t getHeight() const
        {
            return m_height;
        }

        uint32_t getDepth() const
        {
            return m_depth;
        }

        const MipDataSizeVector& getMipDataSizes() const
        {
            return m_mipDataSizes;
        }

        EPixelStorageFormat getTextureFormat() const
        {
            return m_format;
        }

        const TextureSwizzleArray& getTextureSwizzle() const
        {
            return m_swizzle;
        }

        bool getGenerateMipChainFlag() const
        {
            return m_generateMipChain;
        }

        void serializeResourceMetadataToStream(IOutputStream& output) const final override
        {
            switch (getTypeID())
            {
            case EResourceType::Texture2D:
                output << m_width;
                output << m_height;
                break;
            case EResourceType::Texture3D:
                output << m_width;
                output << m_height;
                output << m_depth;
                break;
            case EResourceType::TextureCube:
                output << m_width;
                break;
            default:
                assert(false);
            }

            output << static_cast<uint32_t>(m_format);
            for (const auto swizzle : m_swizzle)
            {
                output << swizzle;
            }
            output << static_cast<uint32_t>(m_mipDataSizes.size());
            for (const auto mipSize : m_mipDataSizes)
            {
                output << mipSize;
            }
            output << m_generateMipChain;
        }

        static std::unique_ptr<IResource> CreateResourceFromMetadataStream(IInputStream& input, EResourceType typeID, std::string_view name)
        {
            TextureMetaInfo texDesc(1u, 1u, 1u);
            uint32_t texelFormat = 0;
            uint32_t mipLevelCount = 0;
            uint32_t mipLevelSize = 0;

            switch (typeID)
            {
            case EResourceType::Texture2D:
                input >> texDesc.m_width;
                input >> texDesc.m_height;
                break;
            case EResourceType::Texture3D:
                input >> texDesc.m_width;
                input >> texDesc.m_height;
                input >> texDesc.m_depth;
                break;
            case EResourceType::TextureCube:
                input >> texDesc.m_width;
                break;
            default:
                assert(false);
            }

            input >> texelFormat;
            texDesc.m_format = static_cast<EPixelStorageFormat>(texelFormat);
            static_assert(texDesc.m_swizzle.size() == 4, "Wrong size of texture swizzle array");
            for (uint32_t ii = 0; ii < 4; ++ii)
            {
                input >> texDesc.m_swizzle[ii];
            }
            input >> mipLevelCount;
            texDesc.m_dataSizes.reserve(mipLevelCount);
            for (uint32_t ii = 0; ii < mipLevelCount; ++ii)
            {
                input >> mipLevelSize;
                texDesc.m_dataSizes.push_back(mipLevelSize);
            }

            input >> texDesc.m_generateMipChain;

            return std::make_unique<TextureResource>(typeID, texDesc, name);
        }

    private:
        static uint32_t GetTotalDataSizeFromMipSizes(const MipDataSizeVector& mipSizes, EResourceType typeID)
        {
            const uint32_t totalMipChainDataSize = std::accumulate(mipSizes.begin(), mipSizes.end(), 0u);
            return (typeID == EResourceType::TextureCube ? 6u * totalMipChainDataSize : totalMipChainDataSize);
        }

        uint32_t                    m_width;
        uint32_t                    m_height;
        uint32_t                    m_depth;
        MipDataSizeVector           m_mipDataSizes;
        EPixelStorageFormat         m_format;
        TextureSwizzleArray         m_swizzle;
        bool                        m_generateMipChain;
    };
}
