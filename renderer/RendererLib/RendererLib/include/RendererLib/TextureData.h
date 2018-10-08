//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREDATA_H
#define RAMSES_TEXTUREDATA_H

#include "RendererAPI/Types.h"
#include "SceneAPI/TextureEnums.h"

namespace ramses_internal
{
    class TextureData
    {
    public:
        TextureData(ETextureFormat format, UInt32 width, UInt32 height, UInt32 depth, const UInt8* data, Bool takeOwnership = false);

        /// Deletes the data, when taking the ownership.
        ~TextureData();

        /// Sets new data, and deletes the old one, when ownership was set.
        void set(ETextureFormat format, UInt32 width, UInt32 height, UInt32 depth, const UInt8* data, Bool takeOwnership);

        ETextureFormat getFormat() const;
        UInt32 getWidth() const;
        UInt32 getHeight() const;
        UInt32 getDepth() const;
        const UInt8* getData() const;
        Bool ownsData() const;

        /// Converts the contained data to another format.
        /** Currently the only supported formats are:
         *  ETextureFormat_R8 => ETextureFormat_RGB8
         *  ETextureFormat_RG8 => ETextureFormat_RGB8
         *  ETextureFormat_BGR8 => ETextureFormat_RGB8
         *  ETextureFormat_BGRA8 => ETextureFormat_RGBA8
         *
         *  @param format The format to convert to.
         *  @return "true", when converting was successful. */
        Bool convert(ETextureFormat format);

    private:

        TextureData(TextureData& other);

        void operator=(const TextureData& other);

        void convertR8ToRGB8();
        void convertRG8ToRGB8();
        void convertBGR8ToRGB8();
        void convertBGRA8ToRGBA8();

        ETextureFormat m_format;
        UInt32 m_width;
        UInt32 m_height;
        UInt32 m_depth;
        const UInt8* m_data;
        Bool m_ownsData;
    };
}

#endif
