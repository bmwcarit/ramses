//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmRendererConfigImpl.h"

namespace ramses
{
    DcsmRendererConfigImpl::DcsmRendererConfigImpl(std::initializer_list<std::pair<Category, DcsmRendererConfig::CategoryInfo>> categories)
    {
        for (const auto& cat : categories)
            addCategory(cat.first, cat.second);
    }

    status_t DcsmRendererConfigImpl::addCategory(Category categoryId, const DcsmRendererConfig::CategoryInfo& categoryInfo)
    {
        if (m_categories.count(categoryId) > 0)
            return addErrorEntry("DcsmRendererConfig: attempting to add category already existing in the list, remove the duplicate!");
        m_categories[categoryId] = categoryInfo;
        return StatusOK;
    }

    const DcsmRendererConfig::CategoryInfo* DcsmRendererConfigImpl::findCategoryInfo(Category categoryId) const
    {
        const auto it = m_categories.find(categoryId);
        return (it != m_categories.cend() ? &it->second : nullptr);
    }

    const std::unordered_map<ramses::Category, ramses::DcsmRendererConfig::CategoryInfo>& DcsmRendererConfigImpl::getCategories() const
    {
        return m_categories;
    }
}
