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
#include <numeric>

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
        const unsigned int ret = lodepng::decode(m_data, m_width, m_height, filename.c_str());
        if (ret != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while loading PNG file: " << filename << " (error " << ret << ": " << lodepng_error_text(ret) << ")");
    }

    void Image::saveToFilePNG(const String& filename) const
    {
        const unsigned int ret = lodepng::encode(filename.c_str(), m_data, m_width, m_height);
        if (ret != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while saving PNG file: " << filename << " (error " << ret << ": " << lodepng_error_text(ret) << ")");
    }

    bool Image::operator==(const Image& other) const
    {
        return m_width == other.m_width
            && m_height == other.m_height
            && m_data == other.m_data;
    }

    bool Image::operator!=(const Image& other) const
    {
        return !operator==(other);
    }

    Image Image::createDiffTo(const Image& other) const
    {
        assert(m_width == other.m_width && m_height == other.m_height);

        std::vector<UInt8> resultData(m_width * m_height * 4u);
        std::transform(m_data.cbegin(), m_data.cend(), other.m_data.cbegin(), resultData.begin(), [](UInt8 c1, UInt8 c2)
        {
            // cast should not be needed but MSVC2017 deduces 'int' here resulting in compilation warning when assigning back to uint8
            return static_cast<UInt8>(c1 >= c2 ? c1 - c2 : c2 - c1);
        });

        return Image(m_width, m_height, std::move(resultData));
    }

    std::pair<Image, Image> Image::createSeparateColorAndAlphaImages() const
    {
        std::vector<UInt8> resultRGBData;
        std::vector<UInt8> resultAlphaData;
        resultRGBData.reserve(m_width * m_height * 4u);
        resultAlphaData.reserve(m_width * m_height * 4u);

        for (size_t px = 0; px < m_data.size() / 4; ++px)
        {
            const UInt8* pxData = &m_data[4 * px];
            resultRGBData.push_back(pxData[0]);
            resultRGBData.push_back(pxData[1]);
            resultRGBData.push_back(pxData[2]);
            resultRGBData.push_back(255u);

            resultAlphaData.push_back(pxData[3]);
            resultAlphaData.push_back(pxData[3]);
            resultAlphaData.push_back(pxData[3]);
            resultAlphaData.push_back(255u);
        }

        Image rgbImage(m_width, m_height, std::move(resultRGBData));
        Image alphaImage(m_width, m_height, std::move(resultAlphaData));

        return { std::move(rgbImage), std::move(alphaImage) };
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

    Vector4i Image::getSumOfPixelValues() const
    {
        auto addWithoutOverflow = [](int32_t& dest, uint8_t value) {
            constexpr int32_t maximumValueBeforeOverflow = std::numeric_limits<int32_t>::max() - std::numeric_limits<uint8_t>::max() - 1;

            const bool overflow = (dest >= maximumValueBeforeOverflow);
            if (overflow)
                dest = std::numeric_limits<int32_t>::max();
            else
                dest += value;

            return overflow;
        };

        bool overflow = false;
        Vector4i result{0};
        for (size_t px = 0; px < m_data.size() / 4; ++px)
        {
            const UInt8* pxData = &m_data[4 * px];
            overflow |= addWithoutOverflow(result.x, pxData[0]);
            overflow |= addWithoutOverflow(result.y, pxData[1]);
            overflow |= addWithoutOverflow(result.z, pxData[2]);
            overflow |= addWithoutOverflow(result.w, pxData[3]);
        }

        if (overflow)
            LOG_WARN(CONTEXT_FRAMEWORK, "Image::getSumOfPixelValues: Overflow of sum of pixel values! The overflown values saturate to max possible positive value for int32_t [="
            << std::numeric_limits<int32_t>::max() << "]");

        return result;
    }

    UInt32 Image::getNumberOfNonBlackPixels(UInt8 maxDiffPerColorChannel) const
    {
        UInt32 result = 0;
        for (size_t px = 0; px < m_data.size() / 4; ++px)
        {
            const UInt8* pxData = &m_data[4 * px];
            if (pxData[0] > maxDiffPerColorChannel ||
                pxData[1] > maxDiffPerColorChannel ||
                pxData[2] > maxDiffPerColorChannel ||
                pxData[3] > maxDiffPerColorChannel)
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
