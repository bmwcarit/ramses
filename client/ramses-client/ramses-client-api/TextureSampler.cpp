//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/TextureSampler.h"

// internal
#include "TextureSamplerImpl.h"

namespace ramses
{
    TextureSampler::TextureSampler(TextureSamplerImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    TextureSampler::~TextureSampler()
    {
    }

    ETextureAddressMode TextureSampler::getWrapUMode() const
    {
        return impl.getWrapUMode();
    }

    ETextureAddressMode TextureSampler::getWrapVMode() const
    {
        return impl.getWrapVMode();
    }

    ETextureAddressMode TextureSampler::getWrapRMode() const
    {
        return impl.getWrapRMode();
    }

    ETextureSamplingMethod TextureSampler::getSamplingMethod() const
    {
        return impl.getSamplingMethod();
    }

    uint32_t TextureSampler::getAnisotropyLevel() const
    {
        return impl.getAnisotropyLevel();
    }

    ERamsesObjectType TextureSampler::getTextureType() const
    {
        return impl.getTextureType();
    }
}
