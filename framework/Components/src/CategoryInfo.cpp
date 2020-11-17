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
            CategoryRect = 1,
            RenderSize = 2,
            SafeRect = 3,
        };

        constexpr const uint32_t CurrentBinaryCategoryInfoVersion = 1;
    }

    CategoryInfo::CategoryInfo()
        : m_categoryRectX(0)
        , m_categoryRectY(0)
        , m_categoryRectWidth(0)
        , m_categoryRectHeight(0)
        , m_renderSizeWidth(0)
        , m_renderSizeHeight(0)
        , m_safeRectX(0)
        , m_safeRectY(0)
        , m_safeRectWidth(0)
        , m_safeRectHeight(0)
        , m_categoryRectChanged(false)
        , m_renderSizeChanged(false)
        , m_safeRectChanged(false)
    {
    }

    CategoryInfo::CategoryInfo(uint32_t width, uint32_t height)
        : CategoryInfo()
    {
        setCategoryRect(0, 0, width, height);
    }

    CategoryInfo::CategoryInfo(absl::Span<const Byte> data)
        : CategoryInfo()
    {
        fromBinary(data);
    }

    void CategoryInfo::setCategoryRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        m_categoryRectX = x;
        m_categoryRectY = y;
        m_categoryRectWidth = width;
        m_categoryRectHeight = height;
        m_categoryRectChanged = true;
    }

    uint32_t CategoryInfo::getCategoryX() const
    {
        return m_categoryRectX;
    }

    uint32_t CategoryInfo::getCategoryY() const
    {
        return m_categoryRectY;
    }

    uint32_t CategoryInfo::getCategoryWidth() const
    {
        return m_categoryRectWidth;
    }

    uint32_t CategoryInfo::getCategoryHeight() const
    {
        return m_categoryRectHeight;
    }

    bool CategoryInfo::hasCategoryRectChange() const
    {
        return m_categoryRectChanged;
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

    void CategoryInfo::setSafeRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        m_safeRectChanged = true;
        m_safeRectX = x;
        m_safeRectY = y;
        m_safeRectWidth = width;
        m_safeRectHeight = height;
    }

    uint32_t CategoryInfo::getSafeRectX() const
    {
        return m_safeRectX;
    }

    uint32_t CategoryInfo::getSafeRectY() const
    {
        return m_safeRectY;
    }

    uint32_t CategoryInfo::getSafeRectWidth() const
    {
        return m_safeRectWidth;
    }

    uint32_t CategoryInfo::getSafeRectHeight() const
    {
        return m_safeRectHeight;
    }

    bool CategoryInfo::hasSafeRectChange() const
    {
        return m_safeRectChanged;
    }

    std::vector<Byte> CategoryInfo::toBinary() const
    {
        BinaryOutputStream os;
        const uint32_t numEntries =
            (m_categoryRectChanged ? 1 : 0)
            + (m_renderSizeChanged ? 1 : 0)
            + (m_safeRectChanged ? 1 : 0);
        os << CurrentBinaryCategoryInfoVersion
            << numEntries;

        // write entries
        if (m_categoryRectChanged)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(uint32_t) * 4);
            os << CategoryInfoUpdateType::CategoryRect
                << size
                << m_categoryRectX
                << m_categoryRectY
                << m_categoryRectWidth
                << m_categoryRectHeight;
        }
        if (m_renderSizeChanged)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(uint32_t) * 2);
            os << CategoryInfoUpdateType::RenderSize
                << size
                << m_renderSizeWidth
                << m_renderSizeHeight;
        }
        if (m_safeRectChanged)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(uint32_t) * 4);
            os << CategoryInfoUpdateType::SafeRect
                << size
                << m_safeRectX
                << m_safeRectY
                << m_safeRectWidth
                << m_safeRectHeight;
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
            case CategoryInfoUpdateType::CategoryRect:
                m_categoryRectChanged = true;
                is >> m_categoryRectX;
                is >> m_categoryRectY;
                is >> m_categoryRectWidth;
                is >> m_categoryRectHeight;
                break;
            case CategoryInfoUpdateType::RenderSize:
                m_renderSizeChanged = true;
                is >> m_renderSizeWidth;
                is >> m_renderSizeHeight;
                break;
            case CategoryInfoUpdateType::SafeRect:
                m_safeRectChanged = true;
                is >> m_safeRectX;
                is >> m_safeRectY;
                is >> m_safeRectWidth;
                is >> m_safeRectHeight;
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
        return m_categoryRectX == rhs.m_categoryRectX
            && m_categoryRectY == rhs.m_categoryRectY
            && m_categoryRectWidth == rhs.m_categoryRectWidth
            && m_categoryRectHeight == rhs.m_categoryRectHeight
            && m_categoryRectChanged == rhs.m_categoryRectChanged
            && m_renderSizeChanged == rhs.m_renderSizeChanged
            && m_renderSizeWidth == rhs.m_renderSizeWidth
            && m_renderSizeHeight == rhs.m_renderSizeHeight
            && m_safeRectX == rhs.m_safeRectX
            && m_safeRectY == rhs.m_safeRectY
            && m_safeRectWidth == rhs.m_safeRectWidth
            && m_safeRectHeight == rhs.m_safeRectHeight
            && m_safeRectChanged == rhs.m_safeRectChanged;
    }

    bool CategoryInfo::operator!=(const CategoryInfo& rhs) const
    {
        return !(*this == rhs);
    }

}
