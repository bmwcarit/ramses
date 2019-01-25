//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURERESOURCE_H
#define RAMSES_TEXTURERESOURCE_H

#include "Resource/TextureMetaInfo.h"
#include "Resource/BufferResource.h"
#include <numeric>

namespace ramses_internal
{
    class TextureResource : public BufferResource
    {
    public:
        TextureResource(EResourceType typeID, const TextureMetaInfo& texDesc, ResourceCacheFlag cacheFlag, const String& name)
            : BufferResource(typeID, GetTotalDataSizeFromMipSizes(texDesc.m_dataSizes, typeID), nullptr, cacheFlag, name)
            , m_width(texDesc.m_width)
            , m_height(texDesc.m_height)
            , m_depth(texDesc.m_depth)
            , m_mipDataSizes(texDesc.m_dataSizes)
            , m_format(texDesc.m_format)
            , m_generateMipChain(texDesc.m_generateMipChain)
        {
            assert(m_width != 0u);
            assert(m_height != 0u);
            assert(m_depth != 0u);
            assert((texDesc.m_dataSizes.size() == 1) || !texDesc.m_generateMipChain);
        };

        virtual ~TextureResource() override
        {
        };

        UInt32 getWidth() const
        {
            return m_width;
        }

        UInt32 getHeight() const
        {
            return m_height;
        }

        UInt32 getDepth() const
        {
            return m_depth;
        }

        const MipDataSizeVector& getMipDataSizes() const
        {
            return m_mipDataSizes;
        }

        ETextureFormat getTextureFormat() const
        {
            return m_format;
        }

        Bool getGenerateMipChainFlag() const
        {
            return m_generateMipChain;
        }

        virtual void serializeResourceMetadataToStream(IOutputStream& output) const override final
        {
            switch (getTypeID())
            {
            case EResourceType_Texture2D:
                output << m_width;
                output << m_height;
                break;
            case EResourceType_Texture3D:
                output << m_width;
                output << m_height;
                output << m_depth;
                break;
            case EResourceType_TextureCube:
                output << m_width;
                break;
            default:
                assert(false);
            }

            output << static_cast<UInt32>(m_format);
            output << static_cast<UInt32>(m_mipDataSizes.size());
            for (const auto mipSize : m_mipDataSizes)
            {
                output << mipSize;
            }
            output << m_generateMipChain;
        }

        static IResource* CreateResourceFromMetadataStream(IInputStream& input, EResourceType typeID, ResourceCacheFlag cacheFlag, const String& name)
        {
            TextureMetaInfo texDesc(1u, 1u, 1u);
            UInt32 texelFormat = 0;
            UInt32 mipLevelCount = 0;
            UInt32 mipLevelSize = 0;

            switch (typeID)
            {
            case EResourceType_Texture2D:
                input >> texDesc.m_width;
                input >> texDesc.m_height;
                break;
            case EResourceType_Texture3D:
                input >> texDesc.m_width;
                input >> texDesc.m_height;
                input >> texDesc.m_depth;
                break;
            case EResourceType_TextureCube:
                input >> texDesc.m_width;
                break;
            default:
                assert(false);
            }

            input >> texelFormat;
            texDesc.m_format = static_cast<ETextureFormat>(texelFormat);
            input >> mipLevelCount;
            texDesc.m_dataSizes.reserve(mipLevelCount);
            for (UInt32 ii = 0; ii < mipLevelCount; ++ii)
            {
                input >> mipLevelSize;
                texDesc.m_dataSizes.push_back(mipLevelSize);
            }
            input >> texDesc.m_generateMipChain;

            return new TextureResource(typeID, texDesc, cacheFlag, name);
        }

        virtual bool isCompressable() const override
        {
            return !IsFormatCompressed(getTextureFormat());
        }

    private:
        static UInt32 GetTotalDataSizeFromMipSizes(const MipDataSizeVector& mipSizes, EResourceType typeID)
        {
            const UInt32 totalMipChainDataSize = std::accumulate(mipSizes.begin(), mipSizes.end(), 0u);
            return (typeID == EResourceType_TextureCube ? 6u * totalMipChainDataSize : totalMipChainDataSize);
        }

        UInt32            m_width;
        UInt32            m_height;
        UInt32            m_depth;
        MipDataSizeVector m_mipDataSizes;
        ETextureFormat    m_format;
        Bool              m_generateMipChain;
    };
}

#endif
