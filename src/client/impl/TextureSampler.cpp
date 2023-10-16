//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/TextureSampler.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/RenderBuffer.h"
#include "impl/APILoggingMacros.h"

// internal
#include "impl/TextureSamplerImpl.h"

namespace ramses
{
    TextureSampler::TextureSampler(std::unique_ptr<internal::TextureSamplerImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::TextureSamplerImpl&>(SceneObject::m_impl) }
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

    bool TextureSampler::setTextureData(const Texture2D& dataSource)
    {
        const bool status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    bool TextureSampler::setTextureData(const Texture3D& dataSource)
    {
        const bool status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    bool TextureSampler::setTextureData(const TextureCube& dataSource)
    {
        const bool status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    bool TextureSampler::setTextureData(const Texture2DBuffer& dataSource)
    {
        const bool status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    bool TextureSampler::setTextureData(const RenderBuffer& dataSource)
    {
        const bool status = m_impl.setTextureData(dataSource);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataSource));
        return status;
    }

    internal::TextureSamplerImpl& TextureSampler::impl()
    {
        return m_impl;
    }

    const internal::TextureSamplerImpl& TextureSampler::impl() const
    {
        return m_impl;
    }
}
