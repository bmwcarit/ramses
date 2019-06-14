//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmConsumerImpl.h"
#include "RamsesFrameworkImpl.h"

namespace ramses
{
    DcsmConsumerImpl::DcsmConsumerImpl(RamsesFrameworkImpl& framework)
        : StatusObjectImpl()
        , m_component(framework.getDcsmComponent())
    {
        m_component.setLocalConsumerAvailability(true);
    }

    DcsmConsumerImpl::~DcsmConsumerImpl()
    {
        m_component.setLocalConsumerAvailability(false);
    }

    status_t DcsmConsumerImpl::dispatchEvents(IDcsmConsumerEventHandler& handler)
    {
        if (!m_component.dispatchConsumerEvents(handler))
            return addErrorEntry("DcsmConsumer::dispatchEvents failed");
        return StatusOK;
    }

    status_t DcsmConsumerImpl::sendCanvasSizeChange(ContentID contentID, SizeInfo size, AnimationInformation animationInformation)
    {
        ramses_internal::SizeInfo riSize;
        riSize.width = size.width;
        riSize.height = size.height;
        ramses_internal::AnimationInformation riAi;
        riAi.startTimeStamp = animationInformation.startTime;
        riAi.finishedTimeStamp = animationInformation.finishTime;

        if (!m_component.sendCanvasSizeChange(ramses_internal::ContentID(contentID.getValue()), riSize, riAi))
            return addErrorEntry("DcsmConsumer::sendCanvasSizeChange failed");
        return StatusOK;
    }

    status_t DcsmConsumerImpl::sendContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation animationInformation)
    {
        ramses_internal::EDcsmStatus riStatus = static_cast<ramses_internal::EDcsmStatus>(status);   // cast is allowed and tested
        ramses_internal::AnimationInformation riAi;
        riAi.startTimeStamp = animationInformation.startTime;
        riAi.finishedTimeStamp = animationInformation.finishTime;

        if (!m_component.sendContentStatusChange(ramses_internal::ContentID(contentID.getValue()), riStatus, riAi))
            return addErrorEntry("DcsmConsumer::sendContentStatusChange failed");
        return StatusOK;
    }
}
