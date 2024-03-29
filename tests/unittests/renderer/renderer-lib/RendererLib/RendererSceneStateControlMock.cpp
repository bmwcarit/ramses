//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneStateControlMock.h"

using namespace testing;

namespace ramses::internal
{
    RendererSceneStateControlMock::RendererSceneStateControlMock()
    {
        ON_CALL(*this, handleSceneDisplayBufferAssignmentRequest(_, _, _)).WillByDefault(Return(true));
    }

    RendererSceneStateControlMock::~RendererSceneStateControlMock() = default;
}
