//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextureCubeImpl.h"
#include "TextureUtils.h"
#include "Components/ManagedResource.h"
#include "ClientApplicationLogic.h"

namespace ramses
{
    TextureCubeImpl::TextureCubeImpl(ramses_internal::ResourceHashUsage texture, RamsesClientImpl& client, const char* name)
        : ResourceImpl(ERamsesObjectType_TextureCube, texture, client, name)
        , m_size(0)
        , m_textureFormat(ETextureFormat_Invalid)
    {
    }

    TextureCubeImpl::~TextureCubeImpl()
    {
    }

    void TextureCubeImpl::initializeFromFrameworkData(uint32_t size, ETextureFormat textureFormat)
    {
        m_size = size;
        m_textureFormat = textureFormat;
    }

    status_t TextureCubeImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(ResourceImpl::serialize(outStream, serializationContext));

        outStream << m_size;
        outStream << static_cast<uint32_t>(m_textureFormat);

        return StatusOK;
    }

    status_t TextureCubeImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(ResourceImpl::deserialize(inStream, serializationContext));

        inStream >> m_size;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_textureFormat = static_cast<ETextureFormat>(enumInt);

        return StatusOK;
    }

    uint32_t TextureCubeImpl::getSize() const
    {
        return m_size;
    }

    ETextureFormat TextureCubeImpl::getTextureFormat() const
    {
        return m_textureFormat;
    }
}
