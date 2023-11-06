//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PlatformBase/RendererLimits.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    uint32_t RendererLimits::getMaximumTextureUnits() const
    {
        return m_maximumTextureUnits;
    }

    void RendererLimits::setMaximumTextureUnits(uint32_t units)
    {
        m_maximumTextureUnits = units;
    }

    uint32_t RendererLimits::getMaximumSamples() const
    {
        return m_maximumSamples;
    }

    void RendererLimits::setMaximumSamples(uint32_t samples)
    {
        m_maximumSamples = samples;
    }

    uint32_t RendererLimits::getMaximumAnisotropy() const
    {
        return m_maximumAnisotropy;
    }

    void RendererLimits::setMaximumAnisotropy(uint32_t anisotropy)
    {
        m_maximumAnisotropy = anisotropy;
    }

    void RendererLimits::setMaxViewport(uint32_t maxWidth, uint32_t maxHeight)
    {
        m_maxViewportWidth = maxWidth;
        m_maxViewportHeight = maxHeight;
    }

    uint32_t RendererLimits::getMaxViewportWidth() const
    {
        return m_maxViewportWidth;
    }

    uint32_t RendererLimits::getMaxViewportHeight() const
    {
        return m_maxViewportHeight;
    }

    uint32_t RendererLimits::getMaximumDrawBuffers() const
    {
        return m_maximumDrawBuffers;
    }

    void RendererLimits::setMaximumDrawBuffers(uint32_t drawBuffers)
    {
        m_maximumDrawBuffers = drawBuffers;
    }

    bool RendererLimits::isTextureFormatAvailable(EPixelStorageFormat format) const
    {
        return m_availableTextureFormats.contains(format);
    }

    void RendererLimits::addTextureFormat(EPixelStorageFormat format)
    {
        m_availableTextureFormats.put(format);
    }

    bool RendererLimits::isExternalTextureExtensionSupported() const
    {
        return m_externalTextureExtensionSupported;
    }

    void RendererLimits::setExternalTextureExtensionSupported(bool supported)
    {
        m_externalTextureExtensionSupported = supported;
    }

    void RendererLimits::logLimits() const
    {
        LOG_INFO(CONTEXT_RENDERER, "Rendering device features and limits:");
        LOG_INFO(CONTEXT_RENDERER, "  - maximum number of texture units:            " << m_maximumTextureUnits);
        LOG_INFO(CONTEXT_RENDERER, "  - maximum number of MSAA samples:             " << m_maximumSamples);
        LOG_INFO(CONTEXT_RENDERER, "  - maximum number of anisotropy samples:       " << m_maximumAnisotropy);
        LOG_INFO(CONTEXT_RENDERER, "  - maximum viewport size:                      " << m_maxViewportWidth << " x " << m_maxViewportHeight);
        LOG_INFO(CONTEXT_RENDERER, "  - maximum number of FBO draw buffers:         " << m_maximumDrawBuffers);
        LOG_INFO(CONTEXT_RENDERER, "  - external textures supported:                " << m_externalTextureExtensionSupported);

        LOG_INFO(CONTEXT_RENDERER, "  - supported texture formats: " << m_availableTextureFormats.size());
        for(const auto& texture : m_availableTextureFormats)
        {
            LOG_DEBUG(CONTEXT_RENDERER, "      " << EnumToString(texture));
        }
    }
}
