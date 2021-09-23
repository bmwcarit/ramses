//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "CategoryInfoUpdateImpl.h"

namespace ramses
{
    CategoryInfoUpdateImpl::CategoryInfoUpdateImpl()
        : m_categoryInfo{}
    {
    };

    bool CategoryInfoUpdateImpl::hasCategoryRectUpdate() const
    {
        return m_categoryInfo.hasCategoryRectChange();
    }

    Rect CategoryInfoUpdateImpl::getCategoryRect() const
    {
        return Rect{m_categoryInfo.getCategoryX(), m_categoryInfo.getCategoryY(), m_categoryInfo.getCategoryWidth(), m_categoryInfo.getCategoryHeight()};
    }

    status_t CategoryInfoUpdateImpl::setCategoryRect(Rect rect)
    {
        m_categoryInfo.setCategoryRect(rect.x, rect.y, rect.width, rect.height);

        return StatusOK;
    }

    bool CategoryInfoUpdateImpl::hasRenderSizeUpdate() const
    {
        return m_categoryInfo.hasRenderSizeChange();
    }

    const ramses_internal::CategoryInfo& CategoryInfoUpdateImpl::getCategoryInfo() const
    {
        return m_categoryInfo;
    }

    void CategoryInfoUpdateImpl::setCategoryInfo(const ramses_internal::CategoryInfo& other)
    {
        m_categoryInfo = other;
    }

    ramses::SizeInfo CategoryInfoUpdateImpl::getRenderSize() const
    {
        return SizeInfo{m_categoryInfo.getRenderSizeWidth(), m_categoryInfo.getRenderSizeHeight()};
    }

    status_t CategoryInfoUpdateImpl::setRenderSize(SizeInfo sizeInfo)
    {
        m_categoryInfo.setRenderSize(sizeInfo.width, sizeInfo.height);

        return StatusOK;
    }

    bool CategoryInfoUpdateImpl::hasSafeRectUpdate() const
    {
        return m_categoryInfo.hasSafeRectChange();
    }

    ramses::Rect CategoryInfoUpdateImpl::getSafeRect() const
    {
        return Rect{m_categoryInfo.getSafeRectX(), m_categoryInfo.getSafeRectY() , m_categoryInfo.getSafeRectWidth() , m_categoryInfo.getSafeRectHeight() };
    }

    status_t CategoryInfoUpdateImpl::setSafeRect(Rect rect)
    {
        m_categoryInfo.setSafeRect(rect.x, rect.y, rect.width, rect.height);

        return StatusOK;
    }

    bool CategoryInfoUpdateImpl::hasActiveLayoutUpdate() const
    {
        return m_categoryInfo.hasActiveLayoutChange();
    }

    CategoryInfoUpdate::Layout CategoryInfoUpdateImpl::getActiveLayout() const
    {
        return static_cast<CategoryInfoUpdate::Layout>(m_categoryInfo.getActiveLayout());
    }

    status_t CategoryInfoUpdateImpl::setActiveLayout(CategoryInfoUpdate::Layout layout)
    {
        m_categoryInfo.setActiveLayout(static_cast<ramses_internal::CategoryInfo::Layout>(layout));
        return StatusOK;
    }
}
