//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderBuffer.h"

// internal
#include "RenderTargetDescriptionImpl.h"

namespace ramses
{
    RenderTargetDescription::RenderTargetDescription()
        : StatusObject(*new RenderTargetDescriptionImpl())
        , impl(static_cast<RenderTargetDescriptionImpl&>(StatusObject::impl))
    {
    }

    status_t RenderTargetDescription::addRenderBuffer(const RenderBuffer& renderBuffer)
    {
        const status_t status = impl.addRenderBuffer(renderBuffer.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderBuffer));
        return status;
    }
}
