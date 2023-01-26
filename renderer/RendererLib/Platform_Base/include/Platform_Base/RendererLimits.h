//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERLIMITS_H
#define RAMSES_RENDERERLIMITS_H

#include "SceneAPI/TextureEnums.h"
#include "RendererAPI/Types.h"
#include "Collections/HashSet.h"

namespace ramses_internal
{
    class RendererLimits
    {
    public:
        // Device limits
        uint32_t getMaximumTextureUnits() const;
        void     setMaximumTextureUnits(uint32_t units);

        uint32_t getMaximumSamples() const;
        void     setMaximumSamples(uint32_t samples);

        UInt32   getMaximumAnisotropy() const;
        void     setMaximumAnisotropy(uint32_t anisotropy);

        void setMaxViewport(uint32_t maxWidth, uint32_t maxHeight);
        uint32_t getMaxViewportWidth() const;
        uint32_t getMaxViewportHeight() const;

        UInt32   getMaximumDrawBuffers() const;
        void     setMaximumDrawBuffers(uint32_t drawBuffers);

        // Texture formats
        bool isTextureFormatAvailable(ETextureFormat format) const;
        void addTextureFormat(ETextureFormat format);

        bool isExternalTextureExtensionSupported() const;
        void setExternalTextureExtensionSupported(bool supported);

        void logLimits() const;

    private:
        uint32_t m_maximumTextureUnits = 1u;
        uint32_t m_maximumSamples = 1u;
        uint32_t m_maximumAnisotropy = 1u;
        uint32_t m_maxViewportWidth = 16u;
        uint32_t m_maxViewportHeight = 16u;
        uint32_t m_maximumDrawBuffers = 4u;
        bool m_externalTextureExtensionSupported = false;
        HashSet<ETextureFormat> m_availableTextureFormats;
    };
}

#endif
