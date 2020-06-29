//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/DcsmContentControlConfig.h"
#include "DcsmContentControlConfigImpl.h"
#include "APILoggingMacros.h"
#include "RamsesFrameworkTypesImpl.h"

namespace ramses
{
    DcsmContentControlConfig::DcsmContentControlConfig()
        : DcsmContentControlConfig({})
    {
        LOG_HL_RENDERER_API_NOARG(LOG_API_VOID);
    }

    DcsmContentControlConfig::DcsmContentControlConfig(std::initializer_list<std::pair<Category, CategoryInfo>> categories)
        : StatusObject(*new DcsmContentControlConfigImpl(categories))
        , m_impl(static_cast<DcsmContentControlConfigImpl&>(StatusObject::impl))
    {
        for (const auto& c : categories)
        {
            LOG_HL_RENDERER_API4(LOG_API_VOID, c.first, c.second.size.width, c.second.size.height, c.second.display);
        }
    }

    status_t DcsmContentControlConfig::addCategory(Category categoryId, const CategoryInfo& categoryInfo)
    {
        const auto status = m_impl.addCategory(categoryId, categoryInfo);
        LOG_HL_RENDERER_API4(status, categoryId, categoryInfo.size.width, categoryInfo.size.height, categoryInfo.display);
        return status;
    }

    const DcsmContentControlConfig::CategoryInfo* DcsmContentControlConfig::findCategoryInfo(Category categoryId) const
    {
        return m_impl.findCategoryInfo(categoryId);
    }
}
