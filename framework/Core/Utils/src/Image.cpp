//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/Image.h"
#include "Utils/File.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "lodepng.h"
#include "Utils/LogMacros.h"
#include "Collections/String.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    Image::Image(UInt32 width, UInt32 height, std::vector<UInt8>&& data)
        : m_width(width)
        , m_height(height)
        , m_data(std::move(data))
    {
        assert(m_width * m_height * 4u == m_data.size());
    }

    void Image::loadFromFilePNG(const String& filename)
    {
        if (lodepng::decode(m_data, m_width, m_height, filename.c_str()) != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while loading PNG file: " << filename);
    }

    void Image::saveToFilePNG(const String& filename) const
    {
        if (lodepng::encode(filename.c_str(), m_data, m_width, m_height) != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while saving PNG file: " << filename);
    }

    Bool Image::operator==(const Image& other) const
    {
        if(m_width != other.m_width || m_height != other.m_height)
        {
            return false;
        }

        for (UInt32 y = 0; y < m_height; ++y)
        {
            for (UInt32 x = 0; x < m_width; ++x)
            {
                const UInt8* myPixel = &m_data[4 * (x + m_width*y)];
                const UInt8* otherPixel = &other.m_data[4 * (x + m_width*y)];

                if (myPixel[2] != otherPixel[2]) return false; //b
                if (myPixel[1] != otherPixel[1]) return false; //g
                if (myPixel[0] != otherPixel[0]) return false; //r
            }
        }

        return true;
    }

    Bool Image::operator!=(const Image& other) const
    {
        return !operator==(other);
    }

    Image Image::createDiffTo(const Image& other) const
    {
        assert(m_width == other.m_width && m_height == other.m_height);

        std::vector<UInt8> resultData(m_width * m_height * 4u);
        for (size_t p = 0u; p < m_width * m_height; ++p)
        {
            const auto* p1 = &m_data[p * 4];
            const auto* p2 = &other.m_data[p * 4];
            auto* dst = &resultData[p * 4];

            for (int i = 0; i < 3; ++i)
                dst[i] = (p1[i] >= p2[i] ? p1[i] - p2[i] : p2[i] - p1[i]);
            dst[3] = std::max<UInt8>(p1[3], p2[3]); // keep max alpha
        }

        return Image(m_width, m_height, std::move(resultData));
    }

    Image Image::createEnlarged(UInt32 width, UInt32 height, std::array<UInt8, 4> fillValue) const
    {
        if (width < m_width || height < m_height)
            return {};
        if (width == m_width && height == m_height)
            return *this;

        std::vector<UInt8> enlargedData(width * height * 4u);

        const size_t srcRowSize = m_width * 4u;
        const size_t dstRowSize = width * 4u;

        // copy source row and fill rest of row
        auto srcRowBegin = m_data.cbegin();
        auto dst = enlargedData.begin();
        for (UInt32 row = 0u; row < m_height; ++row)
        {
            // copy source row
            auto srcRowEnd = srcRowBegin + srcRowSize;
            std::copy(srcRowBegin, srcRowEnd, dst);
            srcRowBegin = srcRowEnd;

            // fill rest of row
            const auto dstRowEnd = dst + dstRowSize;
            dst += srcRowSize;
            for (; dst < dstRowEnd; dst += sizeof(fillValue))
                std::copy(fillValue.cbegin(), fillValue.cend(), dst);
        }

        // fill rest of rows
        for (; dst < enlargedData.end(); dst += sizeof(fillValue))
            std::copy(fillValue.cbegin(), fillValue.cend(), dst);

        return Image(width, height, std::move(enlargedData));
    }

    UInt32 Image::getWidth() const
    {
        return m_width;
    }

    UInt32 Image::getHeight() const
    {
        return m_height;
    }

    UInt32 Image::getNumberOfPixels() const
    {
        return m_width * m_height;
    }

    UInt64 Image::getSumOfPixelValues() const
    {
        UInt64 result = 0;

        for (UInt32 y = 0; y < m_height; ++y)
        {
            for (UInt32 x = 0; x < m_width; ++x)
            {
                const UInt8* myPixel = &m_data[4 * (x + m_width*y)];

                result += myPixel[2]; //b
                result += myPixel[1]; //g
                result += myPixel[0]; //r
            }
        }

        return result;
    }

    UInt32 Image::getNumberOfNonBlackPixels(UInt8 maxDiffPerColorChannel) const
    {
        UInt32 result = 0;
        for (UInt32 px = 0; px < m_data.size() / 4; ++px)
        {
            const UInt8* pxData = &m_data[4 * px];
            if (pxData[0] > maxDiffPerColorChannel ||
                pxData[1] > maxDiffPerColorChannel ||
                pxData[2] > maxDiffPerColorChannel)
            {
                result++;
            }
        }

        return result;
    }

    const std::vector<ramses_internal::UInt8>& Image::getData() const
    {
        return m_data;
    }
}
