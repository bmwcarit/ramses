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

        /**
         * @brief The possible layout values in the instrument cluster.
         */
        enum class Layout : uint32_t
        {
            Drive = 0,
            Focus,
            Gallery,
            Autonomous,
            Sport_Road,
            Sport_Track,
        };

        /// @brief Default constructor with no data set
        CategoryInfoUpdate();

        /**
         * @brief constructor for CategoryInfoUpdate
         *
         * @param renderSize the new render size
         * @param categoryRect the new category rect
         * @param safeRect the new safe rect
         * @param layout the new active layout
         */
        CategoryInfoUpdate(SizeInfo renderSize, Rect categoryRect, Rect safeRect = {0,0,0,0}, Layout layout = Layout::Drive);

        /// Destructor
        ~CategoryInfoUpdate();

        /**
         * @brief Check if object contains category rect update.
         *
         * @return true when has category rect update, false when not
         */
        bool hasCategoryRectUpdate() const;

        /**
         * @brief Get new category rect.
         *        Only valid when hasCategoryRectUpdate().
         *
         * @return rectangle describing the category dimensions
         */
        Rect getCategoryRect() const;

        /**
         * @brief Set new category rect.
         *
         * Defined as rectangle inside render size (offsetX, offsetY, width, height).
         * The offset is relative to lower left corner of render size.
         * This is the area where the content should be. It is OK if you render content outside of category rect,
         * but such content will not be visible in the rendered result (except potentially during layout size transition).
         *
         * @param rect rectangle describing the new category dimensions
         * @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
         */
        status_t setCategoryRect(Rect rect);

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
         * Defined as width and height.
         * This can be thought of as the 'canvas' (display or offscreen render target) where content will be rendered.
         *
         * @param sizeInfo the new render size
         * @return StatusOK for success, otherwise the returned status can be used
        *          to resolve error message using getStatusMessage().
         */
        status_t setRenderSize(SizeInfo sizeInfo);

        /**
         * @brief Check if object contains safe rect update.
         *
         * @return true when has safe rect update, false when not
         */
        bool hasSafeRectUpdate() const;

        /**
         * @brief Get new safe rect.
         *        Only valid when hasSafeRectUpdate().
         *
         * @return rectangle describing the safe dimensions
         */
        Rect getSafeRect() const;

        /**
         * @brief Set new safe rect.
         *
         * @param rect rectangle describing the new safe dimensions
         *
         * Defined as rectangle relative to render size (offsetX, offsetY, width, height).
         * The offset is relative to lower left corner of render size.
         * This is just a hint from consumer which gives some guarantee that safe rect is not covered
         * by any other UI element on the display (if content actively shown).
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *          to resolve error message using getStatusMessage().
         */
        status_t setSafeRect(Rect rect);

        /**
         * @brief Check if object contains active layout update.
         *
         * @return true when has active layout update, false when not.
         */
        bool hasActiveLayoutUpdate() const;

        /**
         * @brief Get new active layout.
         *        Only valid when hasActiveLayoutUpdate().
         *
         * @return The currently active layout.
         */
        Layout getActiveLayout() const;

        /**
         * @brief Set new active layout.
         *
         * @param layout The currently active layout.
         *
         * The active layout provides information about the overall layout of the contents on
         * the consumer side. Depending on the layout, provider might want to provide different
         * visuals for his contents.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *          to resolve error message using getStatusMessage().
         */
        status_t setActiveLayout(CategoryInfoUpdate::Layout layout);

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
