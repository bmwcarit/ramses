//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CATEGORYINFOUPDATE_H
#define RAMSES_CATEGORYINFOUPDATE_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    class CategoryInfoUpdateImpl;

    /**
     * @brief Update of information about a DCSM category. Contains information like changes in size.
     *        Objects of this type will be provided by ramses callbacks and are used to give information on consumer side.
     *        Do not reuse objects on consumer side, since they track changes and would transmit these again if reused
     *
     * The object can be queried for all available data and when available can be extracted.
     * See IDcsmProviderEventHandler::contentSizeChange() for more details.
     */
    class RAMSES_API CategoryInfoUpdate : public StatusObject
    {
    public:

        /// @brief Default constructor with no data set
        CategoryInfoUpdate();

        /*
         * @brief Convenience constructor setting the category width and height (x/y position 0)
         *
         * @param categorySize the new category size
         */
        explicit CategoryInfoUpdate(SizeInfo categorySize);

        /// Destructor
        ~CategoryInfoUpdate();

        /**
         * @brief Check if object contains category size update.
         *
         * @return true when has category size update, false when not
         */
        bool hasCategorySizeUpdate() const;

        /**
         * @brief Get new category size.
         *        Only valid when hasCategorySizeUpdate().
         *
         * @return rectangle describing the category siye
         */
        Rect getCategorySize() const;

        /**
         * @brief Set new category size.
         *
         * @param rect rectangle describing the new category size
         * @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
         */
        status_t setCategorySize(Rect rect);

        /**
         * @brief Check if object contains render size update.
         *
         * @return true when has render size update, false when not
         */
        bool hasRenderSizeUpdate() const;

        /**
         * @brief Get new render size.
         *        Only valid when hasRenderSizeUpdate().
         *
         * @return the new render size
         */
        SizeInfo getRenderSize() const;

        /**
         * @brief Set new render size.
         *
         * @param sizeInfo the new render size
         * @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
         */
        status_t setRenderSize(SizeInfo sizeInfo);

        /**
         * @brief Check if object contains safe area size update.
         *
         * @return true when has safe area size update, false when not
         */
        bool hasSafeAreaSizeUpdate() const;

        /**
         * @brief Get new safe area size.
         *        Only valid when hasSafeAreaSizeUpdate().
         *
         * @return rectangle describing the safe area size
         */
        Rect getSafeAreaSize() const;

        /**
         * @brief Set new safe area size.
         *
         * @param rect rectangle describing the new safe area size
         * @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
         */
        status_t setSafeAreaSize(Rect rect);

        /**
         * @brief Constructor from impl
         * @param impl_ impl
         */
        explicit CategoryInfoUpdate(CategoryInfoUpdateImpl& impl_);

        /**
         * @brief Deleted move constructor
         * @param other unused
         */
        CategoryInfoUpdate(CategoryInfoUpdate&& other) = delete;

        /**
         * @brief Deleted move assignment
         * @param other unused
         * @return unused
         */
        CategoryInfoUpdate& operator=(CategoryInfoUpdate&& other) = delete;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        CategoryInfoUpdate(const CategoryInfoUpdate& other) = delete;

        /**
         * @brief The equality comparison operator
         * @param rhs The instance to compare to
         * @return True if same, false otherwise
         */
        bool operator==(const CategoryInfoUpdate& rhs) const;
        /**
         * @brief The inequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        bool operator!=(const CategoryInfoUpdate& rhs) const;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        CategoryInfoUpdate& operator=(const CategoryInfoUpdate& other) = delete;

        /**
         * Stores internal data for implementation specifics of CategoryInfo
         */
        CategoryInfoUpdateImpl& impl;
    };
}

#endif
