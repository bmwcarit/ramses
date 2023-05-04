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
#include "APILoggingMacros.h"

// internal
#include "TextureSamplerImpl.h"

namespace ramses
{
    TextureSampler::TextureSampler(std::unique_ptr<TextureSamplerImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<TextureSamplerImpl&>(SceneObject::m_impl) }
    {
    }

    ETextureAddressMode TextureSampler::getWrapUMode() const
    {
        return m_impl.getWrapUMode();
    }

    ETextureAddressMode TextureSampler::getWrapVMode() const
    {
        return m_impl.getWrapVMode();
    }

    ETextureAddressMode TextureSampler::getWrapRMode() const
    {
        return m_impl.getWrapRMode();
    }

    ETextureSamplingMethod TextureSampler::getMinSamplingMethod() const
    {
        return m_impl.getMinSamplingMethod();
    }

    ETextureSamplingMethod TextureSampler::getMagSamplingMethod() const
    {
        return m_impl.getMagSamplingMethod();
    }

    uint32_t TextureSampler::getAnisotropyLevel() const
    {
        return m_impl.getAnisotropyLevel();
    }

    ERamsesObjectType TextureSampler::getTextureType() const
    {
        return m_impl.getTextureType();
    }

    status_t TextureSampler::setTextureData(const Texture2D& dataSource)
    {
        const status_t status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const Texture3D& dataSource)
    {
        const status_t status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const TextureCube& dataSource)
    {
        const status_t status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const Texture2DBuffer& dataSource)
    {
        const status_t status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    status_t TextureSampler::setTextureData(const RenderBuffer& dataSource)
    {
        const status_t status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }
}
