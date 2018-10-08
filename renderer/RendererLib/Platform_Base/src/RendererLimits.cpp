//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/RendererLimits.h"
#include "Utils/LogMacros.h"
#include "Common/Cpp11Macros.h"

namespace ramses_internal
{
    RendererLimits::RendererLimits()
        : m_maximumTextureUnits(1u)
        , m_maximumAnisotropy(1u)
    {
    }

    UInt32 RendererLimits::getMaximumTextureUnits() const
    {
        return m_maximumTextureUnits;
    }

    void RendererLimits::setMaximumTextureUnits(UInt32 units)
    {
        m_maximumTextureUnits = units;
    }

    UInt32 RendererLimits::getMaximumAnisotropy() const
    {
        return m_maximumAnisotropy;
    }

    void RendererLimits::setMaximumAnisotropy(UInt32 anisotropy)
    {
        m_maximumAnisotropy = anisotropy;
    }

    Bool RendererLimits::isTextureFormatAvailable(ETextureFormat format) const
    {
        return m_availableTextureFormats.hasElement(format);
    }

    void RendererLimits::addTextureFormat(ETextureFormat format)
    {
        m_availableTextureFormats.put(format);
    }

    void RendererLimits::logLimits() const
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Device features and limits:");
        LOG_DEBUG(CONTEXT_RENDERER, "  - maximum number of texture units:            " << m_maximumTextureUnits);
        LOG_DEBUG(CONTEXT_RENDERER, "  - maximum number of anisotropy samples:       " << m_maximumAnisotropy);

        LOG_DEBUG(CONTEXT_RENDERER, "  - supported texture formats:");
        ramses_foreach(m_availableTextureFormats, texIt)
        {
            LOG_DEBUG(CONTEXT_RENDERER, "      " << EnumToString(*texIt));
        }
    }
}
