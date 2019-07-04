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

    status_t DcsmConsumerImpl::assignContentToConsumer(ContentID contentID, SizeInfo size)
    {
        ramses_internal::SizeInfo riSize;
        riSize.width = size.width;
        riSize.height = size.height;

        if (!m_component.sendContentStateChange(ramses_internal::ContentID(contentID.getValue()), ramses_internal::EDcsmState::Assigned,
                                                riSize, ramses_internal::AnimationInformation{0, 0}))
            return addErrorEntry("DcsmConsumer::sendContentStateChange caused by assignContentToConsumer failed");
        return StatusOK;
    }

    status_t DcsmConsumerImpl::contentSizeChange(ContentID contentID, SizeInfo size, AnimationInformation animationInformation)
    {
        ramses_internal::SizeInfo riSize;
        riSize.width = size.width;
        riSize.height = size.height;
        ramses_internal::AnimationInformation riAi;
        riAi.startTimeStamp = animationInformation.startTime;
        riAi.finishedTimeStamp = animationInformation.finishTime;

        if (!m_component.sendCanvasSizeChange(ramses_internal::ContentID(contentID.getValue()), riSize, riAi))
            return addErrorEntry("DcsmConsumer::sendContentSizeChange failed");
        return StatusOK;
    }

    status_t DcsmConsumerImpl::contentStateChange(ContentID contentID, EDcsmState state, AnimationInformation animationInformation)
    {
        ramses_internal::EDcsmState riState;
        switch (state)
        {
        case EDcsmState::Offered:
            riState = ramses_internal::EDcsmState::Offered;
            break;
        case EDcsmState::Assigned:
            riState = ramses_internal::EDcsmState::Assigned;
            break;
        case EDcsmState::Ready:
            riState = ramses_internal::EDcsmState::Ready;
            break;
        case EDcsmState::Shown:
            riState = ramses_internal::EDcsmState::Shown;
            break;
        default:
            return addErrorEntry("DcsmConsumer::sendContentStatusChange failed with invalid EDcsmState");
        }
        ramses_internal::AnimationInformation riAi;
        riAi.startTimeStamp = animationInformation.startTime;
        riAi.finishedTimeStamp = animationInformation.finishTime;

        if (!m_component.sendContentStateChange(ramses_internal::ContentID(contentID.getValue()), riState, ramses_internal::SizeInfo{0, 0}, riAi))
            return addErrorEntry("DcsmConsumer::sendContentStatusChange failed");
        return StatusOK;
    }

    status_t DcsmConsumerImpl::acceptStopOffer(ContentID contentID, AnimationInformation animationInformation)
    {
        ramses_internal::AnimationInformation riAi;
        riAi.startTimeStamp = animationInformation.startTime;
        riAi.finishedTimeStamp = animationInformation.finishTime;
        if (!m_component.sendContentStateChange(ramses_internal::ContentID(contentID.getValue()), ramses_internal::EDcsmState::AcceptStopOffer, ramses_internal::SizeInfo{0, 0}, riAi))
            return addErrorEntry("DcsmConsumer::sendAcceptStopOffer failed");
        return StatusOK;
    }

}
