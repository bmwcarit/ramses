//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONTENTCONTROLCONFIG_H
#define RAMSES_DCSMCONTENTCONTROLCONFIG_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    /**
    * @brief Container with parameters to use to instantiate #ramses::DcsmContentControl.
    *
    * Dcsm categories are considered static within application session, DcsmContentControlConfig
    * is the place to put all the valid categories that should be used.
    * #ramses::DcsmContentControl will then accept Dcsm content offers for these categories (see #ramses::DcsmContentControl for more info).
    *
    * Note: modifying/adding any parameter after instantiating #ramses::DcsmContentControl will have no effect.
    */
    class RAMSES_API DcsmContentControlConfig : public StatusObject
    {
    public:
        /// Initial category information
        struct CategoryInfo
        {
            /// Size in pixels that a content assigned to this category should be configured for.
            /// Size can be modified later via #ramses::DcsmContentControl::setCategorySize.
            SizeInfo size;
            /// Display where this category belongs. All contents assigned to this category will be mapped to its display.
            displayId_t display;
        };

        /// Default constructor
        DcsmContentControlConfig();
        /** @brief Constructor with list of categories.
        *          Equivalent to calling #ramses::DcsmContentControlConfig::addCategory for each element.
        * @param categories Categories to be added to this config.
        */
        DcsmContentControlConfig(std::initializer_list<std::pair<Category, CategoryInfo>> categories);

        /** @brief Adds category to the list of categories owned by a #ramses::DcsmContentControl instantiated later using this config.
        * @param categoryId Unique ID of the category added (must be known by content provider which offers content for it).
        * @param categoryInfo Initial category information.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addCategory(Category categoryId, const CategoryInfo& categoryInfo);

        /** @brief Finds category information that was previously added.
        * @param categoryId Unique ID of the category to find.
        * @return Pointer to category information if found, nullptr otherwise.
        */
        const CategoryInfo* findCategoryInfo(Category categoryId) const;

        /// Implementation
        class DcsmContentControlConfigImpl& m_impl;
    };
}

#endif
