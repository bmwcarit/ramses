//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/CategoryInfo.h"
#include "Utils/LogMacros.h"
#include <cassert>
#include "Utils/BinaryInputStream.h"
#include "Utils/BinaryOutputStream.h"

namespace ramses_internal
{
    namespace
    {
        enum class CategoryInfoUpdateType : uint32_t
        {
            CategorySize = 1,
            RenderSize = 2,
            SafeArea = 3,
        };

        constexpr const uint32_t CurrentBinaryCategoryInfoVersion = 1;
    }

    CategoryInfo::CategoryInfo()
        : m_categorySizeX(0)
        , m_categorySizeY(0)
        , m_categorySizeWidth(0)
        , m_categorySizeHeight(0)
        , m_renderSizeWidth(0)
        , m_renderSizeHeight(0)
        , m_safeAreaX(0)
        , m_safeAreaY(0)
        , m_safeAreaWidth(0)
        , m_safeAreaHeight(0)
        , m_categorySizeChanged(false)
        , m_renderSizeChanged(false)
        , m_safeAreaChanged(false)
    {
    }

    CategoryInfo::CategoryInfo(uint32_t width, uint32_t height)
        : CategoryInfo()
    {
        setCategorySize(0, 0, width, height);
    }

    CategoryInfo::CategoryInfo(absl::Span<const Byte> data)
        : CategoryInfo()
    {
        fromBinary(data);
    }

    void CategoryInfo::setCategorySize(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        m_categorySizeX = x;
        m_categorySizeY = y;
        m_categorySizeWidth = width;
        m_categorySizeHeight = height;
        m_categorySizeChanged = true;
    }

    uint32_t CategoryInfo::getCategoryX() const
    {
        return m_categorySizeX;
    }

    uint32_t CategoryInfo::getCategoryY() const
    {
        return m_categorySizeY;
    }

    uint32_t CategoryInfo::getCategoryWidth() const
    {
        return m_categorySizeWidth;
    }

    uint32_t CategoryInfo::getCategoryHeight() const
    {
        return m_categorySizeHeight;
    }

    bool CategoryInfo::hasCategorySizeChange() const
    {
        return m_categorySizeChanged;
    }

    void CategoryInfo::setRenderSize(uint32_t width, uint32_t height)
    {
        m_renderSizeChanged = true;
        m_renderSizeWidth = width;
        m_renderSizeHeight = height;
    }

    uint32_t CategoryInfo::getRenderSizeWidth() const
    {
        return m_renderSizeWidth;
    }

    uint32_t CategoryInfo::getRenderSizeHeight() const
    {
        return m_renderSizeHeight;
    }

    bool CategoryInfo::hasRenderSizeChange() const
    {
        return m_renderSizeChanged;
    }

    void CategoryInfo::setSafeArea(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        m_safeAreaChanged = true;
        m_safeAreaX = x;
        m_safeAreaY = y;
        m_safeAreaWidth = width;
        m_safeAreaHeight = height;
    }

    uint32_t CategoryInfo::getSafeAreaX() const
    {
        return m_safeAreaX;
    }

    uint32_t CategoryInfo::getSafeAreaY() const
    {
        return m_safeAreaY;
    }

    uint32_t CategoryInfo::getSafeAreaWidth() const
    {
        return m_safeAreaWidth;
    }

    uint32_t CategoryInfo::getSafeAreaHeight() const
    {
        return m_safeAreaHeight;
    }

    bool CategoryInfo::hasSafeAreaSizeChange() const
    {
        return m_safeAreaChanged;
    }

    std::vector<Byte> CategoryInfo::toBinary() const
    {
        BinaryOutputStream os;
        const uint32_t numEntries =
            (m_categorySizeChanged ? 1 : 0)
            + (m_renderSizeChanged ? 1 : 0)
            + (m_safeAreaChanged ? 1 : 0);
        os << CurrentBinaryCategoryInfoVersion
            << numEntries;

        // write entries
        if (m_categorySizeChanged)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(uint32_t) * 4);
            os << CategoryInfoUpdateType::CategorySize
                << size
                << m_categorySizeX
                << m_categorySizeY
                << m_categorySizeWidth
                << m_categorySizeHeight;
        }
        if (m_renderSizeChanged)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(uint32_t) * 2);
            os << CategoryInfoUpdateType::RenderSize
                << size
                << m_renderSizeWidth
                << m_renderSizeHeight;
        }
        if (m_safeAreaChanged)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(uint32_t) * 4);
            os << CategoryInfoUpdateType::SafeArea
                << size
                << m_safeAreaX
                << m_safeAreaY
                << m_safeAreaWidth
                << m_safeAreaHeight;
        }
        return os.release();
    }

    void CategoryInfo::fromBinary(absl::Span<const Byte> data)
    {
        LOG_DEBUG(CONTEXT_DCSM, "CategoryInfo::fromBinary: length " << data.size());
        BinaryInputStream is(data.data());

        uint32_t readCategoryInfoDataVersion = 0;
        is >> readCategoryInfoDataVersion;
        if (readCategoryInfoDataVersion != CurrentBinaryCategoryInfoVersion)
        {
            LOG_WARN(CONTEXT_DCSM, "CategoryInfo::fromBinary: ignore because unexpected category info data version " << readCategoryInfoDataVersion << ", expected " << CurrentBinaryCategoryInfoVersion);
            return;
        }

        uint32_t numEntries = 0;
        is >> numEntries;

        for (uint32_t i = 0; i < numEntries; ++i)
        {
            uint32_t size = 0;
            CategoryInfoUpdateType type;
            is >> type
                >> size;
            switch (type)
            {
            case CategoryInfoUpdateType::CategorySize:
                m_categorySizeChanged = true;
                is >> m_categorySizeX;
                is >> m_categorySizeY;
                is >> m_categorySizeWidth;
                is >> m_categorySizeHeight;
                break;
            case CategoryInfoUpdateType::RenderSize:
                m_renderSizeChanged = true;
                is >> m_renderSizeWidth;
                is >> m_renderSizeHeight;
                break;
            case CategoryInfoUpdateType::SafeArea:
                m_safeAreaChanged = true;
                is >> m_safeAreaX;
                is >> m_safeAreaY;
                is >> m_safeAreaWidth;
                is >> m_safeAreaHeight;
                break;
            default:
                LOG_WARN(CONTEXT_DCSM, "CategoryInfo::fromBinary: skip unknown type " << static_cast<uint32_t>(type) << ", size " << size);
                is.skip(size);
            }
        }

        assert(is.readPosition() == &*data.end());
    }

    bool CategoryInfo::operator==(const CategoryInfo& rhs) const
    {
        return m_categorySizeX == rhs.m_categorySizeX
            && m_categorySizeY == rhs.m_categorySizeY
            && m_categorySizeWidth == rhs.m_categorySizeWidth
            && m_categorySizeHeight == rhs.m_categorySizeHeight
            && m_categorySizeChanged == rhs.m_categorySizeChanged
            && m_renderSizeChanged == rhs.m_renderSizeChanged
            && m_renderSizeWidth == rhs.m_renderSizeWidth
            && m_renderSizeHeight == rhs.m_renderSizeHeight
            && m_safeAreaX == rhs.m_safeAreaX
            && m_safeAreaY == rhs.m_safeAreaY
            && m_safeAreaWidth == rhs.m_safeAreaWidth
            && m_safeAreaHeight == rhs.m_safeAreaHeight
            && m_safeAreaChanged == rhs.m_safeAreaChanged;
    }

    bool CategoryInfo::operator!=(const CategoryInfo& rhs) const
    {
        return !(*this == rhs);
    }

}
