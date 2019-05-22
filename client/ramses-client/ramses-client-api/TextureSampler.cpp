//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/StreamTexture.h"
#include "APILoggingMacros.h"

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

    ETextureSamplingMethod TextureSampler::getMinSamplingMethod() const
    {
        return impl.getMinSamplingMethod();
    }

    ETextureSamplingMethod TextureSampler::getMagSamplingMethod() const
    {
        return impl.getMagSamplingMethod();
    }

    uint32_t TextureSampler::getAnisotropyLevel() const
    {
        return impl.getAnisotropyLevel();
    }

    ERamsesObjectType TextureSampler::getTextureType() const
    {
        return impl.getTextureType();
    }

    status_t TextureSampler::setTextureData(const Texture2D& dataSource)
    {
        const status_t status = impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const Texture3D& dataSource)
    {
        const status_t status = impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const TextureCube& dataSource)
    {
        const status_t status = impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const Texture2DBuffer& dataSource)
    {
        const status_t status = impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const RenderBuffer& dataSource)
    {
        const status_t status = impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const StreamTexture& dataSource)
    {
        const status_t status = impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }
}
