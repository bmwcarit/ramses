//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMMETADATAUPDATE_H
#define RAMSES_DCSMMETADATAUPDATE_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/CarModelViewMetadata.h"
#include <string>
#include <vector>

namespace ramses
{
    class DcsmMetadataUpdateImpl;

    /**
     * @brief Class to extract DCSM metadata entries from. Objects of this type
     *        will be provided by ramses callbacks and should not be created directly.
     *
     * The object can be queried for all available metadata types and when they are
     * available the metadata entries can be extracted. See
     * IDcsmConsumerEventHandler::contentMetadataUpdated() for more details.
     */
    class RAMSES_API DcsmMetadataUpdate : public StatusObject
    {
    public:
        /// Destructor
        ~DcsmMetadataUpdate();

        /**
         * @brief Check if object contains PNG preview image metadata entry.
         *
         * @return true when has preview image metadata, false when not
         */
        bool hasPreviewImagePng() const;

        /**
         * @brief Get preview image metadata entry. The vector contains in memory
         *        representation of a PNG file.
         *        Will be empty when not hasPreviewImagePng().
         *
         * @return vector with PNG buffer
         */
        std::vector<unsigned char> getPreviewImagePng() const;

        /**
         * @brief Check if object contains preview description metadata entry.
         *
         * @return true when has preview description metadata, false when not
         */
        bool hasPreviewDescription() const;

        /**
         * @brief Get preview description metadata entry as utf32 string.
         *        Will be empty when not hasPreviewDescription().
         *
         * @return utf32 string with preview description
         */
        std::u32string getPreviewDescription() const;

        /**
         * @brief Check if object contains widget order metadata entry.
         *
         * @return true when has widget order metadata, false when not
         */
        bool hasWidgetOrder() const;

        /**
         * @brief Get widget order metadata entry.
         *        Value is undefined when not hasWidgetOrder().
         *
         * @return widget order
         */
        int32_t getWidgetOrder() const;

        /**
         * @brief Check if object contains widget background id metadata entry.
         *
         * @return true when has widget background id metadata, false when not
         */
        bool hasWidgetBackgroundID() const;

        /**
         * @brief Get widget background id metadata entry.
         *        Value is undefined when not hasWidgetBackgroundID().
         *
         * @return widget background id
         */
        int32_t getWidgetBackgroundID() const;

        /**
         * @brief Check if object contains widget hud line id metadata entry.
         *
         * @return true when has widget hud line id metadata, false when not
         */
        bool hasWidgetHUDLineID() const;

        /**
         * @brief Get widget hud line id metadata entry.
         *        Value is undefined when not hasWidgetHUDLineID().
         *
         * @return widget background id
         */
        int32_t getWidgetHUDLineID() const;

        /**
         * @brief Check if object contains car model metadata entry.
         *
         * @return true when has car model metadata, false when not
         */
        bool hasCarModel() const;

        /**
         * @brief Get car model metadata entry.
         *        Value is undefined when not hasCarModel().
         *
         * @return car model metadata
         */
        int32_t getCarModel() const;

        /**
         * @brief Check if object contains car model view metadata entry.
         *
         * @return true when has car model view metadata, false when not
         */
        bool hasCarModelView() const;

        /**
         * @brief Get car model view metadata entry.
         *        Value is undefined when not hasCarModelView().
         *
         * @return car model view metadata
         */
        CarModelViewMetadata getCarModelView() const;

        /**
         * @brief Check if object contains car model view extended metadata entry.
         *
         * @return true when has car model view extended metadata, false when not
         */
        bool hasCarModelViewExtended() const;

        /**
         * @brief Get car model view extended metadata entry.
         *        Value is undefined when not hasCarModelViewExtended().
         *
         * @return car model view extended metadata
         */
        CarModelViewMetadataExtended getCarModelViewExtended() const;

        /**
        * @brief Get animation information associated with car model view metadata entry.
        *        Value is undefined when not hasCarModelView().
        *
        * @return animation information for car model view metadata
        */
        AnimationInformation getCarModelViewAnimationInfo() const;

        /**
         * @brief Check if object contains car model visibility metadata entry.
         *
         * @return true when has car model visibility metadata, false when not
         */
        bool hasCarModelVisibility() const;

        /**
         * @brief Get car model visibility metadata entry.
         *        Value is undefined when not hasCarModelVisibility().
         *
         * @return car model visibility
         */
        bool getCarModelVisibility() const;

        /**
         * @brief Check if object contains exclusive background metadata entry.
         *
         * @return true when has exclusive background metadata, false when not
         */
        bool hasExclusiveBackground() const;

        /**
         * @brief Get exclusive background metadata entry.
         *        Value is undefined when not hasExclusiveBackground().
         *
         * @return exclusive background state
         */
        bool getExclusiveBackground() const;

        /**
         * @brief Check if object contains streamID metadata entry.
         *
         * @return true when has streamID metadata, false when not
         */
        bool hasStreamID() const;

        /**
         * @brief Get streamID metadata entry.
         *        Value is undefined when not hasStreamID().
         *
         * @return streamID
         */
        int32_t getStreamID() const;

        /**
        * @brief Check if object contains displayed data flags metadata entry.
        *
        * @return true when has displayed data flags metadata, false when not
        */
        bool hasDisplayedDataFlags() const;

        /**
        * @brief Get displayed data flags.
        *        Value is undefined when not #hasDisplayedDataFlags().
        *
        * @return displayed data flags
        */
        uint32_t getDisplayedDataFlags() const;

        /**
         * @brief Check if object contains a contentFlippedVertically entry.
         *
         * @return true when has content flipped vertically metadata, false when not
         */
        bool hasContentFlippedVertically() const;

        /**
         * @brief Get contentFlippedVertically metadata entry.
         *        Value is undefined when not hasContentFlippedVertically().
         *
         * When set provider requests to flip content on Y-axis before displaying.
         *
         * @return content flipped vertically state
         */
        bool getContentFlippedVertically() const;

        /**
        * @brief Check if object contains layoutAvailability metadata entry.
        *
        * @return true when has layoutAvailability metadata, false when not
        */
        bool hasLayoutAvailability() const;

        /**
        * @brief Get layout availability metadata entry.
        *        Value is undefined when not #hasLayoutAvailability().
        *
        * @return Bitmask of displaycluster::M_Modes
        */
        uint8_t getLayoutAvailability() const;

        /**
         * @brief Deleted default constructor
         */
        DcsmMetadataUpdate() = delete;

        /**
         * @brief Constructor from impl
         * @param impl_ impl
         */
        explicit DcsmMetadataUpdate(DcsmMetadataUpdateImpl& impl_);

        /**
         * @brief Deleted move constructor
         * @param other unused
         */
        DcsmMetadataUpdate(DcsmMetadataUpdate&& other) = delete;

        /**
         * @brief Deleted move assignment
         * @param other unused
         * @return unused
         */
        DcsmMetadataUpdate& operator=(DcsmMetadataUpdate&& other) = delete;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        DcsmMetadataUpdate(const DcsmMetadataUpdate& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        DcsmMetadataUpdate& operator=(const DcsmMetadataUpdate& other) = delete;

        /**
         * Stores internal data for implementation specifics of DcsmMetadataUpdate
         */
        DcsmMetadataUpdateImpl& impl;
    };
}

#endif
