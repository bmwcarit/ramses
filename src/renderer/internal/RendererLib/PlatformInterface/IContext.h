//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"

namespace ramses::internal
{
    class DeviceResourceMapper;

    class IContext
    {
    public:
        virtual ~IContext() = default;

        virtual bool swapBuffers() = 0;
        virtual bool enable() = 0;
        virtual bool disable() = 0;
        virtual DeviceResourceMapper& getResources() = 0;

        // TODO Violin this should be removed - provides access to platform-specific data
        virtual void* getProcAddress(const char* name) const = 0;
    };
}
