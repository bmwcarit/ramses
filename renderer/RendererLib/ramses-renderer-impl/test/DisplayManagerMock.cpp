//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayManagerMock.h"

using namespace testing;

namespace ramses
{
    DisplayManagerMock::DisplayManagerMock()
    {
        ON_CALL(*this, setSceneState(_, _, _)).WillByDefault(Return(true));
        ON_CALL(*this, setSceneMapping(_, _, _)).WillByDefault(Return(true));
        ON_CALL(*this, setSceneOffscreenBufferMapping(_, _, _, _, _, _)).WillByDefault(Return(true));
        EXPECT_CALL(*this, processConfirmationEchoCommand(_)).Times(AnyNumber());
    }
}
