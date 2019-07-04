//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmConsumer.h"
#include "APILoggingHelper.h"
#include "DcsmConsumerImpl.h"

namespace ramses
{
    DcsmConsumer::DcsmConsumer(DcsmConsumerImpl& impl_)
        : StatusObject(impl_)
        , impl(impl_)
    {
        LOG_HL_CLIENT_API1(LOG_API_VOID, LOG_API_GENERIC_OBJECT_STRING(impl));
    }

    DcsmConsumer::~DcsmConsumer()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    status_t DcsmConsumer::dispatchEvents(IDcsmConsumerEventHandler& handler)
    {
        const status_t result = impl.dispatchEvents(handler);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_OBJECT_STRING(handler));
        return result;
    }

    status_t DcsmConsumer::assignContentToConsumer(ContentID contentID, SizeInfo size)
    {
        const status_t result = impl.assignContentToConsumer(contentID, size);
        LOG_HL_CLIENT_API3(result, contentID.getValue(), size.width, size.height);
        return result;
    }

    status_t DcsmConsumer::contentSizeChange(ContentID contentID, SizeInfo size, AnimationInformation animationInformation)
    {
        const status_t result = impl.contentSizeChange(contentID, size, animationInformation);
        LOG_HL_CLIENT_API5(result, contentID.getValue(), size.width, size.height, animationInformation.startTime, animationInformation.finishTime);
        return result;
    }

    status_t DcsmConsumer::contentStateChange(ContentID contentID, EDcsmState state, AnimationInformation animationInformation)
    {
        const status_t result = impl.contentStateChange(contentID, state, animationInformation);
        LOG_HL_CLIENT_API4(result, contentID.getValue(), static_cast<uint32_t>(state), animationInformation.startTime, animationInformation.finishTime);
        return result;
    }

    status_t DcsmConsumer::acceptStopOffer(ContentID contentID, AnimationInformation animationInformation)
    {
        const status_t result = impl.acceptStopOffer(contentID, animationInformation);
        LOG_HL_CLIENT_API3(result, contentID.getValue(), animationInformation.startTime, animationInformation.finishTime);
        return result;
    }

}
