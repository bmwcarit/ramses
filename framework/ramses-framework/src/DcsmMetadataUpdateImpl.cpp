//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmMetadataUpdateImpl.h"

namespace ramses
{
    DcsmMetadataUpdateImpl::DcsmMetadataUpdateImpl() = default;
    DcsmMetadataUpdateImpl::~DcsmMetadataUpdateImpl() = default;

    bool DcsmMetadataUpdateImpl::hasPreviewImagePng() const
    {
        return m_metadata.hasPreviewImagePng();
    }

    std::vector<unsigned char> DcsmMetadataUpdateImpl::getPreviewImagePng() const
    {
        return m_metadata.getPreviewImagePng();
    }

    bool DcsmMetadataUpdateImpl::hasPreviewDescription() const
    {
        return m_metadata.hasPreviewDescription();
    }

    std::u32string DcsmMetadataUpdateImpl::getPreviewDescription() const
    {
        return m_metadata.getPreviewDescription();
    }

    bool DcsmMetadataUpdateImpl::hasWidgetOrder() const
    {
        return m_metadata.hasWidgetOrder();
    }

    int32_t DcsmMetadataUpdateImpl::getWidgetOrder() const
    {
        return m_metadata.getWidgetOrder();
    }

    bool DcsmMetadataUpdateImpl::hasWidgetBackgroundID() const
    {
        return m_metadata.hasWidgetBackgroundID();
    }

    int32_t DcsmMetadataUpdateImpl::getWidgetBackgroundID() const
    {
        return m_metadata.getWidgetBackgroundID();
    }

    bool DcsmMetadataUpdateImpl::hasWidgetHUDLineID() const
    {
        return m_metadata.getWidgetHUDLineID();
    }

    int32_t DcsmMetadataUpdateImpl::getWidgetHUDLineID() const
    {
        return m_metadata.getWidgetHUDLineID();
    }

    bool DcsmMetadataUpdateImpl::hasCarModel() const
    {
        return m_metadata.hasCarModel();
    }

    int32_t DcsmMetadataUpdateImpl::getCarModel() const
    {
        return m_metadata.getCarModel();
    }

    bool DcsmMetadataUpdateImpl::hasCarModelView() const
    {
        return m_metadata.hasCarModelView();
    }

    CarModelViewMetadata DcsmMetadataUpdateImpl::getCarModelView() const
    {
        return m_metadata.getCarModelView();
    }

    AnimationInformation DcsmMetadataUpdateImpl::getCarModelViewAnimationInfo() const
    {
        const auto& timing = m_metadata.getCarModelViewAnimationInfo();
        return AnimationInformation{ timing.startTimeStamp, timing.finishedTimeStamp };
    }

    bool DcsmMetadataUpdateImpl::hasCarModelVisibility() const
    {
        return m_metadata.hasCarModelVisibility();
    }

    bool DcsmMetadataUpdateImpl::getCarModelVisibility() const
    {
        return m_metadata.getCarModelVisibility();
    }

    bool DcsmMetadataUpdateImpl::hasExclusiveBackground() const
    {
        return m_metadata.hasExclusiveBackground();
    }

    bool DcsmMetadataUpdateImpl::getExclusiveBackground() const
    {
        return m_metadata.getExclusiveBackground();
    }

    bool DcsmMetadataUpdateImpl::hasStreamID() const
    {
        return m_metadata.hasStreamID();
    }

    int32_t DcsmMetadataUpdateImpl::getStreamID() const
    {
        return m_metadata.getStreamID();
    }

    void DcsmMetadataUpdateImpl::setMetadata(ramses_internal::DcsmMetadata metadata)
    {
        m_metadata = std::move(metadata);
    }

    ramses_internal::DcsmMetadata DcsmMetadataUpdateImpl::getMetadata() const
    {
        return m_metadata;
    }
}
