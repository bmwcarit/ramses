//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "DcsmMetadataUpdateImpl.h"
#include "APILoggingMacros.h"

namespace ramses
{
    DcsmMetadataUpdate::DcsmMetadataUpdate(DcsmMetadataUpdateImpl& impl_)
        : StatusObject(impl_)
        , impl(impl_)
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    DcsmMetadataUpdate::~DcsmMetadataUpdate()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    bool DcsmMetadataUpdate::hasPreviewImagePng() const
    {
        return impl.hasPreviewImagePng();
    }

    std::vector<unsigned char> DcsmMetadataUpdate::getPreviewImagePng() const
    {
        return impl.getPreviewImagePng();
    }

    bool DcsmMetadataUpdate::hasPreviewDescription() const
    {
        return impl.hasPreviewDescription();
    }

    std::u32string DcsmMetadataUpdate::getPreviewDescription() const
    {
        return impl.getPreviewDescription();
    }

    bool DcsmMetadataUpdate::hasWidgetOrder() const
    {
        return impl.hasWidgetOrder();
    }

    int32_t DcsmMetadataUpdate::getWidgetOrder() const
    {
        return impl.getWidgetOrder();
    }

    bool DcsmMetadataUpdate::hasWidgetBackgroundID() const
    {
        return impl.hasWidgetBackgroundID();
    }

    int32_t DcsmMetadataUpdate::getWidgetBackgroundID() const
    {
        return impl.getWidgetBackgroundID();
    }

    bool DcsmMetadataUpdate::hasWidgetHUDLineID() const
    {
        return impl.hasWidgetHUDLineID();
    }

    int32_t DcsmMetadataUpdate::getWidgetHUDLineID() const
    {
        return impl.getWidgetHUDLineID();
    }

    bool DcsmMetadataUpdate::hasCarModel() const
    {
        return impl.hasCarModel();
    }

    int32_t DcsmMetadataUpdate::getCarModel() const
    {
        return impl.getCarModel();
    }

    bool DcsmMetadataUpdate::hasCarModelView() const
    {
        return impl.hasCarModelView();
    }

    CarModelViewMetadata DcsmMetadataUpdate::getCarModelView() const
    {
        return impl.getCarModelView();
    }

    AnimationInformation DcsmMetadataUpdate::getCarModelViewAnimationInfo() const
    {
        return impl.getCarModelViewAnimationInfo();
    }

    bool DcsmMetadataUpdate::hasCarModelVisibility() const
    {
        return impl.hasCarModelVisibility();
    }

    bool DcsmMetadataUpdate::getCarModelVisibility() const
    {
        return impl.getCarModelVisibility();
    }

    bool DcsmMetadataUpdate::hasExclusiveBackground() const
    {
        return impl.hasExclusiveBackground();
    }

    bool DcsmMetadataUpdate::getExclusiveBackground() const
    {
        return impl.getExclusiveBackground();
    }

    bool DcsmMetadataUpdate::hasStreamID() const
    {
        return impl.hasStreamID();
    }

    int32_t DcsmMetadataUpdate::getStreamID() const
    {
        return impl.getStreamID();
    }

    bool DcsmMetadataUpdate::hasContentFlippedVertically() const
    {
        return impl.hasContentFlippedVertically();
    }

    bool DcsmMetadataUpdate::getContentFlippedVertically() const
    {
        return impl.getContentFlippedVertically();
    }

    bool DcsmMetadataUpdate::hasDisplayedDataFlags() const
    {
        return impl.hasDisplayedDataFlags();
    }

    uint32_t DcsmMetadataUpdate::getDisplayedDataFlags() const
    {
        return impl.getDisplayedDataFlags();
    }
}
