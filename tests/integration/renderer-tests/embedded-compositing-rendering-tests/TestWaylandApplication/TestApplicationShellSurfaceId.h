//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/StronglyTypedValue.h"

namespace ramses::internal
{
    struct TestApplicationShellSurfaceIdTag
    {
    };
    using TestApplicationShellSurfaceId = StronglyTypedValue<uint32_t, 0xFFFFFFFF, TestApplicationShellSurfaceIdTag>;
}
