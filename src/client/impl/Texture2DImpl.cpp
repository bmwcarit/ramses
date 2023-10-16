//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/Texture2DImpl.h"
#include "impl/TextureUtils.h"
#include "internal/Components/ManagedResource.h"
#include "impl/ResourceImpl.h"
#include "internal/ClientApplicationLogic.h"

#include <string_view>

namespace ramses::internal
{
    Texture2DImpl::Texture2DImpl(ramses::internal::ResourceHashUsage resource,
        SceneImpl& scene,
        std::string_view name,
        ERamsesObjectType overrideType /* = ERamsesObjectType_Texture2D*/)
        : ResourceImpl(overrideType, std::move(resource), scene, name)
        , m_width(0)
        , m_height(0)
        , m_textureFormat(ETextureFormat::RGBA8)
    {
    }

    Texture2DImpl::~Texture2DImpl() = default;

    void Texture2DImpl::initializeFromFrameworkData(uint32_t width, uint32_t height, ETextureFormat textureFormat, const TextureSwizzle& swizzle)
    {
        m_width = width;
        m_height = height;
        m_textureFormat = textureFormat;
        m_swizzle = swizzle;
    }

    bool Texture2DImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!ResourceImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_width;
        outStream << m_height;
        outStream << static_cast<uint32_t>(m_textureFormat);
        outStream << m_swizzle.channelRed;
        outStream << m_swizzle.channelGreen;
        outStream << m_swizzle.channelBlue;
        outStream << m_swizzle.channelAlpha;

        return true;
    }

    bool Texture2DImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!ResourceImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_width;
        inStream >> m_height;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_textureFormat = static_cast<ETextureFormat>(enumInt);
        inStream >> m_swizzle.channelRed;
        inStream >> m_swizzle.channelGreen;
        inStream >> m_swizzle.channelBlue;
        inStream >> m_swizzle.channelAlpha;

        return true;
    }

    uint32_t Texture2DImpl::getWidth() const
    {
        return m_width;
    }

    uint32_t Texture2DImpl::getHeight() const
    {
        return m_height;
    }

    ETextureFormat Texture2DImpl::getTextureFormat() const
    {
        return m_textureFormat;
    }

    const TextureSwizzle& Texture2DImpl::getTextureSwizzle() const
    {
        return m_swizzle;
    }

}

