//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMAPITYPES_H
#define RAMSES_DCSMAPITYPES_H

#include "StronglyTypedValue.h"
#include <functional>
#include <limits>
#include <cstdint>

namespace ramses
{
    /// Unique type tag for ContentID
    struct ContentIDTag {};

    /**
     * @brief Identifier for DCSM content. Must be globally unique.
     */
    using ContentID = StronglyTypedValue<uint64_t, 0, ContentIDTag>;

    /// Unique type tag for Category
    struct CategoryTag {};

    /**
     * @brief Category descriptor for DCSM.
     */
    using Category = StronglyTypedValue<uint64_t, 0, CategoryTag>;

    /// Enum describing the possible distribution mechanisms for a specific content
    enum class ETechnicalContentType
    {
        RamsesSceneID,
    };

    /// Unique type tag for TechnicalContentDescriptor
    struct TechnicalContentDescriptorTag {};

    /**
     * @brief Distribution specific descriptor for DCSM content.
     *        It allows bindings content IDs to specific distribution mechanism, e.g. ramses scenes
     *        via sceneId_t.
     */
    using TechnicalContentDescriptor = StronglyTypedValue<uint64_t, 0, TechnicalContentDescriptorTag>;

    /**
     * @brief Rectangle consisting of x, y, width, height
     */
    struct Rect
    {
        /**
        * @brief Constructor requiring values to be specified explicitly
        * @param[in] xpos x position
        * @param[in] ypos y position
        * @param[in] w Width
        * @param[in] h Height
        */
        constexpr Rect(uint32_t xpos, uint32_t ypos, uint32_t w, uint32_t h)
            : x(xpos)
            , y(ypos)
            , width(w)
            , height(h)
        {
        }

        /// x position of rectangle
        uint32_t x;

        /// y position of rectangle
        uint32_t y;

        /// width of rectangle
        uint32_t width;

        /// height of rectangle
        uint32_t height;

        /**
         * @brief The equality comparison operator
         * @param rhs The instance to compare to
         * @return True if same, false otherwise
         */
        constexpr bool operator==(const Rect& rhs) const
        {
            return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
        }

        /**
         * @brief The inequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        constexpr bool operator!=(const Rect& rhs) const
        {
            return !(*this == rhs);
        }

    };
    /**
     * @brief Size information for DCSM canvas size change
     */
    struct SizeInfo
    {
        /**
        * @brief Constructor requiring size to be specified explicitly
        * @param[in] w Width
        * @param[in] h Height
        */
        constexpr SizeInfo(uint32_t w, uint32_t h)
            : width(w)
            , height(h)
        {
        }

        /// canvas width in pixels
        uint32_t width;

        /// canvas height in pixels
        uint32_t height;

        /**
         * @brief The equality comparison operator
         * @param rhs The instance to compare to
         * @return True if same, false otherwise
         */
        constexpr bool operator==(const SizeInfo& rhs) const
        {
            return (width == rhs.width && height == rhs.height);
        }

        /**
         * @brief The inequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        constexpr bool operator!=(const SizeInfo& rhs) const
        {
            return !(*this == rhs);
        }
    };

    /**
     * Animation timings for DCSM transition
     */
    struct AnimationInformation
    {
        /// start time of animation
        uint64_t startTime;

        /// end time of animation
        uint64_t finishTime;

        /**
         * @brief The equality comparison operator
         * @param rhs The instance to compare to
         * @return True if same, false otherwise
         */
        constexpr bool operator==(const AnimationInformation& rhs) const
        {
            return (startTime == rhs.startTime && finishTime == rhs.finishTime);
        }

        /**
         * @brief The unequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        constexpr bool operator!=(const AnimationInformation& rhs) const
        {
            return !(*this == rhs);
        }
    };

    /**
     * @brief DCSM content states. Used to request a specific state from provider via
     *        DcsmConsumer::sendContentStatusChange.
     */
    enum class EDcsmState
    {
        /// Release content to offered state, i.e. no longer assigned to consumer
        Offered,
        /// Request content to assigned only state
        Assigned,
        /// Request content to be prepare to be shown at any time
        Ready,
        /// Request content in shown state, i.e. actively updated
        Shown,
    };
}

#endif
