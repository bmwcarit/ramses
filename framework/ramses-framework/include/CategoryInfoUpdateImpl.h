//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CATEGORYINFOUPDATEIMPL_H
#define RAMSES_CATEGORYINFOUPDATEIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "StatusObjectImpl.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "Components/CategoryInfo.h"

namespace ramses
{
    class CategoryInfoUpdateImpl : public StatusObjectImpl
    {
    public:
        CategoryInfoUpdateImpl();
        ~CategoryInfoUpdateImpl() = default;

        const ramses_internal::CategoryInfo& getCategoryInfo() const;
        void setCategoryInfo(const ramses_internal::CategoryInfo& other);

        bool hasCategorySizeUpdate() const;
        Rect getCategorySize() const;
        status_t setCategorySize(Rect rect);

        bool hasRenderSizeUpdate() const;
        SizeInfo getRenderSize() const;
        status_t setRenderSize(SizeInfo sizeInfo);

        bool hasSafeAreaSizeUpdate() const;
        Rect getSafeAreaSize() const;
        status_t setSafeAreaSize(Rect rect);
    private:
        ramses_internal::CategoryInfo m_categoryInfo;
    };
}

#endif
