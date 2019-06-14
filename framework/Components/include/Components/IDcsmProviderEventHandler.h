//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENTS_IDCSMPROVIDEREVENTHANDLER_H
#define RAMSES_COMPONENTS_IDCSMPROVIDEREVENTHANDLER_H

#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses_internal
{
    class Guid;

    class IDcsmProviderEventHandler
    {
    public:
        virtual ~IDcsmProviderEventHandler() = default;

        virtual void canvasSizeChange(ramses::ContentID, ramses::SizeInfo, ramses::AnimationInformation, const Guid& consumerID) = 0;
        virtual void contentStatusChange(ramses::ContentID, ramses::EDcsmStatus, ramses::AnimationInformation, const Guid& consumerID) = 0;
    };
}

#endif
