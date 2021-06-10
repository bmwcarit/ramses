//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDCSMCONSUMERIMPL_H
#define RAMSES_IDCSMCONSUMERIMPL_H

#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class IDcsmConsumerEventHandler;
    class CategoryInfoUpdate;
    class DcsmStatusMessage;

    class IDcsmConsumerImpl
    {
    public:
        virtual ~IDcsmConsumerImpl() = default;

        virtual status_t dispatchEvents(IDcsmConsumerEventHandler& handler) = 0;
        virtual status_t assignContentToConsumer(ContentID contentID, const CategoryInfoUpdate& size) = 0;
        virtual status_t contentSizeChange(ContentID contentID, const CategoryInfoUpdate& size, AnimationInformation animationInformation) = 0;
        virtual status_t contentStateChange(ContentID contentID, EDcsmState status, AnimationInformation animationInformation) = 0;
        virtual status_t acceptStopOffer(ContentID contentID, AnimationInformation animationInformation) = 0;
        virtual status_t sendContentStatus(ContentID contentID, DcsmStatusMessage const& message) = 0;
    };
}

#endif
