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

    bool CategoryInfoUpdate::hasSafeAreaSizeUpdate() const
    {
        return impl.hasSafeAreaSizeUpdate();
    }

    ramses::Rect CategoryInfoUpdate::getSafeAreaSize() const
    {
        return impl.getSafeAreaSize();
    }

    ramses::status_t CategoryInfoUpdate::setSafeAreaSize(Rect rect)
    {
        const auto status = impl.setSafeAreaSize(rect);
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

    CategoryInfoUpdate::CategoryInfoUpdate(SizeInfo categorySize)
        : CategoryInfoUpdate()
    {
        setCategorySize(Rect(0, 0, categorySize.width, categorySize.height));
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

    bool CategoryInfoUpdate::hasCategorySizeUpdate() const
    {
        return impl.hasCategorySizeUpdate();
    }

    Rect CategoryInfoUpdate::getCategorySize() const
    {
        return impl.getCategorySize();
    }

    status_t CategoryInfoUpdate::setCategorySize(Rect rect)
    {
        const auto status = impl.setCategorySize(rect);
        LOG_HL_CLIENT_API1(status, rect);
        return status;
    }

    bool CategoryInfoUpdate::hasRenderSizeUpdate() const
    {
        return impl.hasRenderSizeUpdate();
    }

}
