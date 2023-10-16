//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/TextureCubeImpl.h"
#include "impl/TextureUtils.h"
#include "internal/Components/ManagedResource.h"
#include "internal/ClientApplicationLogic.h"

namespace ramses::internal
{
    TextureCubeImpl::TextureCubeImpl(ramses::internal::ResourceHashUsage texture, SceneImpl& scene, std::string_view name)
        : ResourceImpl(ERamsesObjectType::TextureCube, std::move(texture), scene, name)
        , m_size(0)
        , m_textureFormat(ETextureFormat::RGBA8)
    {
    }

    TextureCubeImpl::~TextureCubeImpl() = default;

    void TextureCubeImpl::initializeFromFrameworkData(uint32_t size, ETextureFormat textureFormat, const TextureSwizzle& swizzle)
    {
        m_size = size;
        m_textureFormat = textureFormat;
        m_swizzle = swizzle;
    }

    bool TextureCubeImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!ResourceImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_size;
        outStream << static_cast<uint32_t>(m_textureFormat);
        outStream << m_swizzle.channelRed;
        outStream << m_swizzle.channelGreen;
        outStream << m_swizzle.channelBlue;
        outStream << m_swizzle.channelAlpha;

        return true;
    }

    bool TextureCubeImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!ResourceImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_size;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_textureFormat = static_cast<ETextureFormat>(enumInt);
        inStream >> m_swizzle.channelRed;
        inStream >> m_swizzle.channelGreen;
        inStream >> m_swizzle.channelBlue;
        inStream >> m_swizzle.channelAlpha;

        return true;
    }

    uint32_t TextureCubeImpl::getSize() const
    {
        return m_size;
    }

    ETextureFormat TextureCubeImpl::getTextureFormat() const
    {
        return m_textureFormat;
    }

    const TextureSwizzle& TextureCubeImpl::getTextureSwizzle() const
    {
        return m_swizzle;
    }
}
