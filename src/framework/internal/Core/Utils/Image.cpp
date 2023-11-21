//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/Image.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "lodepng.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include <numeric>

namespace ramses::internal
{
    Image::Image(uint32_t width, uint32_t height, std::vector<uint8_t>&& data)
        : m_width(width)
        , m_height(height)
        , m_data(std::move(data))
    {
        assert(m_width * m_height * 4u == m_data.size());
    }

    void Image::loadFromFilePNG(const std::string& filename)
    {
        const unsigned int ret = lodepng::decode(m_data, m_width, m_height, filename);
        if (ret != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while loading PNG file: {} (error {}: {})", filename, ret, lodepng_error_text(ret));
    }

    void Image::saveToFilePNG(const std::string& filename) const
    {
        const unsigned int ret = lodepng::encode(filename, m_data, m_width, m_height);
        if (ret != 0)
            LOG_ERROR(CONTEXT_FRAMEWORK, "Error while saving PNG file: {} (error {}: {})", filename, ret, lodepng_error_text(ret));
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

        std::vector<uint8_t> resultData(m_width * m_height * 4u);
        std::transform(m_data.cbegin(), m_data.cend(), other.m_data.cbegin(), resultData.begin(), [](uint8_t c1, uint8_t c2)
        {
            // cast should not be needed but MSVC2017 deduces 'int' here resulting in compilation warning when assigning back to uint8
            return static_cast<uint8_t>(c1 >= c2 ? c1 - c2 : c2 - c1);
        });

        return Image(m_width, m_height, std::move(resultData));
    }

    std::pair<Image, Image> Image::createSeparateColorAndAlphaImages() const
    {
        std::vector<uint8_t> resultRGBData;
        std::vector<uint8_t> resultAlphaData;
        resultRGBData.reserve(m_width * m_height * 4u);
        resultAlphaData.reserve(m_width * m_height * 4u);

        for (size_t px = 0; px < m_data.size() / 4; ++px)
        {
            const uint8_t* pxData = &m_data[4 * px];
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

    Image Image::createEnlarged(uint32_t width, uint32_t height, std::array<uint8_t, 4> fillValue) const
    {
        if (width < m_width || height < m_height)
            return {};
        if (width == m_width && height == m_height)
            return *this;

        std::vector<uint8_t> enlargedData(width * height * 4u);

        const auto srcRowSize = static_cast<ptrdiff_t>(m_width) * 4;
        const auto dstRowSize = static_cast<ptrdiff_t>(width) * 4;

        // copy source row and fill rest of row
        auto srcRowBegin = m_data.cbegin();
        auto dst = enlargedData.begin();
        for (uint32_t row = 0u; row < m_height; ++row)
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

    uint32_t Image::getWidth() const
    {
        return m_width;
    }

    uint32_t Image::getHeight() const
    {
        return m_height;
    }

    uint32_t Image::getNumberOfPixels() const
    {
        return m_width * m_height;
    }

    glm::ivec4 Image::getSumOfPixelValues() const
    {
        auto addWithoutOverflow = [](int32_t& dest, uint8_t value) {
            constexpr int32_t maximumValueBeforeOverflow = std::numeric_limits<int32_t>::max() - std::numeric_limits<uint8_t>::max() - 1;

            const bool overflow = (dest >= maximumValueBeforeOverflow);
            if (overflow)
            {
                dest = std::numeric_limits<int32_t>::max();
            }
            else
            {
                dest += value;
            }

            return overflow;
        };

        bool overflow = false;
        glm::ivec4 result{0};
        for (size_t px = 0; px < m_data.size() / 4; ++px)
        {
            const uint8_t* pxData = &m_data[4 * px];
            overflow |= addWithoutOverflow(result.x, pxData[0]);
            overflow |= addWithoutOverflow(result.y, pxData[1]);
            overflow |= addWithoutOverflow(result.z, pxData[2]);
            overflow |= addWithoutOverflow(result.w, pxData[3]);
        }

        if (overflow)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "Image::getSumOfPixelValues: Overflow of sum of pixel values! The overflown values saturate to max possible positive value for int32_t [={}]",
                std::numeric_limits<int32_t>::max());
        }

        return result;
    }

    uint32_t Image::getNumberOfNonBlackPixels(uint8_t maxDiffPerColorChannel) const
    {
        uint32_t result = 0;
        for (size_t px = 0; px < m_data.size() / 4; ++px)
        {
            const uint8_t* pxData = &m_data[4 * px];
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

    const std::vector<uint8_t>& Image::getData() const
    {
        return m_data;
    }

    std::vector<std::byte> Image::getDataAsByte() const
    {
        std::vector<std::byte> data(m_data.size());
        std::transform(m_data.begin(), m_data.end(), data.begin(), [](uint8_t x) { return static_cast<std::byte>(x); });
        return data;
    }
}
