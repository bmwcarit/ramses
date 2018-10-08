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

    Texture2D::Texture2D(Texture2DImpl& pimpl)
        : Resource(pimpl)
        , impl(pimpl)
    {
    }

    Texture2D::~Texture2D()
    {
    }

    uint32_t Texture2D::getWidth() const
    {
        return impl.getWidth();
    }

    uint32_t Texture2D::getHeight() const
    {
        return impl.getHeight();
    }

    ETextureFormat Texture2D::getTextureFormat() const
    {
        return impl.getTextureFormat();
    }
}

