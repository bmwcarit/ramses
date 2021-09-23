//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "CategoryInfoUpdateImpl.h"
#include "RamsesFrameworkTypesImpl.h"
#include "APILoggingMacros.h"

namespace ramses
{

    ramses::SizeInfo CategoryInfoUpdate::getRenderSize() const
    {
        return impl.getRenderSize();
    }

    ramses::status_t CategoryInfoUpdate::setRenderSize(SizeInfo sizeInfo)
    {
        const auto status = impl.setRenderSize(sizeInfo);
        LOG_HL_CLIENT_API2(status, sizeInfo.width, sizeInfo.height);
        return status;
    }

    bool CategoryInfoUpdate::hasSafeRectUpdate() const
    {
        return impl.hasSafeRectUpdate();
    }

    ramses::Rect CategoryInfoUpdate::getSafeRect() const
    {
        return impl.getSafeRect();
    }

    ramses::status_t CategoryInfoUpdate::setSafeRect(Rect rect)
    {
        const auto status = impl.setSafeRect(rect);
        LOG_HL_CLIENT_API1(status, rect);
        return status;
    }

    CategoryInfoUpdate::CategoryInfoUpdate(CategoryInfoUpdateImpl& impl_)
        : StatusObject(impl_)
        , impl(impl_)
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    CategoryInfoUpdate::CategoryInfoUpdate()
    : StatusObject(*new CategoryInfoUpdateImpl)
        , impl(static_cast<CategoryInfoUpdateImpl&>(StatusObject::impl))
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    CategoryInfoUpdate::CategoryInfoUpdate(SizeInfo renderSize, Rect categoryRect, Rect safeRect, CategoryInfoUpdate::Layout layout)
        : CategoryInfoUpdate()
    {
        setRenderSize(renderSize);
        setCategoryRect(categoryRect);
        setSafeRect(safeRect);
        setActiveLayout(layout);
    }

    bool CategoryInfoUpdate::operator!=(const CategoryInfoUpdate& rhs) const
    {
        return !(*this == rhs);
    }

    bool CategoryInfoUpdate::operator==(const CategoryInfoUpdate& rhs) const
    {
        return impl.getCategoryInfo() == rhs.impl.getCategoryInfo();
    }

    CategoryInfoUpdate::~CategoryInfoUpdate()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    bool CategoryInfoUpdate::hasCategoryRectUpdate() const
    {
        return impl.hasCategoryRectUpdate();
    }

    Rect CategoryInfoUpdate::getCategoryRect() const
    {
        return impl.getCategoryRect();
    }

    status_t CategoryInfoUpdate::setCategoryRect(Rect rect)
    {
        const auto status = impl.setCategoryRect(rect);
        LOG_HL_CLIENT_API1(status, rect);
        return status;
    }

    bool CategoryInfoUpdate::hasRenderSizeUpdate() const
    {
        return impl.hasRenderSizeUpdate();
    }

    CategoryInfoUpdate::Layout CategoryInfoUpdate::getActiveLayout() const
    {
        return impl.getActiveLayout();
    }

    status_t CategoryInfoUpdate::setActiveLayout(CategoryInfoUpdate::Layout layout)
    {
        const auto status = impl.setActiveLayout(layout);
        LOG_HL_CLIENT_API1(status, layout);
        return status;
    }

    bool CategoryInfoUpdate::hasActiveLayoutUpdate() const
    {
        return impl.hasActiveLayoutUpdate();
    }

}
