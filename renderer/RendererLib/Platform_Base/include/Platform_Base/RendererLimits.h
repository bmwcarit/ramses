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
        void     setMaximumTextureUnits(uint32_t count);

        uint32_t getMaximumSamples() const;
        void     setMaximumSamples(uint32_t samples);

        UInt32   getMaximumAnisotropy() const;
        void     setMaximumAnisotropy(uint32_t anisotropy);

        // Texture formats
        bool isTextureFormatAvailable(ETextureFormat format) const;
        void addTextureFormat(ETextureFormat format);

        void logLimits() const;

    private:
        uint32_t m_maximumTextureUnits = 1u;
        uint32_t m_maximumSamples = 1u;
        uint32_t m_maximumAnisotropy = 1u;
        HashSet<ETextureFormat> m_availableTextureFormats;
    };
}

#endif
