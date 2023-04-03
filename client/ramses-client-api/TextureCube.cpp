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
    TextureCube::TextureCube(TextureCubeImpl& pimpl)
        : Resource(pimpl)
        , impl(pimpl)
    {
    }

    TextureCube::~TextureCube()
    {
    }

    ETextureFormat TextureCube::getTextureFormat() const
    {
        return impl.getTextureFormat();
    }

    uint32_t TextureCube::getSize() const
    {
        return impl.getSize();
    }

    const TextureSwizzle& TextureCube::getTextureSwizzle() const
    {
        return impl.getTextureSwizzle();
    }
}
