//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONSUMERIMPL_H
#define RAMSES_DCSMCONSUMERIMPL_H

#include "StatusObjectImpl.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "Components/DcsmComponent.h"

namespace ramses
{
    class RamsesFrameworkImpl;

    class DcsmConsumerImpl : public StatusObjectImpl
    {
    public:
        DcsmConsumerImpl(RamsesFrameworkImpl& framework);
        ~DcsmConsumerImpl();

        status_t dispatchEvents(IDcsmConsumerEventHandler& handler);

        status_t assignContentToConsumer(ContentID contentID, SizeInfo size);
        status_t contentSizeChange(ContentID contentID, SizeInfo size, AnimationInformation animationInformation);
        status_t contentStateChange(ContentID contentID, EDcsmState status, AnimationInformation animationInformation);
        status_t acceptStopOffer(ContentID contentID, AnimationInformation animationInformation);

    private:
        ramses_internal::DcsmComponent& m_component;
    };
}

#endif
