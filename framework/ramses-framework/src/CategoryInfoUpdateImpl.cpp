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

    bool CategoryInfoUpdateImpl::hasCategorySizeUpdate() const
    {
        return m_categoryInfo.hasCategorySizeChange();
    }

    Rect CategoryInfoUpdateImpl::getCategorySize() const
    {
        return Rect{m_categoryInfo.getCategoryX(), m_categoryInfo.getCategoryY(), m_categoryInfo.getCategoryWidth(), m_categoryInfo.getCategoryHeight()};
    }

    status_t CategoryInfoUpdateImpl::setCategorySize(Rect rect)
    {
        m_categoryInfo.setCategorySize(rect.x, rect.y, rect.width, rect.height);

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

    bool CategoryInfoUpdateImpl::hasSafeAreaSizeUpdate() const
    {
        return m_categoryInfo.hasSafeAreaSizeChange();
    }

    ramses::Rect CategoryInfoUpdateImpl::getSafeAreaSize() const
    {
        return Rect{m_categoryInfo.getSafeAreaX(), m_categoryInfo.getSafeAreaY() , m_categoryInfo.getSafeAreaWidth() , m_categoryInfo.getSafeAreaHeight() };
    }

    status_t CategoryInfoUpdateImpl::setSafeAreaSize(Rect rect)
    {
        m_categoryInfo.setSafeArea(rect.x, rect.y, rect.width, rect.height);

        return StatusOK;
    }

}
