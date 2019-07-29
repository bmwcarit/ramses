//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMRENDERERCONFIGIMPL_H
#define RAMSES_DCSMRENDERERCONFIGIMPL_H

#include "ramses-renderer-api/DcsmRendererConfig.h"
#include "StatusObjectImpl.h"
#include <unordered_map>

namespace ramses
{
    class DcsmRendererConfigImpl : public StatusObjectImpl
    {
    public:
        DcsmRendererConfigImpl(std::initializer_list<std::pair<Category, DcsmRendererConfig::CategoryInfo>> categories);

        status_t addCategory(Category categoryId, const DcsmRendererConfig::CategoryInfo& categoryInfo);
        const DcsmRendererConfig::CategoryInfo* findCategoryInfo(Category categoryId) const;

        const std::unordered_map<Category, DcsmRendererConfig::CategoryInfo>& getCategories() const;

    private:
        std::unordered_map<Category, DcsmRendererConfig::CategoryInfo> m_categories;
    };
}

#endif
