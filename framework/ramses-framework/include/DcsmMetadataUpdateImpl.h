//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMMETADATAUPDATEIMPL_H
#define RAMSES_DCSMMETADATAUPDATEIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Components/DcsmMetadata.h"
#include "StatusObjectImpl.h"
#include "ramses-framework-api/CarModelViewMetadata.h"

namespace ramses
{
    class DcsmMetadataUpdateImpl : public StatusObjectImpl
    {
    public:
        // TODO(tobias) construct with metadata
        DcsmMetadataUpdateImpl();
        ~DcsmMetadataUpdateImpl();

        bool hasPreviewImagePng() const;
        std::vector<unsigned char> getPreviewImagePng() const;

        bool hasPreviewDescription() const;
        std::u32string getPreviewDescription() const;

        bool hasWidgetOrder() const;
        int32_t getWidgetOrder() const;

        bool hasWidgetBackgroundID() const;
        int32_t getWidgetBackgroundID() const;

        bool hasWidgetHUDLineID() const;
        int32_t getWidgetHUDLineID() const;

        bool hasCarModel() const;
        int32_t getCarModel() const;

        bool hasCarModelView() const;
        CarModelViewMetadata getCarModelView() const;
        AnimationInformation getCarModelViewAnimationInfo() const;

        bool hasCarModelViewExtended() const;
        CarModelViewMetadataExtended getCarModelViewExtended() const;

        bool hasCarModelVisibility() const;
        bool getCarModelVisibility() const;

        bool hasExclusiveBackground() const;
        bool getExclusiveBackground() const;

        bool hasStreamID() const;
        int32_t getStreamID() const;

        bool hasDisplayedDataFlags() const;
        uint32_t getDisplayedDataFlags() const;

        bool hasContentFlippedVertically() const;
        bool getContentFlippedVertically() const;

        bool hasLayoutAvailability() const;
        uint8_t getLayoutAvailability() const;

        void setMetadata(ramses_internal::DcsmMetadata metadata);
        ramses_internal::DcsmMetadata getMetadata() const;

    private:
        ramses_internal::DcsmMetadata m_metadata;
    };
}

#endif
