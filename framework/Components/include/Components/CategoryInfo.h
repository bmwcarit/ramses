//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENT_CATEGORYINFO_H
#define RAMSES_COMPONENT_CATEGORYINFO_H

#include "Utils/LoggingUtils.h"
#include "Utils/StringOutputSpecialWrapper.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "absl/types/span.h"

namespace ramses_internal
{
    class CategoryInfo
    {
    public:
        CategoryInfo();
        CategoryInfo(uint32_t width, uint32_t height);
        explicit CategoryInfo(absl::Span<const Byte> data);

        void setCategorySize(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        uint32_t getCategoryX() const;
        uint32_t getCategoryY() const;
        uint32_t getCategoryWidth() const;
        uint32_t getCategoryHeight() const;
        bool hasCategorySizeChange() const;

        void setRenderSize(uint32_t width, uint32_t height);
        uint32_t getRenderSizeWidth() const;
        uint32_t getRenderSizeHeight() const;
        bool hasRenderSizeChange() const;

        void setSafeArea(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        uint32_t getSafeAreaX() const;
        uint32_t getSafeAreaY() const;
        uint32_t getSafeAreaWidth() const;
        uint32_t getSafeAreaHeight() const;
        bool hasSafeAreaSizeChange() const;

        std::vector<Byte> toBinary() const;

        bool operator==(const CategoryInfo& rhs) const;
        bool operator!=(const CategoryInfo& rhs) const;
    private:
        void fromBinary(absl::Span<const Byte> data);

        uint32_t m_categorySizeX;
        uint32_t m_categorySizeY;
        uint32_t m_categorySizeWidth;
        uint32_t m_categorySizeHeight;
        uint32_t m_renderSizeWidth;
        uint32_t m_renderSizeHeight;
        uint32_t m_safeAreaX;
        uint32_t m_safeAreaY;
        uint32_t m_safeAreaWidth;
        uint32_t m_safeAreaHeight;
        bool m_categorySizeChanged;
        bool m_renderSizeChanged;
        bool m_safeAreaChanged;
    };
}

template <>
struct fmt::formatter<ramses_internal::CategoryInfo> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext>
    auto format(const ramses_internal::CategoryInfo& categoryInfo, FormatContext& ctx)
    {
        fmt::format_to(ctx.out(), "[categorySize:");
        if (categoryInfo.hasCategorySizeChange())
            fmt::format_to(ctx.out(), "{}x{}", categoryInfo.getCategoryWidth(), categoryInfo.getCategoryHeight());
        return fmt::format_to(ctx.out(), "]");
    }
};

#endif
