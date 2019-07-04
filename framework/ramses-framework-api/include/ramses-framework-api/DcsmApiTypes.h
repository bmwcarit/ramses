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

namespace ramses
{
    /// Unique type tag for ContentID
    struct ContentIDTag {};

    /**
     * @brief Identifier for DCSM content. Must be globally unique.
     */
    using ContentID = StronglyTypedValue<uint64_t, ContentIDTag>;

    /// Unique type tag for Category
    struct CategoryTag {};

    /**
     * @brief Category descriptor for DCSM.
     */
    typedef StronglyTypedValue<uint64_t, CategoryTag> Category;

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
    typedef StronglyTypedValue<uint64_t, TechnicalContentDescriptorTag> TechnicalContentDescriptor;

    /**
     * @brief Size information for DCSM canvas size change
     */
    struct SizeInfo
    {
        /// canvas width in pixels
        uint32_t width;

        /// canvas height in pixels
        uint32_t height;

        /**
         * @brief The equality comparison operator
         * @param rhs The instance to compare to
         * @return True if same, false otherwise
         */
        bool operator==(const SizeInfo& rhs) const
        {
            return (width == rhs.width && height == rhs.height);
        }

        /**
         * @brief The unequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        bool operator!=(const SizeInfo& rhs) const
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
        bool operator==(const AnimationInformation& rhs) const
        {
            return (startTime == rhs.startTime && finishTime == rhs.finishTime);
        }

        /**
         * @brief The unequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        bool operator!=(const AnimationInformation& rhs) const
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
