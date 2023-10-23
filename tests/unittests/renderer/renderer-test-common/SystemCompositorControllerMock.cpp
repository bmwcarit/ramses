//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SystemCompositorControllerMock.h"

using namespace testing;

namespace ramses::internal
{
    SystemCompositorControllerMock::SystemCompositorControllerMock()
    {
        ON_CALL(*this, addSurfaceToLayer(_,_)).WillByDefault(Return(true));
    }

    SystemCompositorControllerMock::~SystemCompositorControllerMock() = default;
}
