//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONTENTCONTROLCONFIGIMPL_H
#define RAMSES_DCSMCONTENTCONTROLCONFIGIMPL_H

#include "ramses-renderer-api/DcsmContentControlConfig.h"
#include "StatusObjectImpl.h"
#include <unordered_map>

namespace ramses
{
    class DcsmContentControlConfigImpl : public StatusObjectImpl
    {
    public:
        DcsmContentControlConfigImpl(std::initializer_list<std::pair<Category, DcsmContentControlConfig::CategoryInfo>> categories);

        status_t addCategory(Category categoryId, const DcsmContentControlConfig::CategoryInfo& categoryInfo);
        const DcsmContentControlConfig::CategoryInfo* findCategoryInfo(Category categoryId) const;

        const std::unordered_map<Category, DcsmContentControlConfig::CategoryInfo>& getCategories() const;

    private:
        std::unordered_map<Category, DcsmContentControlConfig::CategoryInfo> m_categories;
    };
}

#endif
