//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IMAGE_H
#define RAMSES_IMAGE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <vector>
#include <array>
#include <assert.h>

namespace ramses_internal
{
    class String;

    class Image
    {
    public:
        Image() = default;
        template <typename Iter>
        Image(UInt32 width, UInt32 height, Iter begin, Iter end, bool flipVertically = false);
        Image(UInt32 width, UInt32 height, std::vector<UInt8>&& data);

        Image(const Image& other) = default;
        Image(Image&& other) = default;

        Image& operator=(const Image& other) = default;
        Image& operator=(Image&& other) = default;

        void loadFromFilePNG(const String& filename);
        void saveToFilePNG(const String& filename) const;

        UInt32 getWidth() const;
        UInt32 getHeight() const;
        UInt32 getNumberOfPixels() const;
        UInt64 getSumOfPixelValues() const;
        UInt32 getNumberOfNonBlackPixels(UInt8 maxDiffPerColorChannel = 1) const;

        Image createDiffTo(const Image& other) const;
        Image createEnlarged(UInt32 width, UInt32 height, std::array<UInt8, 4> fillValue = {0x0, 0x0, 0x0, 0xff}) const;

        const std::vector<UInt8>& getData() const;

        Bool operator==(const Image& other) const;
        Bool operator!=(const Image& other) const;

    private:
        UInt32 m_width = 0u;
        UInt32 m_height = 0u;
        std::vector<UInt8> m_data;
    };

    template <typename Iter>
    Image::Image(UInt32 width, UInt32 height, Iter begin, Iter end, bool flipVertically /*= false*/)
        : m_width(width)
        , m_height(height)
        , m_data(width * height * 4u)
    {
        assert(m_width * m_height * 4u == static_cast<UInt32>(std::distance(begin, end)));

        if (flipVertically)
        {
            const size_t rowSize = m_width * 4u;
            Iter srcRowBegin = begin;
            auto dst = m_data.end();
            for (size_t row = 0u; row < m_height; ++row)
            {
                dst -= rowSize;
                const Iter srcRowEnd = srcRowBegin + rowSize;
                std::copy(srcRowBegin, srcRowEnd, dst);
                srcRowBegin = srcRowEnd;
            }
        }
        else
            m_data.assign(begin, end);
    }
}

#endif
