//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Texture3DImpl.h"
#include "ramses-client-api/Texture3D.h"

namespace ramses
{
    Texture3D::Texture3D(std::unique_ptr<Texture3DImpl> impl)
        : Resource{ std::move(impl) }
        , m_impl{ static_cast<Texture3DImpl&>(Resource::m_impl) }
    {
    }

    uint32_t Texture3D::getWidth() const
    {
        return m_impl.getWidth();
    }

    uint32_t Texture3D::getHeight() const
    {
        return m_impl.getHeight();
    }

    uint32_t Texture3D::getDepth() const
    {
        return m_impl.getDepth();
    }

    ETextureFormat Texture3D::getTextureFormat() const
    {
        return m_impl.getTextureFormat();
    }
}

