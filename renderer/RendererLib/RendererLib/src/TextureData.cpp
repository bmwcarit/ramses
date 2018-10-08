//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/TextureData.h"

namespace ramses_internal
{

    TextureData::TextureData(ETextureFormat format, UInt32 width, UInt32 height, UInt32 depth, const UInt8* data, Bool takeOwnership)
                : m_format(format)
                , m_width(width)
                , m_height(height)
                , m_depth(depth)
                , m_data(data)
                , m_ownsData(takeOwnership)
    {
    }

    TextureData::~TextureData()
    {
        if (m_ownsData)
        {
            delete[] m_data;
        }
    }

    void TextureData::set(ETextureFormat format, UInt32 width, UInt32 height, UInt32 depth, const UInt8* data, Bool takeOwnership)
    {
        if (m_ownsData)
        {
            if (m_data != data)
            {
                delete[] m_data;
            }
        }

        m_format = format;
        m_width = width;
        m_height = height;
        m_depth = depth;

        m_data = data;
        m_ownsData = takeOwnership;
    }

    ETextureFormat TextureData::getFormat() const
    {
        return m_format;
    }

    UInt32 TextureData::getWidth() const
    {
        return m_width;
    }

    UInt32 TextureData::getHeight() const
    {
        return m_height;
    }

    UInt32 TextureData::getDepth() const
    {
        return m_depth;
    }

    const UInt8* TextureData::getData() const
    {
        return m_data;
    }

    Bool TextureData::ownsData() const
    {
        return m_ownsData;
    }

    TextureData::TextureData(TextureData& other)
    {
        UNUSED(other)
        /// Not implemented.
    }

    void TextureData::operator=(const TextureData& other)
    {
        UNUSED(other)
        /// Not implemented.
    }

    Bool TextureData::convert(ETextureFormat format)
    {
        if (m_format == ETextureFormat_R8 && format == ETextureFormat_RGB8)
        {
            convertR8ToRGB8();
            return true;
        }

        if (m_format == ETextureFormat_RG8 && format == ETextureFormat_RGB8)
        {
            convertRG8ToRGB8();
            return true;
        }

        if (m_format == ETextureFormat_BGR8 && format == ETextureFormat_RGB8)
        {
            convertBGR8ToRGB8();
            return true;
        }

        if (m_format == ETextureFormat_BGRA8 && format == ETextureFormat_RGBA8)
        {
            convertBGRA8ToRGBA8();
            return true;
        }

        return false;
    }

    void TextureData::convertR8ToRGB8()
    {
        UInt32 n = m_width * m_height * m_depth;
        UInt8* data = new UInt8[n * 3];

        for (UInt32 i = 0; i < n; i++)
        {
            data[i * 3 + 0] = m_data[i];
            data[i * 3 + 1] = 0;
            data[i * 3 + 2] = 0;
        }

        set(ETextureFormat_RGB8, m_width, m_height, m_depth, data, true);
    }

    void TextureData::convertRG8ToRGB8()
    {
        UInt32 n = m_width * m_height * m_depth;
        UInt8* data = new UInt8[n * 3];

        for (UInt32 i = 0; i < n; i++)
        {
            data[i * 3 + 0] = m_data[i * 2 + 0];
            data[i * 3 + 1] = m_data[i * 2 + 1];
            data[i * 3 + 2] = 0;
        }

        set(ETextureFormat_RGB8, m_width, m_height, m_depth, data, true);
    }

    void TextureData::convertBGR8ToRGB8()
    {
        UInt32 n = m_width * m_height * m_depth;
        UInt8* data = new UInt8[n * 3];

        for (UInt32 i = 0; i < n; i++)
        {
            data[i * 3 + 0] = m_data[i * 3 + 2];
            data[i * 3 + 1] = m_data[i * 3 + 1];
            data[i * 3 + 2] = m_data[i * 3 + 0];
        }

        set(ETextureFormat_RGB8, m_width, m_height, m_depth, data, true);
    }

    void TextureData::convertBGRA8ToRGBA8()
    {
        UInt32 n = m_width * m_height * m_depth;
        UInt8* data = new UInt8[n * 4];

        for (UInt32 i = 0; i < n; i++)
        {
            data[i * 4 + 0] = m_data[i * 4 + 2];
            data[i * 4 + 1] = m_data[i * 4 + 1];
            data[i * 4 + 2] = m_data[i * 4 + 0];
            data[i * 4 + 3] = m_data[i * 4 + 3];
        }

        set(ETextureFormat_RGBA8, m_width, m_height, m_depth, data, true);
    }
}
