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
    Texture3D::Texture3D(Texture3DImpl& pimpl)
        : Resource(pimpl)
        , impl(pimpl)
    {
    }

    Texture3D::~Texture3D()
    {
    }

    uint32_t Texture3D::getWidth() const
    {
        return impl.getWidth();
    }

    uint32_t Texture3D::getHeight() const
    {
        return impl.getHeight();
    }

    uint32_t Texture3D::getDepth() const
    {
        return impl.getDepth();
    }

    ETextureFormat Texture3D::getTextureFormat() const
    {
        return impl.getTextureFormat();
    }
}

