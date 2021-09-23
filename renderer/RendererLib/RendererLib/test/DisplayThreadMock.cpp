//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayThreadMock.h"

namespace ramses_internal
{
    DisplayThreadMock::DisplayThreadMock()
    {
        ON_CALL(*this, getFrameCounter()).WillByDefault(Return(0u));
        EXPECT_CALL(*this, getFrameCounter()).Times(AnyNumber());
    }

    DisplayThreadMock::~DisplayThreadMock() = default;
}
