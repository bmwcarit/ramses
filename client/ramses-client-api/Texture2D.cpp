//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Texture2DImpl.h"
#include "ramses-client-api/Texture2D.h"

namespace ramses
{

    Texture2D::Texture2D(std::unique_ptr<Texture2DImpl> impl)
        : Resource{ std::move(impl) }
        , m_impl{ static_cast<Texture2DImpl&>(Resource::m_impl) }
    {
    }

    uint32_t Texture2D::getWidth() const
    {
        return m_impl.getWidth();
    }

    uint32_t Texture2D::getHeight() const
    {
        return m_impl.getHeight();
    }

    ETextureFormat Texture2D::getTextureFormat() const
    {
        return m_impl.getTextureFormat();
    }

    const TextureSwizzle& Texture2D::getTextureSwizzle() const
    {
        return m_impl.getTextureSwizzle();
    }
}

