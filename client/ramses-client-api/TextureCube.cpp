//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextureCubeImpl.h"
#include "ramses-client-api/TextureCube.h"

namespace ramses
{
    TextureCube::TextureCube(std::unique_ptr<TextureCubeImpl> impl)
        : Resource{ std::move(impl) }
        , m_impl{ static_cast<TextureCubeImpl&>(Resource::m_impl) }
    {
    }

    ETextureFormat TextureCube::getTextureFormat() const
    {
        return m_impl.getTextureFormat();
    }

    uint32_t TextureCube::getSize() const
    {
        return m_impl.getSize();
    }

    const TextureSwizzle& TextureCube::getTextureSwizzle() const
    {
        return m_impl.getTextureSwizzle();
    }
}
