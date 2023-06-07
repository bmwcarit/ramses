//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Texture3DImpl.h"
#include "TextureUtils.h"
#include "Components/ManagedResource.h"
#include "ClientApplicationLogic.h"

#include <string_view>

namespace ramses
{
    Texture3DImpl::Texture3DImpl(ramses_internal::ResourceHashUsage resource,
        SceneImpl& scene,
        std::string_view name)
        : ResourceImpl(ERamsesObjectType::Texture3D, std::move(resource), scene, name)
        , m_width(0)
        , m_height(0)
        , m_depth(0)
        , m_textureFormat(ETextureFormat::Invalid)
    {
    }

    Texture3DImpl::~Texture3DImpl()
    {
    }

    void Texture3DImpl::initializeFromFrameworkData(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat textureFormat)
    {
        m_width = width;
        m_height = height;
        m_depth = depth;
        m_textureFormat = textureFormat;
    }

    status_t Texture3DImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(ResourceImpl::serialize(outStream, serializationContext));

        outStream << m_width;
        outStream << m_height;
        outStream << m_depth;
        outStream << static_cast<uint32_t>(m_textureFormat);

        return StatusOK;
    }

    status_t Texture3DImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(ResourceImpl::deserialize(inStream, serializationContext));

        inStream >> m_width;
        inStream >> m_height;
        inStream >> m_depth;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_textureFormat = static_cast<ETextureFormat>(enumInt);

        return StatusOK;
    }

    uint32_t Texture3DImpl::getWidth() const
    {
        return m_width;
    }

    uint32_t Texture3DImpl::getHeight() const
    {
        return m_height;
    }

    uint32_t Texture3DImpl::getDepth() const
    {
        return m_depth;
    }

    ETextureFormat Texture3DImpl::getTextureFormat() const
    {
        return m_textureFormat;
    }
}
