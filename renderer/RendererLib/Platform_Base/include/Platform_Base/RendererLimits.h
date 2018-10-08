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
        RendererLimits();

        // Device limits
        UInt32  getMaximumTextureUnits() const;
        void    setMaximumTextureUnits(UInt32 count);

        UInt32  getMaximumAnisotropy() const;
        void    setMaximumAnisotropy(UInt32 anisotropy);

        // Texture formats
        Bool    isTextureFormatAvailable(ETextureFormat format) const;
        void    addTextureFormat(ETextureFormat format);

        void    logLimits() const;

    private:
        UInt32 m_maximumTextureUnits;
        UInt32 m_maximumAnisotropy;
        HashSet<ETextureFormat> m_availableTextureFormats;
    };
}

#endif
