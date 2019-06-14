//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENT_IDCSMCOMPONENT_H
#define RAMSES_COMPONENT_IDCSMCOMPONENT_H

#include "Components/DcsmTypes.h"
#include "Components/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"

namespace ramses_internal
{
    class IDcsmComponent
    {
    public:
        virtual ~IDcsmComponent() = default;

        virtual bool setLocalConsumerAvailability(bool available) = 0;
        virtual bool setLocalProviderAvailability(bool available) = 0;

        virtual bool sendRegisterContent(ContentID contentID, Category) = 0;
        virtual bool sendContentAvailable(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor) = 0;
        virtual bool sendCategoryContentSwitchRequest(const Guid& to, ContentID contentID) = 0;
        virtual bool sendRequestUnregisterContent(ContentID contentID) = 0;

        virtual bool sendCanvasSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation ai) = 0;
        virtual bool sendContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation ai) = 0;

        virtual bool dispatchProviderEvents(IDcsmProviderEventHandler& handler) = 0;
        virtual bool dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler) = 0;
    };
}

#endif
