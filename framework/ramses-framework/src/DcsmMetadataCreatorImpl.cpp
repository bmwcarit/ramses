//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmMetadataCreatorImpl.h"
#include "Components/DcsmTypes.h"

namespace ramses
{
    DcsmMetadataCreatorImpl::DcsmMetadataCreatorImpl() = default;
    DcsmMetadataCreatorImpl::~DcsmMetadataCreatorImpl() = default;

    status_t DcsmMetadataCreatorImpl::setPreviewImagePng(const void* data, size_t dataLength)
    {
        if (!m_metadata.setPreviewImagePng(data, dataLength))
            return addErrorEntry("DcsmMetadataCreatorImpl::setPreviewImagePng: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setPreviewDescription(std::u32string previewDescription)
    {
        if (!m_metadata.setPreviewDescription(std::move(previewDescription)))
            return addErrorEntry("DcsmMetadataCreatorImpl::setPreviewDescription: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setWidgetOrder(int32_t widgetOrder)
    {
        if (!m_metadata.setWidgetOrder(widgetOrder))
            return addErrorEntry("DcsmMetadataCreatorImpl::setWidgetOrder: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setWidgetBackgroundID(int32_t backgroundID)
    {
        if (!m_metadata.setWidgetBackgroundID(backgroundID))
            return addErrorEntry("DcsmMetadataCreatorImpl::setWidgetBackgroundID: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setWidgetHUDLineID(int32_t widgetHudLineID)
    {
        if (!m_metadata.setWidgetHUDLineID(widgetHudLineID))
            return addErrorEntry("DcsmMetadataCreatorImpl::setWidgetHUDLineID: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setCarModel(int32_t carModel)
    {
        if (!m_metadata.setCarModel(carModel))
            return addErrorEntry("DcsmMetadataCreatorImpl::setCarModel: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setCarModelView(const CarModelViewMetadata& data, const AnimationInformation& timingInfo)
    {
        if (!m_metadata.setCarModelView(data, ramses_internal::AnimationInformation{ timingInfo.startTime, timingInfo.finishTime }))
            return addErrorEntry("DcsmMetadataCreatorImpl::setCarModelView: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setCarModelViewExtended(const CarModelViewMetadataExtended& data)
    {
        if (!m_metadata.setCarModelViewExtended(data))
            return addErrorEntry("DcsmMetadataCreatorImpl::setCarModelViewExtended: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setCarModelVisibility(bool visibility)
    {
        if (!m_metadata.setCarModelVisibility(visibility))
            return addErrorEntry("DcsmMetadataCreatorImpl::setCarModelVisibility: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setExclusiveBackground(bool state)
    {
        if (!m_metadata.setExclusiveBackground(state))
            return addErrorEntry("DcsmMetadataCreatorImpl::setExclusiveBackground: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setStreamID(int32_t streamID)
    {
        if (!m_metadata.setStreamID(streamID))
            return addErrorEntry("DcsmMetadataCreatorImpl::setStreamID: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setContentFlippedVertically(bool state)
    {
        if (!m_metadata.setContentFlippedVertically(state))
            return addErrorEntry("DcsmMetadataCreatorImpl::setContentFlippedVertically: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setDisplayedDataFlags(uint32_t flags)
    {
        if (!m_metadata.setDisplayedDataFlags(flags))
            return addErrorEntry("DcsmMetadataCreatorImpl::setDisplayedDataFlags: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setLayoutAvailability(uint8_t flags)
    {
        if (!m_metadata.setLayoutAvailability(flags))
            return addErrorEntry("DcsmMetadataCreatorImpl::setLayoutAvailability: input not valid");
        return StatusOK;
    }

    status_t DcsmMetadataCreatorImpl::setConfiguratorPriority(uint8_t priority)
    {
        if (!m_metadata.setConfiguratorPriority(priority))
            return addErrorEntry("DcsmMetadataCreatorImpl::setConfiguratorPriority: input not valid");
        return StatusOK;
    }

    ramses_internal::DcsmMetadata DcsmMetadataCreatorImpl::getMetadata() const
    {
        return m_metadata;
    }
}
