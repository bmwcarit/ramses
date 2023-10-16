//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Macros.h"
#include "internal/Core/Utils/AssertMovable.h"
#include "impl/DataTypesImpl.h"

#include <cstdint>
#include <vector>
#include <array>
#include <cassert>
#include <string>

namespace ramses::internal
{
    class Image
    {
    public:
        Image() = default;
        template <typename Iter>
        Image(uint32_t width, uint32_t height, Iter begin, Iter end, bool flipVertically = false);
        Image(uint32_t width, uint32_t height, std::vector<uint8_t>&& data);

        Image(const Image& other) = default;
        Image(Image&& other) noexcept = default;

        Image& operator=(const Image& other) = default;
        Image& operator=(Image&& other) noexcept = default;

        void loadFromFilePNG(const std::string& filename);
        void saveToFilePNG(const std::string& filename) const;

        [[nodiscard]] uint32_t getWidth() const;
        [[nodiscard]] uint32_t getHeight() const;
        [[nodiscard]] uint32_t getNumberOfPixels() const;
        [[nodiscard]] glm::ivec4 getSumOfPixelValues() const;
        [[nodiscard]] uint32_t getNumberOfNonBlackPixels(uint8_t maxDiffPerColorChannel = 1) const;

        [[nodiscard]] Image createDiffTo(const Image& other) const;
        [[nodiscard]] std::pair<Image, Image> createSeparateColorAndAlphaImages() const;
        [[nodiscard]] Image createEnlarged(uint32_t width, uint32_t height, std::array<uint8_t, 4> fillValue = {0x0, 0x0, 0x0, 0xff}) const;

        [[nodiscard]] const std::vector<uint8_t>& getData() const;

        bool operator==(const Image& other) const;
        bool operator!=(const Image& other) const;

    private:
        uint32_t m_width = 0u;
        uint32_t m_height = 0u;
        std::vector<uint8_t> m_data;
    };

    ASSERT_MOVABLE(Image)

    template <typename Iter>
    Image::Image(uint32_t width, uint32_t height, Iter begin, Iter end, bool flipVertically /*= false*/)
        : m_width(width)
        , m_height(height)
        , m_data(width * height * 4u)
    {
        assert(m_width * m_height * 4u == static_cast<uint32_t>(std::distance(begin, end)));

        if (flipVertically)
        {
            const size_t rowSize = m_width * 4u;
            Iter srcRowBegin = begin;
            auto dst = m_data.end();
            for (size_t row = 0u; row < m_height; ++row)
            {
                dst -= static_cast<ptrdiff_t>(rowSize);
                const Iter srcRowEnd = srcRowBegin + rowSize;
                std::copy(srcRowBegin, srcRowEnd, dst);
                srcRowBegin = srcRowEnd;
            }
        }
        else
            m_data.assign(begin, end);
    }
}
