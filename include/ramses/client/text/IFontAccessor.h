//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/FontInstanceId.h"

namespace ramses
{
    class IFontInstance;

    /**
    * @brief Interface for getting font instances using font instance ids.
    *
    * This interface allows overriding the standard Freetype/Harfbuzz logic
    * to create and access fonts and e.g. replace with other implementations,
    * or just override behavior/settings. Can be also used to pre-load font
    * instances and a predefined set of glyphs statically (or load from file).
    * @ingroup TextAPI
    */
    class RAMSES_API IFontAccessor
    {
    public:
        /**
        * @brief Empty destructor
        */
        virtual ~IFontAccessor() = default;

        /**
        * @brief Gets a font instance object using font instance id
        * @param[in] fontInstanceId The id of font instance
        * @return The font instance object
        */
        [[nodiscard]] virtual IFontInstance* getFontInstance(FontInstanceId fontInstanceId) const = 0;
    };
}
